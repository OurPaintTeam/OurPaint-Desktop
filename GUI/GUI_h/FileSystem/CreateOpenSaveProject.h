#ifndef OURPAINT_SAVELOADPROJECT_H
#define OURPAINT_SAVELOADPROJECT_H

#include <QStandardPaths>
#include <QString>
#include  <QPushButton>
#include <QObject>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>

class ModeManager;
class InputWindow;
class MainWindow;

class CreateOpenSaveProject final : public QObject {
    Q_OBJECT
private:
    MainWindow* mainWindow = nullptr;

    const QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString listProjectsPath = QDir::cleanPath(documentsPath + "/OurPaint/list_projects.set");
    const QString defaultProjectsPath = QDir::cleanPath(documentsPath + "/OurPaint/projects");
    QString projectPath;

    QString openDirectoryForOpenedProject() const;

public:
    explicit CreateOpenSaveProject(MainWindow* mw);
    void initListProjectStartWindow();
    QString getProjectPath();

    void saveSettings(const QString& path) const;
    void saveProject();

    void openProject(const QString &projectPath_);
    void loadSettings(const QString& path) const;
    void createFile(const QString &fileName) const;

    void deleteAllProject() const;

public slots:
    void slotOpenProject();
    void createProject();

signals:
    void ChangeTabs(const QString& chars);
    void OpenProject(const QString& chars);
    void SaveProject(const QString& chars);
};

#endif //OURPAINT_SAVELOADPROJECT_H