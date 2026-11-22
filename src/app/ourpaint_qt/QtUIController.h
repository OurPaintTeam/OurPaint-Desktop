#pragma once

#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QTranslator>
#include <map>
#include <memory>
#include <vector>

#include "Lib/Core/ProjectManager.h"
#include "Lib/DocumentSystems/SettingsSystem.h"
#include "UIController.h"
#include "UIObserver.h"

class QtViewport;

// Owns Qt presentation only. All CAD/document/view operations go through App's
// two entry points; UI tab tokens identify views, never database rows or paths.
class QtUIController final : public QObject {
    Q_OBJECT
public:
    explicit QtUIController(app::UIController& controller, const QString& configDirectory);
    ~QtUIController() override;
    void openFilesFromCommandLine(const QStringList& paths);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private:
    struct Window {
        std::unique_ptr<QtViewport> pendingViewport;
        std::unique_ptr<UI::ProjectManager> ui;
        QPointer<QtViewport> viewport;
        QPointer<QWidget> widget;
        app::ViewId active = app::InvalidViewId;
        bool light = false;
        bool closing = false;
        ~Window();
    };
    struct Binding {
        Window* window;
        app::ViewId view;
    };
    Window& createWindow(bool light = false);
    Window& mainWindow();
    void connectWindow(Window& window);
    void installWindowActions(Window& window);
    void attach(Window& window, app::ViewId view);
    void detach(app::ViewId view);
    void activate(Window& window, app::ViewId view);
    void moveView(app::ViewId view, Window& target);
    void retire(Window& window);
    void destroyWindow(Window* window);
    void createDocument(Window& window, const QString& name, bool newWindow);
    void openDocument(Window& window, const QString& path, bool newWindow);
    void closeView(app::ViewId view);
    bool confirmClose(app::DocumentId document, QWidget* parent);
    bool save(app::DocumentId document, QWidget* parent, bool saveAs = false);
    void rename(qint64 token, const QString& name);
    void updateTitles(app::DocumentId document);
    void refreshRecentDocuments();
    void presentTool(app::ViewId view);
    void selectTool(Window& window, qint64 token, ToolId tool, std::optional<double> value = {});
    void primitive(Window& window, qint64 token, UI::PrimitiveType type);
    void constraint(Window& window, qint64 token, UI::ConstraintType type);
    void applyLanguage();
    void showError(const app::FileError& error, QWidget* parent);
    app::ViewId viewFor(qint64 token) const;
    QString title(app::DocumentId document) const;

    app::UIController& controller_;
    UIObserver observer_;
    UI::SettingsSystem settings_;
    QTranslator translator_;
    std::vector<std::unique_ptr<Window>> windows_;
    std::map<qint64, Binding> bindings_;
    std::map<qint64, QString> recentPaths_;
    bool closingDecision_ = false;
};
