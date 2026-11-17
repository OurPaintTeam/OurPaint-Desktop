#include "QtUIController.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QScopedValueRollback>
#include <QSignalBlocker>
#include <QTimer>
#include <algorithm>
#include <set>

#include "QtViewport.h"
#include "ViewportController.h"

namespace {
std::filesystem::path nativePath(const QString& path) {
#ifdef Q_OS_WIN
    return path.toStdWString();
#else
    return std::filesystem::u8path(path.toUtf8().constData());
#endif
}
QString displayPath(const std::filesystem::path& path) {
#ifdef Q_OS_WIN
    return QString::fromStdWString(path.native());
#else
    const auto text = path.u8string();
    return QString::fromUtf8(reinterpret_cast<const char*>(text.data()), qsizetype(text.size()));
#endif
}
qint64 token(app::ViewId view) { return static_cast<qint64>(view); }
}  // namespace

QtUIController::Window::~Window() {
    // ProjectManager owns the window container and its embedded QWindow.
    if (viewport) {
        viewport->releaseAll();
    }
    ui.reset();
}

QtUIController::QtUIController(app::UIController& controller, const QString& configDirectory) : controller_(controller), settings_(configDirectory) {
    connect(&observer_, &UIObserver::viewClosing, this, &QtUIController::detach, Qt::DirectConnection);
    connect(&observer_, &UIObserver::activeChanged, this, [this](app::ViewId view) { presentTool(view); });
    connect(&observer_, &UIObserver::selectedToolChanged, this, [this](app::ViewId view, ToolId) { presentTool(view); });
    connect(&observer_, &UIObserver::redraw, this, [this](app::ViewId view) {
        const auto found = bindings_.find(token(view));
        if (found != bindings_.end() && found->second.window->active == view) {
            found->second.window->viewport->update();
        }
    });
    applyLanguage();
    auto& window = createWindow();
    createDocument(window, QStringLiteral("Untitled"), false);
    refreshRecentDocuments();
    if (controller_.metadataError()) {
        window.ui->addNotification(tr("Could not read .ourpaint; recent documents will not be overwritten."));
    }
    // Subscribe only after construction succeeds; App must never retain a pointer
    // to an observer whose constructor unwound before its destructor could run.
    controller_.setObserver(&observer_);
}

QtUIController::~QtUIController() {
    controller_.setObserver(nullptr);
    for (auto& window : windows_) {
        if (window->widget) {
            window->widget->removeEventFilter(this);
        }
        if (window->viewport) {
            window->viewport->activated = {};
        }
    }
    // Contexts must outlive renderer shutdown, and controllers must outlive windows.
    windows_.clear();
    controller_.clearDocumentViews();
    qApp->removeTranslator(&translator_);
}

QtUIController::Window& QtUIController::createWindow(bool light) {
    auto window = std::make_unique<Window>();
    window->pendingViewport = std::make_unique<QtViewport>();
    window->viewport = window->pendingViewport.get();
    window->light = light;
    window->ui = std::make_unique<UI::ProjectManager>(UI::EditorContext{nullptr, &settings_, window->viewport, nullptr});
    auto& result = *window;
    windows_.push_back(std::move(window));
    connectWindow(result);
    result.viewport->activated = [this, &result] {
        if (!result.closing && result.active != app::InvalidViewId) {
            controller_.setActiveDocumentView(result.active);
        }
    };
    return result;
}

QtUIController::Window& QtUIController::mainWindow() {
    for (auto& window : windows_) {
        if (!window->light && !window->closing) {
            return *window;
        }
    }
    return createWindow();
}

