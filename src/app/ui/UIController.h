#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <QPointer>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "../../core/DocumentManager.h"
#include "../editor/SketchEditor.h"
#include "../editor/tools/ToolId.h"
#include "../platform/IPlatformRuntime.h"
#include "../platform/IViewportHost.h"
#include "../platform/QtViewportHost.h"
#include "../project/DocumentView.h"
#include "../project/ProjectSaveLoad.h"
#include "Lib/Core/ProjectManager.h"
#include "SettingsSystem.h"

class QTranslator;
class UIController : public QObject {
    Q_OBJECT

public slots:
    void createFile(UI::ProjectManager* currentManager, const QString& path, bool newWindow);
    void openFile(UI::ProjectManager* currentManager, const QString& path, bool newWindow);
    void openFileByID(UI::ProjectManager* currentManager, qint64 id, bool newWindow);
    void renameFile(UI::ProjectManager* currentManager, UI::FileData& data);
    bool saveDocument(DocumentId id, const QString& path);
    void closeOneDocument(DocumentId docId, UI::ProjectManager* currentManager);
    void removeFile(UI::ProjectManager* currentManager, const qint64 id);
    void closeDocumentView(DocumentId id);
    std::size_t pathHash(const std::string& path) const;
    DocumentId idFromPath(const std::string& path) const;
    void closeFile(UI::ProjectManager* currentManager, const qint64 id);
    void setActiveFile(UI::ProjectManager* currentManager, const qint64 id);

    void sentCommand(UI::ProjectManager* currentManager, const qint64 id, const QString command);
    void primitive(UI::ProjectManager* currentManager, const qint64 id, UI::PrimitiveType type);
    void constraint(UI::ProjectManager* currentManager, const qint64 id, UI::ConstraintType type);
    void tools(UI::ProjectManager* currentManager, const qint64 id, UI::ToolsType type);
    void script(UI::ProjectManager* currentManager, const QString& text);

    void settingsChanged(UI::ProjectManager* currentManager);
    void saveFullProject(UI::ProjectManager* currentManager, bool close);

    void lightEditorRequested(UI::ProjectManager* currentManager, const qint64 id);
    void openLightEditor(UI::ProjectManager* currentManager, UI::FileData& data);
    void returnTabToEditorRequested(UI::ProjectManager* currentManager, qint64 fileId);
    void closeLightEditorWindowTriggered(UI::ProjectManager* currentManager);

public:
    void openFilesFromCommandLine(const QStringList& filePaths);
    UIController(std::vector<std::unique_ptr<DocumentView>>& views, DocumentManager& manager, IPlatformRuntime& platformRuntime);
    ~UIController();

private:
    qint64 getFileID(const QString& filePath);
    QString getFilePath(qint64 fileId);
    QString getFileName(qint64 fileId);
    UI::FileSystem::ProjectData getProjectData(qint64 fileId);

    UI::ProjectManager* createBaseManager(const UI::EditorContext& context);
    UI::ProjectManager* createLightManager(const UI::EditorContext& context);
    UI::EditorContext createEditorContext(bool isNewWindow = false) const;
    void applyLanguage(UI::SettingsSystem::Language lang);

    void selectTool(ToolId tool, std::optional<double> value, const std::string& tabName = {});
    void showConstraintPrompt(const QString& promptText, ToolId constraintType, std::string path);
    void deleteManager(UI::ProjectManager* manager);

    bool loadDocument(const QString& path, UI::ProjectManager* currentManager);
    void attachDocument(UI::ProjectManager& manager, DocumentId id, qint64 fileID);
    void presentActiveTool(UI::ProjectManager* currentManager, DocumentId id, ToolId tool);

    DocumentView* findView(const std::string& path) const;
    DocumentView* findViewByName(const std::string& name) const;
    DocumentView* findViewByHash(std::size_t hash) const;

    void initBaseSignals(UI::ProjectManager& manager);
    void initLightManager(UI::ProjectManager& manager);
    void handleTabMove(qint64 fileId, quintptr sourceWindowPtr, quintptr targetWindowPtr);

    std::vector<std::unique_ptr<DocumentView>>& views_;
    DocumentManager& manager_;
    IPlatformRuntime& platformRuntime_;


    bool projectModified_ = true;

    QPointer<QtViewportHost> activeHost_{nullptr};
    std::unordered_map<UI::ProjectManager*, QPointer<QtViewportHost>> managerHosts_;
    std::unordered_map<DocumentId, QPointer<QtViewportHost>> documentHosts_;
    std::unordered_map<std::size_t, DocumentId> documentIds_;

    std::unordered_map<std::size_t, int> indicesMap_;
    std::unordered_map<ViewportController*, int> controllerMap_;
    std::size_t activeTabHash_{0};

    UI::FileSystem fs_;
    UI::SettingsSystem settings_;
    QVector<UI::ProjectManager*> projectManagers_;

    QTranslator* translator_{nullptr};
    UI::SettingsSystem::Language currentLanguage_{UI::SettingsSystem::Language::English};
};

#endif  // ! OURPAINT_APPLICATION_UI_CONTROLLER_H_