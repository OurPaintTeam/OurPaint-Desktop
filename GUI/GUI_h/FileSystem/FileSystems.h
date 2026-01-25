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

class MainWindow;

class FileSystems final : public QObject {
    Q_OBJECT
public:
    explicit FileSystems(MainWindow* mw);

    QString getProjectPath() const;
    void scanAndLoadProjects();

public slots:
    void openOrCreateProject();
    void saveProject();
    void createNewFile(const QString& ourpFileName);

    //  Documents/OurPaint/projects/nameProject/ name.ourp , name2.ourp
    signals:
    void DeleteTab(const QString& tabName); // name.ourp
    void CreateNewTab(const QString& tabName); // name.ourp
    void OpenProject(const QString& workDir); // path/project
    void ChangeTabs(const QString& tabName); // name.ourp
    void SaveProject(const QString& workDir); // path/project

private:
    MainWindow* mainWindow = nullptr;
    QString projectPath;

    const QString documentsPath =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString defaultProjectsPath =
        QDir::cleanPath(documentsPath + "/OurPaint/projects");

    QString selectProjectDirectory() const;

    void createNewProject();
    void createTab(const QString& ourpFileName);
    void openExistingProject();
};
#endif //OURPAINT_FILESYSTEMS_H