void QtUIController::connectWindow(Window& w) {
    auto* ui = w.ui.get();
    connect(ui, &UI::ProjectManager::createFileTriggered, this, [this, &w](const QString& name, bool newWindow) { createDocument(w, name, newWindow); });
    connect(ui, &UI::ProjectManager::openFileTriggered, this, [this, &w](const QString& path, bool newWindow) { openDocument(w, path, newWindow); });
    connect(ui, &UI::ProjectManager::openFileByIDTriggered, this, [this, &w](qint64 id, bool newWindow) {
        const auto found = recentPaths_.find(id);
        if (found != recentPaths_.end()) {
            openDocument(w, found->second, newWindow);
        }
    });
    connect(ui, &UI::ProjectManager::closeFileTriggered, this, [this](qint64 id) { closeView(viewFor(id)); });
    connect(ui, &UI::ProjectManager::setActiveFileTriggered, this, [this, &w](qint64 id) { activate(w, viewFor(id)); });
    connect(ui, &UI::ProjectManager::renameFileTriggered, this, [this](UI::FileData& data) { rename(data.id, data.name); });
    connect(ui, &UI::ProjectManager::removeFileTriggered, this, [&w](qint64) {
        // The library conflates closing a tab, forgetting a recent path and deleting
        // a file. Do not silently turn one of these requests into data deletion.
        w.ui->addNotification(tr("Deleting files is not supported here. Use Close to close a view."));
    });
    connect(ui, &UI::ProjectManager::saveTriggered, this, [this, &w] {
        if (w.active != app::InvalidViewId) {
            save(controller_.documentOf(w.active), w.widget);
        }
    });
    connect(ui, &UI::ProjectManager::primitiveTriggered, this, [this, &w](qint64 id, UI::PrimitiveType type) { primitive(w, id, type); });
    connect(ui, &UI::ProjectManager::constraintTriggered, this, [this, &w](qint64 id, UI::ConstraintType type) { constraint(w, id, type); });
    connect(ui, &UI::ProjectManager::toolsTriggered, this, [this, &w](qint64 id, UI::ToolsType type) {
        if (type == UI::ToolsType::None) {
            selectTool(w, id, ToolId::Cursor);
        } else {
            w.ui->addNotification(tr("This tool is not available yet."));
        }
    });
    connect(ui, &UI::ProjectManager::sentCommandTriggered, this,
            [&w](qint64, const QString&) { w.ui->addNotification(tr("Console commands are not available yet.")); });
    connect(ui, &UI::ProjectManager::scriptTriggered, this, [&w](const QString&) { w.ui->addNotification(tr("Running scripts is not available yet.")); });
    connect(ui, &UI::ProjectManager::settingsChanged, this, [this, &w] {
        applyLanguage();
        w.ui->addNotification(tr("Interface settings applied. Grid and snap preferences are not supported yet."));
    });
    connect(ui, &UI::ProjectManager::lightEditorRequested, this, [this](qint64 id) {
        const auto view = viewFor(id);
        if (view != app::InvalidViewId) {
            moveView(view, createWindow(true));
        }
    });
    connect(ui, &UI::ProjectManager::returnTabToEditorRequested, this, [this](qint64 id) {
        const auto view = viewFor(id);
        if (view != app::InvalidViewId) {
            moveView(view, mainWindow());
        }
    });
    connect(ui, &UI::ProjectManager::tabMovedToAnotherWindow, this, [this](qint64 id, quintptr source, quintptr destination) {
        const auto found = bindings_.find(id);
        if (found == bindings_.end() || reinterpret_cast<quintptr>(found->second.window->widget.data()) != source) {
            return;
        }
        for (auto& target : windows_) {
            if (reinterpret_cast<quintptr>(target->widget.data()) == destination && !target->light && !target->closing) {
                moveView(found->second.view, *target);
                break;
            }
        }
    });
}

void QtUIController::installWindowActions(Window& window) {
    if (window.widget) {
        return;
    }
    window.widget = window.ui->windowWidget();
    window.widget->installEventFilter(this);
    const auto action = [&](const QKeySequence& key, auto callback) {
        auto* a = new QAction(window.widget);
        a->setShortcut(key);
        window.widget->addAction(a);
        connect(a, &QAction::triggered, this, callback);
    };
    action(QKeySequence::SaveAs, [this, &window] {
        if (window.active != app::InvalidViewId) {
            save(controller_.documentOf(window.active), window.widget, true);
        }
    });
    action(QKeySequence::New, [this, &window] { createDocument(window, QStringLiteral("Untitled"), window.light); });
    action(QKeySequence::Close, [this, &window] { closeView(window.active); });
    // Open another independent view of the same document without duplicating CAD data.
    action(QKeySequence(QStringLiteral("Ctrl+Shift+N")), [this, &window] {
        const auto document = controller_.documentOf(window.active);
        if (document != app::InvalidDocumentId) {
            attach(createWindow(), controller_.createDocumentView(document));
        }
    });
}

QString QtUIController::title(app::DocumentId document) const {
    const auto path = controller_.documentPath(document);
    if (!path.empty()) {
        return QFileInfo(displayPath(path)).fileName();
    }
    return QString::fromStdString(controller_.documentName(document));
}

