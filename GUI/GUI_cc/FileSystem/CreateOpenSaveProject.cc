#include "CreateOpenSaveProject.h"

#include "Modes.h"
#include "InputWindow.h"
#include "Mainwindow.h"


CreateOpenSaveProject::CreateOpenSaveProject(MainWindow* mw)
    : QObject(mw),
      mainWindow(mw)
{
}


QString CreateOpenSaveProject::openDirectoryForOpenedProject() const {
    const QString dirPath = QFileDialog::getExistingDirectory(
        mainWindow,
        tr("Выберите директорию проекта"),
        defaultProjectsPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (dirPath.isEmpty()) {
        qDebug() << "Выбор директории отменён пользователем";
        return "";
    }

    qDebug() << "Выбрана директория проекта:" << dirPath;
    return dirPath;
}


void CreateOpenSaveProject::initListProjectStartWindow() {
    QStringList linesToKeep;
    QFile file(listProjectsPath);

    if (!file.exists()) {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Не удалось создать файл:" << listProjectsPath;
            return;
        }
        file.close();
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) {
                continue;
            }

            if (const qint32 sepIndex = line.indexOf(" - "); sepIndex != -1) {
                QString name = line.left(sepIndex).trimmed();
                QString path = line.mid(sepIndex + 3).trimmed();

                if (QString projectDir = QFileInfo(path).absolutePath(); !name.isEmpty() && !path.isEmpty() && QDir(projectDir).exists()) {

                    const auto button = mainWindow->ui->addProjectInListStartWindow(name,path);

                    connect(button, &QPushButton::clicked, [this, button]() {
                        openProject(button->text());
                    });

                    linesToKeep.append(line);
                } else {
                    qDebug() << "Error:" << name << path;
                }
            }
        }
        file.close();
    } else {
        qDebug() << "Не удалось открыть файл:" << listProjectsPath;
    }

    if (!linesToKeep.isEmpty()) {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            for (const QString &line : linesToKeep) {
                out << line << "\n";
            }
            file.close();
        } else {
            qDebug() << "Не удалось перезаписать файл:" << listProjectsPath;
        }
    }

}


QString CreateOpenSaveProject::getProjectPath(){
    return projectPath;
}


void CreateOpenSaveProject::saveSettings(const QString& path) const
{
    const QString setPath = path + ".set";

    QFile file(setPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Не удалось сохранить настройки:" << setPath;
        return;
    }

    QTextStream out(&file);
    out << "grid: " << (mainWindow->ui->componentGrid->isChecked() ? "true" : "false") << "\n";
    out << "axis: " << (mainWindow->ui->componentAxis->isChecked() ? "true" : "false") << "\n";

    file.close();

    qDebug() << "Настройки сохранены в:" << setPath;
}


void CreateOpenSaveProject::saveProject(){
    emit SaveProject(projectPath);

    if (ModeManager::getSave()) {
        QFile listProjects(listProjectsPath);

        if (listProjects.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&listProjects);

            const QString name = QFileInfo(projectPath).fileName();

            out << name << " - " << projectPath << "\n";
            listProjects.close();
            qDebug() << "Проект записан в файл:" << listProjectsPath;
        } else {
            qDebug() << "Не удалось открыть файл для записи:" << listProjectsPath;
        }
    }
}


void CreateOpenSaveProject::openProject(const QString& projectPath_) {

    if (const QDir dir(projectPath); !dir.exists()) {
        mainWindow->showError("Директория не существует:" + projectPath);
        qDebug() << "Директория не существует:" << projectPath;
        return;
    }

    projectPath = projectPath_;

    QDirIterator it(projectPath, QDir::Files, QDirIterator::Subdirectories);

    bool ok = false;

    while (it.hasNext()) {
        QString fileP = it.next();

        if(QString tabName = QFileInfo(fileP).fileName(); tabName.contains(".ourp")) {

            ok = true;

            const auto tabButton = mainWindow->ui->createTabProject(tabName);

            QString ProjectPath_ = fileP;
            ProjectPath_.chop(5);

            QString lastProjectPath_ = projectPath;
            lastProjectPath_.chop(5);

            connect(tabButton, &QPushButton::clicked, [this, tabButton,ProjectPath_,lastProjectPath_]() {
                saveSettings(lastProjectPath_);
                loadSettings(ProjectPath_);
                emit ChangeTabs(tabButton->objectName());
            });

            const QString filePath = projectPath + tabName;

            loadSettings(filePath);

            emit OpenProject(tabButton->objectName());
        }

    }

    if (ok) {
        mainWindow->ui->inProject();
        ModeManager::setSave(false);
    }else {
        mainWindow->showWarning("Bad directory!");
    }
}


