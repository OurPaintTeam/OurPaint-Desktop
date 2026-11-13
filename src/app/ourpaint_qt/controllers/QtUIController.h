#ifndef OURPAINT_APPLICATION_QTUICONTROLLER_H_
#define OURPAINT_APPLICATION_QTUICONTROLLER_H_

#include <QObject>
#include <QPointer>
#include <QTranslator>
#include <unordered_map>

#include "DocumentView.h"
#include "FileSystem.h"
#include "ProjectManager.h"
#include "QtViewportHost.h"
#include "Settings.h"
#include "SettingsSystem.h"
#include "ToolsType.h"

class QtViewportHost;
class QtPlatformRuntime;
class IPlatformRuntime;
class DocumentController;
class QtUIController : public QObject {
    Q_OBJECT

public:
    QtUIController(DocumentController& docController,
                   QtPlatformRuntime& platformRuntime, Settings& settings);
    ~QtUIController() override;

public slots:

    bool createFile(UI::ProjectManager* currentManager, const QString& path, bool newWindow);
    bool openFile(UI::ProjectManager* currentManager, const QString& path, bool newWindow);
    bool openFileByID(UI::ProjectManager* currentManager, qint64 id, bool newWindow);
    bool removeFile(UI::ProjectManager* currentManager, const qint64 id);
    bool closeFile(UI::ProjectManager* currentManager, const qint64 id);
    bool setActiveFile(UI::ProjectManager* currentManager, qint64 fileId);

    bool lightEditorRequested(UI::ProjectManager* currentManager, const qint64 id);
    bool returnTabToEditorRequested(UI::ProjectManager* currentManager, qint64 fileId);
    bool handleTabMove(qint64 fileId, quintptr sourceWindowPtr, quintptr targetWindowPtr);
    bool closeLightEditorWindowTriggered(UI::ProjectManager* currentManager);
    bool saveFullProject(UI::ProjectManager* currentManager, bool close);

    void sentCommand(UI::ProjectManager* currentManager, const qint64 id, const QString command);
    void primitive(UI::ProjectManager* currentManager, const qint64 id, UI::PrimitiveType type);
    void constraint(UI::ProjectManager* currentManager, const qint64 id, UI::ConstraintType type);
    void tools(UI::ProjectManager* currentManager, const qint64 id, UI::ToolsType type);
    void script(UI::ProjectManager* currentManager, const QString& text);
    void settingsChanged(UI::ProjectManager* currentManager);

    void openFilesFromCommandLine(const QStringList& filePaths);

private:
    UI::ProjectManager* createBaseManager(const UI::EditorContext& context);
    UI::ProjectManager* createLightManager(const UI::EditorContext& context);
    UI::EditorContext createEditorContext(bool isNewWindow = false) const;

    bool attachDocument(UI::ProjectManager& manager, DocumentId id, qint64 fileID);
    bool closeDocumentView(DocumentId id);
    bool closeOneDocument(DocumentId docId, UI::ProjectManager* currentManager);

    void initBaseSignals(UI::ProjectManager& manager);
    void initLightManager(UI::ProjectManager& manager);
    void deleteManager(UI::ProjectManager* manager);
    void applyLanguage(UI::SettingsSystem::Language lang);
    void presentActiveTool(UI::ProjectManager* currentManager, DocumentId id, ToolId tool);

    void selectTool(ToolId tool, std::optional<double> value, const std::string& path = {});
    void showConstraintPrompt(const QString& promptText, ToolId constraintType, std::string path);

    bool renameFile(UI::ProjectManager* currentManager, UI::FileData& data);
    QString getFilePath(qint64 fileId);

private:
    DocumentController& docController_;
    QtPlatformRuntime& platformRuntime_;
    Settings& app_settings_;

    UI::FileSystem fs_;
    UI::SettingsSystem ui_settings_;
    QTranslator* translator_{nullptr};
    UI::SettingsSystem::Language currentLanguage_{UI::SettingsSystem::Language::English};

    QPointer<QtViewportHost> activeHost_;
    std::unordered_map<UI::ProjectManager*, QPointer<QtViewportHost>> managerHosts_;
    std::unordered_map<DocumentId, QPointer<QtViewportHost>> documentHosts_;

    QVector<UI::ProjectManager*> projectManagers_;
};

#endif // OURPAINT_APPLICATION_QTUICONTROLLER_H_