void QtUIController::attach(Window& window, app::ViewId view) {
    if (view == app::InvalidViewId) {
        return;
    }
    bindings_.emplace(token(view), Binding{&window, view});
    UI::FileData data{token(view), title(controller_.documentOf(view))};
    {
        const QSignalBlocker blocker(window.ui.get());
        if (window.light) {
            window.ui->showLightEditorWindow(data);
        } else {
            window.ui->addTabSlot(data);
        }
    }
    // The container now owns the surface. QPointer only observes its lifetime.
    window.pendingViewport.release();
    installWindowActions(window);
    activate(window, view);
    refreshRecentDocuments();
}

void QtUIController::activate(Window& window, app::ViewId view) {
    const auto found = bindings_.find(token(view));
    if (found == bindings_.end() || found->second.window != &window || window.closing) {
        return;
    }
    window.active = view;
    window.viewport->showViewport(controller_.findViewport(view));
    {
        const QSignalBlocker blocker(window.ui.get());
        window.ui->setActiveTab(token(view));
    }
    controller_.setActiveDocumentView(view);
    presentTool(view);
}

void QtUIController::detach(app::ViewId view) {
    const auto found = bindings_.find(token(view));
    if (found == bindings_.end()) {
        return;
    }
    auto& window = *found->second.window;
    window.viewport->releaseViewport(controller_.findViewport(view));
    bindings_.erase(found);
    if (!window.light) {
        const QSignalBlocker blocker(window.ui.get());
        window.ui->closeTabSlot(token(view));
    }
    if (window.active == view) {
        window.active = app::InvalidViewId;
        // Observer callbacks may query App but must not mutate it reentrantly.
        const auto next = viewFor(window.ui->getActiveTabId());
        if (next != app::InvalidViewId) {
            window.active = next;
            window.viewport->showViewport(controller_.findViewport(next));
            presentTool(next);
        }
    }
}

void QtUIController::moveView(app::ViewId view, Window& target) {
    const auto found = bindings_.find(token(view));
    if (found == bindings_.end() || found->second.window == &target) {
        return;
    }
    auto& source = *found->second.window;
    // Recreate only context-bound renderer resources. Camera, editor and ViewId survive.
    detach(view);
    attach(target, view);
    if (source.light) {
        retire(source);
    }
}

void QtUIController::retire(Window& window) {
    window.closing = true;
    if (window.widget) {
        window.widget->hide();
    }
    // Never delete a signal sender/window while its event handler is on the stack.
    QTimer::singleShot(0, this, [this, pointer = &window] { destroyWindow(pointer); });
}

void QtUIController::destroyWindow(Window* window) {
    const auto found = std::find_if(windows_.begin(), windows_.end(), [window](const auto& item) { return item.get() == window; });
    if (found == windows_.end()) {
        return;
    }
    // Destruction can send native focus/window events synchronously. Finish the
    // container mutation first so those events see a stable list of live windows.
    auto dying = std::move(*found);
    windows_.erase(found);
    if (dying->widget) {
        dying->widget->removeEventFilter(this);
    }
    if (dying->viewport) {
        dying->viewport->activated = {};
    }
}

void QtUIController::createDocument(Window& window, const QString& requested, bool newWindow) {
    // Creation only gives a draft its display name; Save As chooses its path.
    QString name = requested.trimmed();
    if (name.isEmpty()) {
        name = QStringLiteral("Untitled");
    }
    const auto document = controller_.createDocument(name.toStdString());
    attach(newWindow || window.light ? createWindow() : window, controller_.createDocumentView(document));
}

void QtUIController::openDocument(Window& window, const QString& path, bool newWindow) {
    const auto opened = controller_.openDocument(nativePath(path));
    if (!opened) {
        showError(opened.error(), window.widget);
        return;
    }
    // Reopening a path reuses its Document. A new window requests an additional view.
    if (!newWindow) {
        for (const auto& [id, binding] : bindings_) {
            if (controller_.documentOf(binding.view) == opened.value()) {
                activate(*binding.window, binding.view);
                binding.window->widget->showNormal();
                binding.window->widget->raise();
                binding.window->widget->activateWindow();
                refreshRecentDocuments();
                return;
            }
        }
    }
    attach(newWindow || window.light ? createWindow() : window, controller_.createDocumentView(opened.value()));
}