void CreateOpenSaveProject::loadSettings(const QString& path) const
{
    const QString setPath = path + ".set";

    QFile file(setPath);
    if (!file.exists()) {
        qDebug() << "Файл настроек не найден:" << setPath;
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл настроек:" << setPath;
        return;
    }

    bool gridChecked = false;
    bool axisChecked = false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        if (const QString line = in.readLine().trimmed(); line.startsWith("grid:")) {
            gridChecked = line.contains("true");
        } else if (line.startsWith("axis:")) {
            axisChecked = line.contains("true");
        }
    }

    file.close();

    mainWindow->ui->componentGrid->setChecked(gridChecked);
    mainWindow->ui->componentAxis->setChecked(axisChecked);

    ModeManager::setCell(gridChecked);
    ModeManager::setAxis(axisChecked);

    qDebug() << "Настройки загружены из:" << setPath;
}


void CreateOpenSaveProject::createFile(const QString& fileName) const {
    const QDir dir(projectPath);

    if (!dir.exists()) {
        qDebug() << "Каталог проекта не существует:" << projectPath;
        return;
    }

    QString finalName = fileName;
    if (!finalName.endsWith(".ourp")) {
        finalName += ".ourp";
    }

    const QString filePath = dir.filePath(finalName);

    if (QFile::exists(filePath)) {
        qDebug() << "Файл уже существует:" << filePath;
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Не удалось создать файл:" << filePath;
        return;
    }

    file.close();

    qDebug() << "Файл проекта успешно создан:" << filePath;
}


void CreateOpenSaveProject::deleteAllProject() const {
    QDir dir(projectPath);

    if (!dir.exists()) {
        qDebug() << "Каталог проекта не существует:" << projectPath;
        return;
    }

    if (!dir.removeRecursively()) {
        qDebug() << "Не удалось удалить каталог проекта:" << projectPath;
        return;
    }

    qDebug() << "Каталог проекта и все содержимое успешно удалены:" << projectPath;
}


void CreateOpenSaveProject::slotOpenProject() {
    const QString dirPath = openDirectoryForOpenedProject();

    openProject(dirPath);
}


void CreateOpenSaveProject::createProject() {
        const QString& projectPath_ = openDirectoryForOpenedProject();

        if (projectPath_.contains('.') ) {
            mainWindow->showError("Ошибка: имя проекта не должно содержать '.'");
            qDebug() << "Ошибка: имя проекта не должно содержать '.'";
            return;
        }

        if (projectPath_.isEmpty()) {
            mainWindow->showError("Отсутствие пути.");
            qDebug() << "Отсутствие пути.";
            return;
        }

        if (projectPath_.endsWith('/') || projectPath_.endsWith('\\')) {
            mainWindow->showError( "Ошибка: имя проекта не должно заканчиваться на '/' или '\\'");
            qDebug() << "Ошибка: имя проекта не должно заканчиваться на '/' или '\\'";
            return;
        }


        const QDir dir(projectPath_);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                mainWindow->showError("Не удалось создать директорию: " + projectPath_);
                qDebug() << "Не удалось создать директорию:" << projectPath_;
                return;
            }

            qDebug() << "Директория создана:" << projectPath_;

        }

        projectPath = projectPath_;

        const QString name = QFileInfo(projectPath).fileName();

        const QString fileProject = dir.filePath(name  + ".ourp");

        if (QFile file(fileProject); file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.close();
            qDebug() << "Файл создан:" << fileProject;

            const QFile _file(fileProject);
            const QFileInfo fileInfo(_file);
            const QString justName = fileInfo.fileName();

            const auto tabButton = mainWindow->ui->createTabProject(justName);

            connect(tabButton, &QPushButton::clicked, [this, tabButton]() {
                emit ChangeTabs(tabButton->objectName());
            });

            createFile(tabButton->objectName());

            mainWindow->ui->inProject();
        } else {
            mainWindow->showError("Ошибка создания файла: " + file.errorString());
            qDebug() << "Ошибка создания файла:" << file.errorString();
        }

}