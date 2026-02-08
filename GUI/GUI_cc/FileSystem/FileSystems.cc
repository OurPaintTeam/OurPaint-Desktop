#include "FileSystems.h"

#include "MainWindow.h"
#include "Modes.h"


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
    if (projectExistsInConfig(dirPath)) {
        mainWindow->showError("Проект уже сущетсвует !");
        LOG_INFO("Проект уже существует:" << dirPath);
        return "";
    }

    return dirPath;
}


void FileSystems::slotOpenProject() {
    if (ModeManager::getProject()) {
        LOG_ERROR("Вы в проекте:" << projectPath);
        mainWindow->showError("Выйдите из проекта!");
        return;
    }

    const QString dirPath = selectProjectDirectory();

    if (dirPath.isEmpty()) {
        return;
    }

    openProjectWithDirPath(dirPath);
}


void FileSystems::openProjectWithDirPath(const QString& pPath) {
    if (ModeManager::getProject()) {
        LOG_ERROR("Вы в проекте:" << projectPath);
        mainWindow->showError("Выйдите из проекта!");
        return;
    }

    const QDir projectDir(pPath);
    if (!projectDir.exists()) {
        LOG_ERROR("Директория не существует:" << pPath);
        mainWindow->showError("Директория проекта не существует");
        return;
    }

    projectPath = projectDir.absolutePath();
    LOG_INFO("Рабочая директория:" << projectPath);

    emit OpenProject(projectPath);

    ModeManager::setSave(true);
    ModeManager::setProject(true);
    mainWindow->ui->inProject();

    openOurpFiles();
}


void FileSystems::openOurpFiles() {
    if (projectPath.isEmpty()) {
        return;
    }

    QDirIterator it(
    projectPath,
    { "*.ourp" },
    QDir::Files,
    QDirIterator::Subdirectories
);

    while (it.hasNext()) {
        it.next();

        const QString name = it.fileName();
        const QString nameOnly = QFileInfo(name).completeBaseName();

        createTabButtons(name);
        emit OpenFile(nameOnly);
    }
}


void FileSystems::slotCreateNewProject() {
    if (ModeManager::getProject()) {
        LOG_ERROR("Вы в проекте:" << projectPath);
        mainWindow->showError("Выйдите из проекта!");
        return;
    }

    const QString dirPath = selectProjectDirectory();
    if (dirPath.isEmpty()) {
        return;
    }

    emit CreateNewProject(dirPath);

    projectPath = dirPath;
    LOG_INFO("Рабочая директория:" << projectPath);

    mainWindow->ui->inProject();

    ModeManager::setProject(true);
    ModeManager::setSave(false);

    const QDir projectDir(dirPath);
    const QString ourpFileName = projectDir.dirName();

    slotCreateNewFile(ourpFileName);
}


void FileSystems::slotCreateNewFile(const QString& ourpFileName) { // name.ourp
    if (projectPath.isEmpty()) {
        return;
    }

    createTabButtons(ourpFileName);
}


void FileSystems::createTabButtons(const QString& ourpFileName) {

    if (ourpFileName.isEmpty()) {
        LOG_ERROR("Неверное имя проекта:" << ourpFileName);
        mainWindow->showError("Имя проекта не должно быть пустым");
        return;
    }

    if (ourpFileName.contains('.')) {
        LOG_ERROR("Неверное имя проекта:" << ourpFileName);
        mainWindow->showError("Имя проекта не должно содержать '.'");
        return;
    }

    const QString ourpName = ourpFileName + ".ourp";

    if (mainWindow->ui->isTabNameExists(ourpName)) {
        LOG_INFO("Такой файл существует :" << ourpName);
        mainWindow->showError("Файл с таким именем существует.");
        return;
    }

    const Ui_MainWindow::TabWidget* tabWidget = nullptr;

    tabWidget = mainWindow->ui->createTabProject(ourpName);

    connect(tabWidget->nameButton, &QPushButton::clicked, [this, ourpName]() {
        LOG_INFO("Переключение на вкладку:" << ourpName);
        emit ChangeTabs(ourpName); // name.ourp
    });

    connect(tabWidget->closeButton, &QToolButton::clicked, [this, tabWidget]() {
        LOG_INFO("Закрытие вкладки: " << tabWidget->name);
        emit CloseTab(tabWidget);
    });

    LOG_INFO("Создание нового файла:" << ourpFileName);

    emit CreateNewTab(ourpName);
}


void FileSystems::slotSaveProject() {
    if (projectPath.isEmpty()) {
        LOG_WARN("Нет проекта для сохранения");
        return;
    }

    if (ModeManager::getSave()) {
        LOG_INFO("Сохранение не требуется");
        return;
    }

    saveProjectToXML();

    emit SaveProject();
}


QString FileSystems::getProjectPath() const {
    return projectPath;
}


void FileSystems::loadProjectsToUI() {
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QXmlStreamReader xml(&file);
    QVector<QPair<QString, QString>> validProjects;
    bool removed = false;

    while (!xml.atEnd()) {
        xml.readNext();

        if (!xml.isStartElement() || xml.name() != "project") {
            continue;
        }

        const QString name = xml.attributes().value("name").toString();
        const QString path = xml.attributes().value("path").toString();

        if (QDir dir(path); !dir.exists()) {
            removed = true;
            continue;
        }

        validProjects.push_back({name, path});

        const auto button =
            mainWindow->ui->addProjectInListStartWindow(name, path);

        connect(button, &QPushButton::clicked, mainWindow,
                [this, path]() {
                    openProjectWithDirPath(path);
                });
    }
    file.close();

    if (removed) {
        QFile out(configPath);
        if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QXmlStreamWriter writer(&out);
            writer.setAutoFormatting(true);
            writer.writeStartDocument();
            writer.writeStartElement("projects");

            for (const auto& p : validProjects) {
                writer.writeStartElement("project");
                writer.writeAttribute("name", p.first);
                writer.writeAttribute("path", p.second);
                writer.writeEndElement();
            }

            writer.writeEndElement();
            writer.writeEndDocument();
        }
    }

    LOG_INFO("Сканирование проектов завершено");
}


void FileSystems::saveProjectToXML() const {
    const QDir dir(projectPath);
    if (!dir.exists()) {
        return;
    }

    const QString projectName =
        QFileInfo(dir.absolutePath()).fileName();

    QFile file(configPath);
    QVector<QPair<QString, QString>> existing;

    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader xml(&file);

        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "project") {
                const QString path =
                    xml.attributes().value("path").toString();
                const QString name =
                    xml.attributes().value("name").toString();

                if (path == projectPath) {
                    return;
                }

                existing.append({ name, path });
            }
        }
        file.close();
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("projects");

    for (const auto& p : existing) {
        xml.writeStartElement("project");
        xml.writeAttribute("name", p.first);
        xml.writeAttribute("path", p.second);
        xml.writeEndElement();
    }

    xml.writeStartElement("project");
    xml.writeAttribute("name", projectName);
    xml.writeAttribute("path", projectPath);
    xml.writeEndElement();

    xml.writeEndElement();
    xml.writeEndDocument();
}


bool FileSystems::projectExistsInConfig(const QString& absoluteProjectPath) const
{
    if (absoluteProjectPath.isEmpty()) {
        return false;
    }

    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QXmlStreamReader xml(&file);

    while (!xml.atEnd()) {
        xml.readNext();

        if (!xml.isStartElement() || xml.name() != "project")
            continue;

        const QString path =
            xml.attributes().value("path").toString();

        if (QDir::cleanPath(path) ==
            QDir::cleanPath(absoluteProjectPath)) {
            return true;
            }
    }

    return false;
}