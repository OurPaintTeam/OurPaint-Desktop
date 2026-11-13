#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>

#include "ProjectWindow.h"
#include "QtMainWindowBinder.h"
#include "QtPath.h"

namespace {

QWidget* dialogParent() {
    for (auto* widget : QApplication::topLevelWidgets()) {
        if (qobject_cast<UI::ProjectWindow*>(widget) && widget->isVisible()) {
            return widget;
        }
    }
    return QApplication::activeWindow();
}

QString chooseFolder(const char* title) {
    return QFileDialog::getExistingDirectory(dialogParent(), QCoreApplication::translate("ProjectPersistence", title), {},
                                             QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
}

QString fileErrorText(const app::FileError& error) {
    QString message;
    using app::FileStage;
    switch (error.stage) {
        case FileStage::ValidateProject:
            message = QCoreApplication::translate("ProjectPersistence", "The project data is invalid.");
            break;
        case FileStage::ValidatePath:
            message = QCoreApplication::translate("ProjectPersistence", "This file or folder cannot be used.");
            break;
        case FileStage::GenerateId:
            message = QCoreApplication::translate("ProjectPersistence", "Could not create a project identifier.");
            break;
        case FileStage::Encode:
            message = QCoreApplication::translate("ProjectPersistence", "Could not prepare a document for saving.");
            break;
        case FileStage::CreateDirectory:
            message = QCoreApplication::translate("ProjectPersistence", "Could not create the project folder.");
            break;
        case FileStage::Open:
        case FileStage::Read:
        case FileStage::Enumerate:
            message = QCoreApplication::translate("ProjectPersistence", "Could not read the file or project folder.");
            break;
        case FileStage::Decode:
            message = QCoreApplication::translate("ProjectPersistence", "The file is damaged or uses an unsupported format.");
            break;
        default:
            message = QCoreApplication::translate("ProjectPersistence", "Could not finish saving the file.");
            break;
    }
    message += QStringLiteral("\n") + app::ui::pathText(error.path);
    if (error.codecError) {
        message += QStringLiteral("\n") + QString::fromStdString(error.codecError->message);
    }
    if (error.systemError) {
        message += QStringLiteral("\n") + QString::fromLocal8Bit(error.systemError.message().c_str());
    }
    return message;
}

void present(UI::ProjectManager& manager, const app::SaveReport& report) {
    if (report) {
        manager.addNotification(QCoreApplication::translate("ProjectPersistence", "Project saved to %1.").arg(app::ui::pathText(report.root)));
        return;
    }
    QString text = fileErrorText(*report.error);
    text += QCoreApplication::translate("ProjectPersistence", "\nSaved files: %1. Not attempted: %2.")
                .arg(static_cast<qulonglong>(report.saved.size()))
                .arg(static_cast<qulonglong>(report.unprocessed.size()));
    if (report.failedFileReplaced) {
        text += QCoreApplication::translate("ProjectPersistence", "\nThe file already contains the new data, but saving could not be fully confirmed.");
    }
    manager.addNotification(text);
}

}  // namespace

void QtMainWindowBinder::updateProjectData() {
    const auto& metadata = controller_.projectMetadata();
    window_.setProjectData({app::ui::pathText(controller_.projectRoot()), QString::fromStdString(metadata.id)});
    if (auto* window = dialogParent()) {
        window->setWindowTitle(QString::fromStdString(metadata.name) + QStringLiteral(" - OurPaint"));
    }
}

bool QtMainWindowBinder::saveProject() {
    if (controller_.projectRoot().empty()) {
        return saveProjectAs();
    }
    auto report = controller_.saveProject();
    updateProjectData();
    present(window_, report);
    return static_cast<bool>(report);
}

bool QtMainWindowBinder::saveProjectAs() {
    const auto path = chooseFolder("Save project to a new or empty folder");
    if (path.isEmpty()) {
        return false;
    }
    auto report = controller_.saveProjectAs(app::ui::nativePath(path));
    if (report) {
        updateProjectData();
    }
    present(window_, report);
    return static_cast<bool>(report);
}

bool QtMainWindowBinder::confirmClose() {
    controller_.endInteractions();
    if (!controller_.needsSaveConfirmation()) {
        return true;
    }
    const auto answer = QMessageBox::question(dialogParent(), QCoreApplication::translate("ProjectPersistence", "Unsaved project"),
                                              QCoreApplication::translate("ProjectPersistence", "Save the current project before continuing?"),
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    return answer == QMessageBox::Discard || (answer == QMessageBox::Save && saveProject());
}

bool QtMainWindowBinder::openProjectInCurrentWindow(const QString& requestedPath) {
    const auto selected = requestedPath.isEmpty() ? chooseFolder("Open project folder") : requestedPath;
    if (selected.isEmpty() || !confirmClose()) {
        return false;
    }
    if (auto error = controller_.openProject(app::ui::nativePath(selected))) {
        window_.addNotification(fileErrorText(*error));
        return false;
    }
    updateProjectData();
    window_.addNotification(QCoreApplication::translate("ProjectPersistence", "Project opened."));
    return true;
}

bool QtMainWindowBinder::createProjectInCurrentWindow(const QString& requestedPath) {
    const auto selected = requestedPath.isEmpty() ? chooseFolder("Create project in a new or empty folder") : requestedPath;
    if (selected.isEmpty() || !confirmClose()) {
        return false;
    }
    auto report = controller_.createProject(app::ui::nativePath(selected));
    if (report) {
        updateProjectData();
    }
    present(window_, report);
    return static_cast<bool>(report);
}

void QtMainWindowBinder::openProjectInNewWindow(const QString& requestedPath) {
    const auto path = requestedPath.isEmpty() ? chooseFolder("Open project folder") : requestedPath;
    if (!path.isEmpty() && !QProcess::startDetached(QCoreApplication::applicationFilePath(), {QStringLiteral("--open-project"), path})) {
        window_.addNotification(QCoreApplication::translate("ProjectPersistence", "Could not open another window."));
    }
}

void QtMainWindowBinder::createProjectInNewWindow(const QString& requestedPath) {
    const auto path = requestedPath.isEmpty() ? chooseFolder("Create project in a new or empty folder") : requestedPath;
    if (!path.isEmpty() && !QProcess::startDetached(QCoreApplication::applicationFilePath(), {QStringLiteral("--create-project"), path})) {
        window_.addNotification(QCoreApplication::translate("ProjectPersistence", "Could not open another window."));
    }
}

void QtMainWindowBinder::openFile(const QString& requestedPath) {
    const auto path = requestedPath.isEmpty()
                          ? QFileDialog::getOpenFileName(dialogParent(), QCoreApplication::translate("ProjectPersistence", "Open sketch or project"), {},
                                                         QCoreApplication::translate("ProjectPersistence", "OurPaint files (*.ourp .ourpaint);;All files (*)"))
                          : requestedPath;
    if (path.isEmpty()) {
        return;
    }
    if (QFileInfo(path).isDir() || QFileInfo(path).fileName() == QStringLiteral(".ourpaint")) {
        openProjectInCurrentWindow(path);
        return;
    }
    if (auto error = controller_.openFile(app::ui::nativePath(path))) {
        window_.addNotification(fileErrorText(*error));
    }
}