void QtUIController::openFilesFromCommandLine(const QStringList& paths) {
    for (const auto& path : paths) {
        openDocument(mainWindow(), path, false);
    }
    if (paths.isEmpty()) {
        for (auto& window : windows_) {
            if (window->closing || !window->widget) {
                continue;
            }
            window->widget->showNormal();
            window->widget->raise();
            window->widget->activateWindow();
            return;
        }
        createDocument(mainWindow(), QStringLiteral("Untitled"), false);
    }
}

bool QtUIController::save(app::DocumentId document, QWidget* parent, bool saveAs) {
    if (document == app::InvalidDocumentId) {
        return false;
    }
    auto path = controller_.documentPath(document);
    if (saveAs || path.empty()) {
        QString proposed = path.empty() ? title(document) : displayPath(path);
        if (path.empty() && !proposed.endsWith(QStringLiteral(".ourp"), Qt::CaseInsensitive)) {
            proposed += QStringLiteral(".ourp");
        }
        auto selected = QFileDialog::getSaveFileName(parent, tr("Save document"), proposed, tr("OurPaint documents (*.ourp)"));
        if (selected.isEmpty()) {
            return false;
        }
        if (!selected.endsWith(QStringLiteral(".ourp"), Qt::CaseInsensitive)) {
            selected += QStringLiteral(".ourp");
            // The dialog confirmed the original spelling, not this completed path.
            if (QFileInfo::exists(selected) && QMessageBox::question(parent, tr("Replace document"), tr("Replace %1?").arg(selected),
                                                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
                return false;
            }
        }
        path = nativePath(selected);
    }
    const auto saved = controller_.saveDocumentAs(document, path);
    if (!saved) {
        showError(saved.error(), parent);
        return false;
    }
    updateTitles(document);
    refreshRecentDocuments();
    if (controller_.metadataError()) {
        QMessageBox::warning(parent, tr("Recent documents"), tr("The document was saved, but .ourpaint could not be updated."));
    }
    return true;
}

bool QtUIController::confirmClose(app::DocumentId document, QWidget* parent) {
    if (!controller_.documentModified(document)) {
        return true;
    }
    const auto choice = QMessageBox::warning(parent, tr("Unsaved changes"), tr("Save changes to %1?").arg(title(document)),
                                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    return choice == QMessageBox::Discard || (choice == QMessageBox::Save && save(document, parent));
}

void QtUIController::closeView(app::ViewId view) {
    const auto found = bindings_.find(token(view));
    if (found == bindings_.end() || closingDecision_) {
        return;
    }
    QScopedValueRollback guard(closingDecision_, true);
    auto& window = *found->second.window;
    const auto document = controller_.documentOf(view);
    const bool last = controller_.documentViews(document).size() == 1;
    controller_.endInteractions();
    if (last && !confirmClose(document, window.widget)) {
        return;
    }
    controller_.closeDocumentView(view);
    if (last) {
        controller_.closeDocument(document);
    }
    if (window.light) {
        // Use close(), rather than just hide(), so Qt's last-window policy applies.
        window.closing = true;
        window.widget->close();
        retire(window);
    } else if (window.active != app::InvalidViewId) {
        activate(window, window.active);
    }
}

bool QtUIController::eventFilter(QObject* object, QEvent* event) {
    auto found = std::find_if(windows_.begin(), windows_.end(), [object](const auto& w) { return w->widget == object; });
    if (found == windows_.end()) {
        return QObject::eventFilter(object, event);
    }
    auto& window = **found;
    if (event->type() == QEvent::WindowActivate && !window.closing && window.active != app::InvalidViewId) {
        controller_.setActiveDocumentView(window.active);
    }
    if (event->type() != QEvent::Close || window.closing) {
        return QObject::eventFilter(object, event);
    }
    auto* close = static_cast<QCloseEvent*>(event);
    if (closingDecision_) {
        close->ignore();
        return true;
    }
    QScopedValueRollback guard(closingDecision_, true);
    controller_.endInteractions();
    std::vector<app::ViewId> closingViews;
    std::set<app::DocumentId> closingDocuments;
    for (const auto& [id, binding] : bindings_) {
        if (binding.window == &window) {
            closingViews.push_back(binding.view);
        }
    }
    for (auto view : closingViews) {
        const auto document = controller_.documentOf(view);
        const auto views = controller_.documentViews(document);
        if (std::all_of(views.begin(), views.end(), [&](auto v) { return std::find(closingViews.begin(), closingViews.end(), v) != closingViews.end(); })) {
            closingDocuments.insert(document);
        }
    }
    // Ask every question before destroying any view. Cancel leaves the window intact.
    for (auto document : closingDocuments) {
        if (!confirmClose(document, window.widget)) {
            close->ignore();
            return true;
        }
    }
    window.closing = true;
    for (auto view : closingViews) {
        controller_.closeDocumentView(view);
    }
    for (auto document : closingDocuments) {
        controller_.closeDocument(document);
    }
    // Defer destruction without hiding before QWidget processes this Close event.
    QTimer::singleShot(0, this, [this, pointer = &window] { destroyWindow(pointer); });
    return false;
}

app::ViewId QtUIController::viewFor(qint64 id) const {
    const auto found = bindings_.find(id);
    return found == bindings_.end() ? app::InvalidViewId : found->second.view;
}

void QtUIController::rename(qint64 id, const QString& name) {
    const auto view = viewFor(id);
    if (view == app::InvalidViewId) {
        mainWindow().ui->addNotification(tr("Recent file names cannot be changed here. Open the document and use Save As."));
        return;
    }
    const auto document = controller_.documentOf(view);
    if (!controller_.documentPath(document).empty()) {
        bindings_.at(id).window->ui->addNotification(tr("Use Save As to change the file name."));
    } else if (!name.trimmed().isEmpty()) {
        controller_.renameDocument(document, name.trimmed().toStdString());
    }
    updateTitles(document);
}

void QtUIController::updateTitles(app::DocumentId document) {
    for (const auto& [id, binding] : bindings_) {
        if (controller_.documentOf(binding.view) != document) {
            continue;
        }
        UI::FileData data{id, title(document)};
        const QSignalBlocker blocker(binding.window->ui.get());
        binding.window->ui->renameTabSlot(data);
        if (binding.window->active != app::InvalidViewId) {
            binding.window->ui->setActiveTab(token(binding.window->active));
        }
    }
}

void QtUIController::refreshRecentDocuments() {
    QVector<UI::ProjectData> recent;
    recentPaths_.clear();
    qint64 id = -2;  // Distinct from positive ViewIds and the UI's invalid token (-1).
    for (const auto& path : controller_.recentDocuments()) {
        const auto value = displayPath(path);
        recentPaths_.emplace(id, value);
        recent.push_back({id--, QFileInfo(value).fileName(), value});
    }
    for (auto& window : windows_) {
        window->ui->setRecentDocuments(recent);
    }
}

void QtUIController::presentTool(app::ViewId view) {
    const auto found = bindings_.find(token(view));
    if (found == bindings_.end() || found->second.window->active != view) {
        return;
    }
    if (const auto tool = controller_.activeToolId(view)) {
        UIObserver::presentActiveTool(*found->second.window->ui, *tool);
    }
}

void QtUIController::selectTool(Window& window, qint64 id, ToolId tool, std::optional<double> value) {
    const auto view = viewFor(id);
    const auto found = bindings_.find(id);
    if (found == bindings_.end() || found->second.window != &window) {
        return;
    }
    activate(window, view);
    controller_.selectTool(view, tool, value);
    window.viewport->requestActivate();
    window.viewport->update();
}

void QtUIController::applyLanguage() {
    qApp->removeTranslator(&translator_);
    const auto language = settings_.language();
    if (language == UI::SettingsSystem::Language::English) {
        return;
    }
    const auto resource =
        language == UI::SettingsSystem::Language::Russian ? QStringLiteral(":/translations/app_ru.qm") : QStringLiteral(":/translations/app_by.qm");
    if (translator_.load(resource)) {
        qApp->installTranslator(&translator_);
    }
}

void QtUIController::showError(const app::FileError& error, QWidget* parent) {
    QString message = displayPath(error.path);
    if (error.systemError) {
        message += QStringLiteral("\n") + QString::fromStdString(error.systemError.message());
    }
    if (error.codecError) {
        message += QStringLiteral("\n") + QString::fromStdString(error.codecError->message);
    }
    QMessageBox::warning(parent, tr("Document operation failed"), message);
}

void QtUIController::primitive(Window& window, qint64 id, UI::PrimitiveType type) {
    switch (type) {
        case UI::PrimitiveType::Point:
            selectTool(window, id, ToolId::Point);
            break;
        case UI::PrimitiveType::Line:
            selectTool(window, id, ToolId::Line);
            break;
        case UI::PrimitiveType::Polyline:
            selectTool(window, id, ToolId::Polyline);
            break;
        case UI::PrimitiveType::InfiniteLine:
            selectTool(window, id, ToolId::InfiniteLine);
            break;
        case UI::PrimitiveType::CircleByRadius:
            selectTool(window, id, ToolId::CircleByRadius);
            break;
        case UI::PrimitiveType::CircleByDiameter:
            selectTool(window, id, ToolId::CircleByDiameter);
            break;
        case UI::PrimitiveType::CircleByTwoPoints:
            selectTool(window, id, ToolId::CircleByTwoPoints);
            break;
        case UI::PrimitiveType::CircleByThreePoints:
            selectTool(window, id, ToolId::CircleByThreePoints);
            break;
        case UI::PrimitiveType::CircleTangentTwoLines:
            selectTool(window, id, ToolId::CircleTangentTwoLines);
            break;
        case UI::PrimitiveType::CircleTangentThreeLines:
            selectTool(window, id, ToolId::CircleTangentThreeLines);
            break;
        case UI::PrimitiveType::ArcByRadius:
            selectTool(window, id, ToolId::ArcByRadius);
            break;
        case UI::PrimitiveType::ArcByDiameter:
            selectTool(window, id, ToolId::ArcByDiameter);
            break;
        case UI::PrimitiveType::ArcThreePoints:
            selectTool(window, id, ToolId::ArcByThreePoints);
            break;
        case UI::PrimitiveType::CubicBezier:
            selectTool(window, id, ToolId::Nurbs);
            break;
        default:
            window.ui->addNotification(tr("These tool settings are not available yet."));
            break;
    }
}

void QtUIController::constraint(Window& window, qint64 id, UI::ConstraintType type) {
    ToolId tool;
    switch (type) {
        case UI::ConstraintType::Distance:
            tool = ToolId::Distance;
            break;
        case UI::ConstraintType::HDistance:
            tool = ToolId::HorizontalDistance;
            break;
        case UI::ConstraintType::VDistance:
            tool = ToolId::VerticalDistance;
            break;
        case UI::ConstraintType::Angle:
            tool = ToolId::Angle;
            break;
        case UI::ConstraintType::Coincident:
            tool = ToolId::Coincident;
            break;
        case UI::ConstraintType::Horizontal:
            tool = ToolId::Horizontal;
            break;
        case UI::ConstraintType::Vertical:
            tool = ToolId::Vertical;
            break;
        case UI::ConstraintType::Parallel:
            tool = ToolId::Parallel;
            break;
        case UI::ConstraintType::Perpendicular:
            tool = ToolId::Perpendicular;
            break;
        case UI::ConstraintType::Tangent:
            tool = ToolId::Tangent;
            break;
        case UI::ConstraintType::Equal:
            tool = ToolId::Equal;
            break;
        case UI::ConstraintType::Fix:
            tool = ToolId::Fix;
            break;
        case UI::ConstraintType::Concentric:
            tool = ToolId::Concentric;
            break;
        case UI::ConstraintType::Midpoint:
            tool = ToolId::Midpoint;
            break;
        case UI::ConstraintType::Collinear:
            tool = ToolId::Collinear;
            break;
        case UI::ConstraintType::Curvature:
            tool = ToolId::Curvature;
            break;
        case UI::ConstraintType::Symmetric:
            tool = ToolId::Symmetric;
            break;
        default:
            return;
    }
    std::optional<double> value;
    if (tool == ToolId::Distance || tool == ToolId::HorizontalDistance || tool == ToolId::VerticalDistance || tool == ToolId::Angle) {
        bool accepted = false;
        const bool angle = tool == ToolId::Angle;
        const double entered = QInputDialog::getDouble(window.widget, tr("Constraint"), angle ? tr("Angle (degrees):") : tr("Distance:"), angle ? 90.0 : 10.0,
                                                       0.000001, angle ? 360.0 : 1e12, 6, &accepted);
        if (!accepted) {
            return;
        }
        value = entered;
    }
    selectTool(window, id, tool, value);
}
