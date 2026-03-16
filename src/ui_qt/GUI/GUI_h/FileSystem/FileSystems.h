#ifndef OURPAINT_FILESYSTEMS_H
#define OURPAINT_FILESYSTEMS_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QFileDialog>
#include <QSet>
#include <QStandardPaths>
#include <qxmlstream.h>

#include "ui_mainwindow.h"

#define LOG_INFO(x)  qDebug()    << "[PROJECT]" << x
#define LOG_WARN(x)  qWarning()  << "[PROJECT WARNING]" << x
#define LOG_ERROR(x) qCritical() << "[PROJECT ERROR]" << x

class MainWindow;

class FileSystems final : public QObject {
    Q_OBJECT
public:
    explicit FileSystems(MainWindow* mw);
    QString getProjectPath() const;
    void loadProjectsToUI();

public slots:
    void slotOpenProject();
    void slotCreateNewProject();
    void slotSaveProject();
    void slotCreateNewFile(const QString& ourpFileName);

    //  Documents/OurPaint/projects/nameProject/ name.ourp , name2.ourp
    signals:
    void DeleteTab(const QString& tabName); // name.ourp
    void CloseTab(const Ui_MainWindow::TabWidget* tabWidget);
    void RenameTab(const QString& oldName,const QString& newName); // name.ourp
    void CreateNewTab(const QString& tabName); // name.ourp
    void CreateNewProject(const QString& workDir); // path/project
    void OpenProject(const QString& workDir); // path/project
    void ChangeTabs(const QString& tabName); // name.ourp
    void OpenFile(const QString& fileName); // name.ourp
    void SaveProject();

private:
    MainWindow* mainWindow = nullptr;
    QString projectPath;

    const QString documentsPath =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString defaultProjectsPath =
        QDir::cleanPath(documentsPath + "/OurPaint/projects");
    const QString configPath =
    QDir::cleanPath(documentsPath + "/OurPaint/config/config.xml");

    QString selectProjectDirectory() const;

    void openProjectWithDirPath(const QString& workDir);
    void openOurpFiles();
    void createTabButtons(const QString& ourpFileName);
    void saveProjectToXML() const;
    bool projectExistsInConfig(const QString &absoluteProjectPath) const;
};
#endif //OURPAINT_FILESYSTEMS_H