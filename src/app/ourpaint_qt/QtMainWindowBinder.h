#ifndef OURPAINT_APPLICATION_QT_MAINWINDOW_BINDER_H_
#define OURPAINT_APPLICATION_QT_MAINWINDOW_BINDER_H_

#include <QObject>
#include <functional>
#include <QPointer>

#include "../editor/UIController.h"
#include "Lib/Core/ProjectManager.h"

class QtViewportHost;

class QtMainWindowBinder : public QObject {
    Q_OBJECT
public:
    explicit QtMainWindowBinder(UI::ProjectManager& window,
                                UIController& controller,
                                QtViewportHost& host,
                                QObject* parent = nullptr);

    ~QtMainWindowBinder() override;
    bool openProjectInCurrentWindow(const QString& path = {});
    bool createProjectInCurrentWindow(const QString& path = {});

private:
    void openProjectInNewWindow(const QString& path);
    void createProjectInNewWindow(const QString& path);
    void openFile(const QString& path);
    bool saveProject();
    bool saveProjectAs();
    bool confirmClose();
    void updateProjectData();
    bool eventFilter(QObject* watched, QEvent* event) override;
    void invoke(const std::function<void()>& action);
    void installProjectActions();

    UI::ProjectManager& window_;
    UIController& controller_;
    QPointer<QtViewportHost> host_;
};

#endif // ! OURPAINT_APPLICATION_QT_MAINWINDOW_BINDER_H_
