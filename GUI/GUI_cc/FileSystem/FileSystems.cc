#include "FileSystems.h"
#include "MainWindow.h"
#include "Modes.h"

#include <QDebug>
#include <QTextStream>
#include <QFileInfo>
#include <QDirIterator>
#include <QFileDialog>

#define LOG_INFO(x)  qDebug()    << "[PROJECT]" << x
#define LOG_WARN(x)  qWarning()  << "[PROJECT WARNING]" << x
#define LOG_ERROR(x) qCritical() << "[PROJECT ERROR]" << x


FileSystems::FileSystems(MainWindow* mw)
    : QObject(mw),
      mainWindow(mw)
{}


QString FileSystems::selectProjectDirectory() const {
    const QString dirPath = QFileDialog::getExistingDirectory(
        mainWindow,
        tr("Выберите директорию проекта"),
        defaultProjectsPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (dirPath.isEmpty()) {
        LOG_INFO("Выбор проекта отменён пользователем");
        return "";
    }

    LOG_INFO("Выбрана директория проекта:" << dirPath);
    return dirPath;
}


void FileSystems::openOrCreateProject() {
    const QString dirPath = selectProjectDirectory();
    if (dirPath.isEmpty()) {
        return;
    }

    const QDir projectDir(dirPath);
    if (!projectDir.exists()) {
        LOG_ERROR("Директория не существует:" << dirPath);
        mainWindow->showError("Директория проекта не существует");
        return;
    }

    projectPath = projectDir.absolutePath();
    LOG_INFO("Рабочая директория:" << projectPath);

    const QStringList ourpFiles = projectDir.entryList({ "*.ourp" }, QDir::Files);
    if (ourpFiles.isEmpty()) {
        LOG_INFO("Нет .ourp файлов → создаём новый проект");
        createNewProject();
    } else {
        LOG_INFO("Найдены .ourp файлы → открываем проект");
        openExistingProject();
    }

    ModeManager::setProject(true);
    mainWindow->ui->inProject();
}


void FileSystems::createNewProject() {
    if (projectPath.isEmpty()) {return;}

    const QDir projectDir(projectPath);
    const QString projectName = projectDir.dirName();

    if (projectName.isEmpty() || projectName.contains('.')) {
        LOG_ERROR("Неверное имя проекта:" << projectName);
        mainWindow->showError("Имя проекта не должно быть пустым и содержать '.'");
        return;
    }

    const QString ourpFileName = projectName + ".ourp";
    LOG_INFO("Создание нового проекта:" << projectName);

    createTab(ourpFileName);

    ModeManager::setSave(false);
}


void FileSystems::createNewFile(const QString& ourpFileName) { // name.ourp
    if (projectPath.isEmpty()) {
        return;
    }

    const QString ourpFilePath = projectPath + '/' + ourpFileName;

    if (QFile::exists(ourpFilePath)) {
        LOG_WARN("Файл проекта уже существует:" << ourpFilePath);
        return;
    }

    createTab(ourpFileName);

    emit CreateNewTab(ourpFileName);

}

void FileSystems::createTab(const QString& ourpFileName) { // name.ourp
    const auto* tabButton = mainWindow->ui->createTabProject(ourpFileName);
    connect(tabButton, &QPushButton::clicked, [this, ourpFileName]() {
        LOG_INFO("Переключение на вкладку:" << ourpFileName);
        emit ChangeTabs(ourpFileName); // name.ourp
    });
}


void FileSystems::openExistingProject() {
    if (projectPath.isEmpty()) {
        return;
    }

    const QDir projectDir(projectPath);
    const QStringList ourpFiles = projectDir.entryList({ "*.ourp" }, QDir::Files);
    if (ourpFiles.isEmpty()){ return;}

    for (const QString& fileName : ourpFiles) {
        createTab(fileName); // name.ourp
    }

    emit OpenProject(projectPath);

    ModeManager::setSave(true);
}


void FileSystems::saveProject() {
    if (projectPath.isEmpty()) {
        LOG_WARN("Нет проекта для сохранения");
        return;
    }

    emit SaveProject(projectPath);

    if (!ModeManager::getSave()) {
        LOG_INFO("Сохранение не требуется");
        return;
    }
}


QString FileSystems::getProjectPath() const {
    return projectPath;
}


void FileSystems::scanAndLoadProjects() {
    const QDir projectsDir(defaultProjectsPath);

    if (!projectsDir.exists()) {
        LOG_WARN("Директория проектов не найдена:" << defaultProjectsPath);
        return;
    }

    QFileInfoList dirs = projectsDir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot
    );

    for (const QFileInfo& dirInfo : dirs) {
        const QDir projectDir(dirInfo.absoluteFilePath());
        const QString projectName = projectDir.dirName();

        QStringList projectFiles = projectDir.entryList(
            QStringList() << "*.ourp",
            QDir::Files | QDir::NoSymLinks
        );

        if (projectFiles.isEmpty()) {
            LOG_WARN("В каталоге нет .ourp файлов, пропускаем:" << projectDir.absolutePath());
            continue;
        }

        const QString firstOurp = projectFiles.first();
        const QString fullPath = projectDir.filePath(firstOurp);

        const auto button = mainWindow->ui->addProjectInListStartWindow(projectName, projectDir.absolutePath());

        connect(button, &QPushButton::clicked, [this, fullPath]() {
            if (fullPath.isEmpty()) {
                LOG_ERROR("Empty project path при открытии");
                mainWindow->showError("Путь проекта пустой");
                return;
            }
            projectPath = fullPath;
            openExistingProject();
        });

        LOG_INFO("Проект загружен в UI:" << projectName << "->" << fullPath);
    }

    LOG_INFO("Сканирование проектов завершено");
}