#include "../ourpaint_qt/UIController.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "../../core/Document.h"
#include "../../core/DocumentManager.h"
#include "../../render/OpenGL2dRenderer.h"
#include "../ourpaint_qt/CommandConsole.h"
#include "../ourpaint_qt/QtViewportHost.h"
#include "ToolsType.h"
#include "ViewportController.h"
#include "render/AxisTexts.h"
#include "render/RenderSceneBuilder.h"

UIController::UIController(std::vector<std::unique_ptr<DocumentView>>& views,
                           DocumentManager& manager,
                           UI::ProjectManager& projectManager,
                           QtViewportHost& host)
    : views_(views),
      manager_(manager),
      viewportHost_(host),
      projectManager_(projectManager),
      activeTabName_({}) {}

UIController::~UIController() {
    for (auto& view : views_) {
        view->editorSession_.setToolChangedCallback({});
        //view->editorSession_.setReportCallback({});
        viewportHost_.removeController(&view->viewportController_);
    }
}

DocumentView* UIController::findView(const std::string& name) const {
    const auto entry = documentIds_.find(name);
    if (entry == documentIds_.end()) {
        return nullptr;
    }
    const Document* document = manager_.at(entry->second);
    for (const auto& view : views_) {
        if (&view->document() == document) {
            return view.get();
        }
    }
    return nullptr;
}

void UIController::selectTool(ToolId tool, std::optional<double> value, const std::string& tabName) {
    if (auto* view = findView(tabName.empty() ? activeTabName_ : tabName)) {
        view->editorSession_.selectTool(tool, value);
        view->viewportController_.requestRedraw();
        viewportHost_.requestRedraw();
    }
}

void UIController::presentActiveTool(DocumentId id, ToolId tool) {
    const auto entry = std::find_if(documentIds_.begin(), documentIds_.end(), [id](const auto& item) { return item.second == id; });
    if (entry == documentIds_.end()) {
        return;
    }
    const auto tabName = QString::fromStdString(entry->first);
    const auto present = [this, &tabName, active = entry->first == activeTabName_](auto uiTool) {
        projectManager_.setActiveToolTabWindow(tabName, uiTool);
        if (active) {
            projectManager_.setActiveToolProjectWindow(uiTool);
        }
    };

    switch (tool) {
        case ToolId::Cursor:
            present(UI::ToolsType::Cursor);
            break;
        case ToolId::Point:
            present(UI::PrimitiveType::Point);
            break;
        case ToolId::Line:
            present(UI::PrimitiveType::Line);
            break;
        case ToolId::Polyline:
            present(UI::PrimitiveType::Polyline);
            break;
        case ToolId::InfiniteLine:
            present(UI::PrimitiveType::InfiniteLine);
            break;
        case ToolId::CircleByRadius:
            present(UI::PrimitiveType::CircleByRadius);
            break;
        case ToolId::CircleByDiameter:
            present(UI::PrimitiveType::CircleByDiameter);
            break;
        case ToolId::CircleByTwoPoints:
            present(UI::PrimitiveType::CircleByTwoPoints);
            break;
        case ToolId::CircleByThreePoints:
            present(UI::PrimitiveType::CircleByThreePoints);
            break;
        case ToolId::CircleTangentTwoLines:
            present(UI::PrimitiveType::CircleTangentTwoLines);
            break;
        case ToolId::CircleTangentThreeLines:
            present(UI::PrimitiveType::CircleTangentThreeLines);
            break;
        case ToolId::ArcByRadius:
            present(UI::PrimitiveType::ArcByRadius);
            break;
        case ToolId::ArcByDiameter:
            present(UI::PrimitiveType::ArcByDiameter);
            break;
        case ToolId::ArcByThreePoints:
            present(UI::PrimitiveType::ArcThreePoints);
            break;
        case ToolId::Nurbs:
            present(UI::PrimitiveType::CubicBezier);
            break;
        case ToolId::Distance:
            present(UI::ConstraintType::Distance);
            break;
        case ToolId::HorizontalDistance:
            present(UI::ConstraintType::HDistance);
            break;
        case ToolId::VerticalDistance:
            present(UI::ConstraintType::VDistance);
            break;
        case ToolId::Angle:
            present(UI::ConstraintType::Angle);
            break;
        case ToolId::Coincident:
            present(UI::ConstraintType::Coincident);
            break;
        case ToolId::Horizontal:
            present(UI::ConstraintType::Horizontal);
            break;
        case ToolId::Vertical:
            present(UI::ConstraintType::Vertical);
            break;
        case ToolId::Parallel:
            present(UI::ConstraintType::Parallel);
            break;
        case ToolId::Perpendicular:
            present(UI::ConstraintType::Perpendicular);
            break;
        case ToolId::Tangent:
            present(UI::ConstraintType::Tangent);
            break;
        case ToolId::Equal:
            present(UI::ConstraintType::Equal);
            break;
        case ToolId::FixUnfix:
            present(UI::ConstraintType::FixUnfix);
            break;
        case ToolId::Concentric:
            present(UI::ConstraintType::Concentric);
            break;
        case ToolId::Midpoint:
            present(UI::ConstraintType::Midpoint);
            break;
        case ToolId::Collinear:
            present(UI::ConstraintType::Collinear);
            break;
        case ToolId::Symmetric:
            present(UI::ConstraintType::Symmetric);
            break;
    }
}

void UIController::executeConsoleCommand(std::string str) {
    (void)str;
    try {
        const auto entry = documentIds_.find(activeTabName_);
        Document* document = entry == documentIds_.end() ? nullptr : manager_.at(entry->second);
        // UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
        // CommandManager& cm = document->commandManager();
        // Transaction* txn = cm.invoke(str);
        // urm.push(std::move(*txn));

        if (document) {
            viewportHost_.requestRedraw();
        }
    }catch (const std::exception& e) {
        throw std::runtime_error(e.what());
    }
}

void UIController::createFile(const std::string& fileName) {
    if (fileName.empty()) {
        return;
    }
    if (findView(fileName)) {
        setActiveFile(fileName);
        return;
    }
    if (auto* previous = findView(activeTabName_)) {
        previous->editorSession_.activeTool()->cancel();
    }
    const auto existing = documentIds_.find(fileName);
    if (existing != documentIds_.end()) {
        attachDocument(existing->second, fileName);
        return;
    }
    const auto id = manager_.createNewDocument(fileName);
    try {
        const auto filename = nextFilename(id);
        project_.documents.push_back({filename, std::cref(*manager_.at(id))});
        attachDocument(id, fileName);
        projectModified_ = true;
    } catch (...) {
        std::erase_if(project_.documents, [this, id](const auto& item) { return &item.document.get() == manager_.at(id); });
        manager_.closeDocument(id);
        throw;
    }
}

void UIController::attachDocument(DocumentId id, const std::string& tabName) {
    auto view = std::make_unique<DocumentView>(*manager_.at(id));
    view->editorSession_.setToolChangedCallback([this, id](ToolId tool) { presentActiveTool(id, tool); });
    //view->editorSession_.setReportCallback([this, id](const ActionReport& report) { presentReport(id, report); });
    const int index = viewportHost_.addController(&view->viewportController_);
    if (index < 0) {
        throw std::runtime_error("Could not create a viewport for the document");
    }
    auto* controller = &view->viewportController_;
    const bool registered = documentIds_.contains(tabName);
    const auto previous = activeTabName_;
    try {
        documentIds_[tabName] = id;
        indicesMap_[tabName] = index;
        controllerMap_[controller] = index;
        views_.push_back(std::move(view));
        projectManager_.addTabSlot(QString::fromStdString(tabName));
        setActiveFile(tabName);
    } catch (...) {
        viewportHost_.removeController(controller);
        std::erase_if(views_, [controller](const auto& candidate) { return &candidate->viewportController_ == controller; });
        indicesMap_.erase(tabName);
        controllerMap_.erase(controller);
        if (!registered) {
            documentIds_.erase(tabName);
        }
        projectManager_.deleteTabSlot(QString::fromStdString(tabName));
        activeTabName_ = previous;
        throw;
    }
}

void UIController::setActiveFile(const std::string& fileName) {
    const auto* view = findView(fileName);
    if (!view) {
        return;
    }
    if (activeTabName_ != fileName) {
        if (auto* previous = findView(activeTabName_)) {
            previous->editorSession_.activeTool()->cancel();
        }
    }

    activeTabName_ = fileName;
    int index = indicesMap_[fileName];
    viewportHost_.setActiveController(index);
    projectManager_.setActiveTabNameProject(QString::fromStdString(fileName));
    presentActiveTool(documentIds_.at(fileName), view->editorSession_.activeToolId());
    viewportHost_.requestRedraw();
}

void UIController::removeFile(const std::string& fileName) {
    const auto* target = findView(fileName);
    for (auto it = views_.begin(); it != views_.end(); ++it) {
        if (it->get() == target) {
            (*it)->editorSession_.setToolChangedCallback({});
            //(*it)->editorSession_.setReportCallback({});
            const auto removed = controllerMap_.find(&(*it)->viewportController_);
            const int removedIndex = removed == controllerMap_.end() ? -1 : removed->second;
            viewportHost_.removeController(&(*it)->viewportController_);
            controllerMap_.erase(&(*it)->viewportController_);
            views_.erase(it);
            indicesMap_.erase(fileName);
            if (removedIndex >= 0) {
                for (auto& [name, index] : indicesMap_) {
                    if (index > removedIndex) {
                        --index;
                    }
                }
                for (auto& [controller, index] : controllerMap_) {
                    if (index > removedIndex) {
                        --index;
                    }
                }
            }
            if (activeTabName_ == fileName) {
                activeTabName_ = indicesMap_.empty() ? std::string{} : indicesMap_.begin()->first;
            }
            if (!activeTabName_.empty()) {
                setActiveFile(activeTabName_);
            }
            return;
        }
    }
}

void UIController::renameTab(const std::string& oldName, const std::string& newName) {
    auto* view = findView(oldName);
    if (!view) {
        return;
    }
    if (newName.empty() || (oldName != newName && documentIds_.contains(newName))) {
        projectManager_.renameTabSlot(QString::fromStdString(newName), QString::fromStdString(oldName));
        return;
    }
    int index = indicesMap_[oldName];
    indicesMap_.erase(oldName);
    indicesMap_[newName] = index;
    const auto id = documentIds_.at(oldName);
    documentIds_.erase(oldName);
    documentIds_[newName] = id;
    view->document().name() = newName;
    projectModified_ = true;
    if (activeTabName_ == oldName) {
        activeTabName_ = newName;
    }
    projectManager_.renameTabSlot(QString::fromStdString(oldName), QString::fromStdString(newName));
}

void UIController::renameProject() {}

void UIController::deleteProject() {}

void UIController::closeApplication() {}











