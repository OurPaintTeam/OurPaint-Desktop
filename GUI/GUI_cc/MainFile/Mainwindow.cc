#include "Mainwindow.h"

#include "FileSystems.h"
#include "LeftMenuBar.h"
#include "ui_mainwindow.h"
#include "Help.h"
#include "CustomWindowError.h"
#include "CustomWindowSuccessful.h"
#include "CustomWindowWarning.h"
#include "CustomIpListWindow.h"
#include "SaveDialog.h"
#include "InputWindow.h"
#include "ui_mainwindow.h"
#include "QTPainter.h"
#include "Modes.h"
#include "LeftMenuBar.h"
#include "ParameterDelegate.h"


MainWindow::MainWindow(QWidget* parent)
                        : QMainWindow(parent){

    ui->setupUi(this);
    setMouseTracking(true);

    this->setFocusPolicy(Qt::StrongFocus);
    this->installEventFilter(this);
    this->setFocus();

    initConnections(); // Initialization of signals
    setupLeftMenu();
    fileSystems->loadProjectsToUI();
}


LeftMenuBar* MainWindow::getLeftMenuBar() const {
    return leftMenuBar;
}


QTPainter* MainWindow::getQTPainter() const {
    return ui->workWindow;
}


QString MainWindow::getProjectPath() const {
    return fileSystems->getProjectPath();
}


void  MainWindow::inStartWindow() const {
    ui->startWindow();
}


void MainWindow::inProjectWindow() const {
    ui->inProject();
}



/// **** PRIVATE:



void MainWindow::initConnections() {

    // Mode buttons
    connect(ui->figurePoint, &QPushButton::clicked, this, &MainWindow::Point);
    connect(ui->figureCircle, &QPushButton::clicked, this, &MainWindow::Circle);
    connect(ui->figureSection, &QPushButton::clicked, this, &MainWindow::Section);
    connect(ui->figureArc, &QPushButton::clicked, this, &MainWindow::Arc);
    connect(ui->toolMoving, &QPushButton::clicked, this, &MainWindow::ToolMoving);
    connect(ui->toolSelected, &QPushButton::clicked, this, &MainWindow::ToolSelected);
    connect(ui->toolShowSize, &QPushButton::clicked, this, &MainWindow::ToolShowSize);

    // Save/import buttons
    connect(ui->openFolderForOpenProject, &QPushButton::clicked, fileSystems, &FileSystems::slotOpenProject);
    connect(ui->openFolderForCreateProject, &QPushButton::clicked, fileSystems, &FileSystems::slotCreateNewProject);
    connect(ui->actionCreate_project_to, &QPushButton::clicked, fileSystems, &FileSystems::slotCreateNewProject);
    connect(ui->actionOpen_project, &QPushButton::clicked, fileSystems, &FileSystems::slotOpenProject);

    connect(fileSystems, &FileSystems::ChangeTabs, this, &MainWindow::slotChangeTabs);
    connect(fileSystems, &FileSystems::OpenProject, this, &MainWindow::slotOpenProject);
    connect(fileSystems, &FileSystems::CreateNewProject, this, &MainWindow::slotCreateNewProject);
    connect(fileSystems, &FileSystems::SaveProject, this, &MainWindow::slotSaveProject);
    connect(fileSystems, &FileSystems::CreateNewTab, this, &MainWindow::slotCreateNewTab);
    connect(fileSystems, &FileSystems::RenameTab, this, &MainWindow::slotRenameTab);
    connect(fileSystems, &FileSystems::DeleteTab, this, &MainWindow::slotDeleteTab);

    connect(ui->actionJPG, &QToolButton::clicked, this, &MainWindow::onExportJPG);
    connect(ui->actionJPEG, &QToolButton::clicked, this, &MainWindow::onExportJPEG);
    connect(ui->actionPNG, &QToolButton::clicked, this, &MainWindow::onExportPNG);
    connect(ui->actionBMP, &QToolButton::clicked, this, &MainWindow::onExportBMP);
    connect(ui->actionTIFF, &QToolButton::clicked, this, &MainWindow::onExportTIFF);
    connect(ui->actionPDF, &QToolButton::clicked, this, &MainWindow::onExportPDF);
    connect(ui->actionSVG, &QToolButton::clicked, this, &MainWindow::onExportSVG);
    connect(ui->actionOURP, &QToolButton::clicked, this, &MainWindow::onExportOURP);
    connect(ui->actionScript, &QToolButton::clicked, this, &MainWindow::buttonScript);

    // Server buttons
  //  connect(ui->actionOpen_server, &QToolButton::clicked, this, &MainWindow::openServer);
   // connect(ui->actionJoin_server, &QToolButton::clicked, this, &MainWindow::joinServer);
   // connect(ui->actionJoin_local_server, &QToolButton::clicked, this, &MainWindow::joinLocalServer);
   // connect(ui->actionExit_from_session, &QToolButton::clicked, this, &MainWindow::exitSession);

    // Processing input to the chat console
    connect(ui->messageConsole, &QLineEdit::returnPressed, this, &MainWindow::Message);
    connect(ui->enterMes, &QPushButton::clicked, this, &MainWindow::Message);


    // Grid Settings
    connect(ui->componentGrid, &QCheckBox::toggled,this,&MainWindow::updateGrid);
    connect(ui->componentAxis, &QCheckBox::toggled, this,&MainWindow::updateAxis);

    // Console input processing
    connect(ui->console, &QLineEdit::returnPressed, this, &MainWindow::commandsInConsole);
    connect(ui->enterConsole, &QPushButton::clicked, this, &MainWindow::commandsInConsole);
}


void MainWindow::setupConsoleCommands(const QStringList& commandList ) const {
    ui->console->setCommands(commandList);
}


void MainWindow::setupLeftMenu() {
    ui->leftMenuView->setModel(leftMenuBar->getTreeModel());
    ui->leftMenuView->setHeaderHidden(true);
    ui->leftMenuView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->leftMenuView->setItemDelegate(new ParameterDelegate(leftMenuBar));

    connect(static_cast<ParameterDelegate*>(ui->leftMenuView->itemDelegate()),
            &ParameterDelegate::doubleClickOnID,
            leftMenuBar,
            &LeftMenuBar::doubleClickID);


    connect(ui->leftMenuView, &QTreeView::customContextMenuRequested,
            this, &MainWindow::onLeftMenuRightClick);
}



/// **** PUBLIC:



void MainWindow::selectLeftMenuElem(const QModelIndex& index) const {
    ui->leftMenuView->setCurrentIndex(index);    // selection
    ui->leftMenuView->scrollTo(index);           // scrolling
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        ui->leftMenuView->expand(parent);
        parent = parent.parent();
    }
    ui->leftMenuView->expand(index);
    ui->leftMenuView->update();
    leftMenuBar->update();
}


void MainWindow::updateExitServerStyle(const bool connect) const {
    ui->updateExitServerStyle(connect);
}


void MainWindow::setMessage(const QString& name, const QString& message) const {
    const QString messageText = name + ": " + message;

    const auto messageLabel = new QLabel(messageText);
    messageLabel->setStyleSheet("color: #D8D8F6;");
    messageLabel->setWordWrap(true);
    messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    ui->messageContentLayout->addWidget(messageLabel);
}



/***    Custom windows      ***/



void MainWindow::showError(const QString& text) const {
   ui->showError(text);
}


void MainWindow::showWarning(const QString& text) const {
    ui->showWarning(text);
}


void MainWindow::showSuccess(const QString& text) const {
    ui->showSuccess(text);
}



/***     Save/import settings       ***/



QString MainWindow::getUserName() {
    return "DEFAULT";
}





///   BUTTONS:



QPushButton* MainWindow::getFirstBut() const {
    return ui->oneReq;
}


QPushButton* MainWindow::getSecondBut() const {
    return ui->twoReq;
}


QPushButton* MainWindow::getThirdBut() const {
    return ui->threeReq;
}


QPushButton* MainWindow::getFourthBut() const {
    return ui->fourReq;
}


QPushButton* MainWindow::getFifthBut() const {
    return ui->fiveReq;
}


QPushButton* MainWindow::getSixthBut() const {
    return ui->sixReq;
}


QPushButton* MainWindow::getSeventhBut() const {
    return ui->sevenReq;
}


QPushButton* MainWindow::getEighthBut() const {
    return ui->eightReq;
}


QPushButton* MainWindow::getNinthBut() const {
    return ui->nineReq;
}


QPushButton* MainWindow::getTenthBut() const {
   return ui->tenReq;
}



/// ***** PROTECTED:

bool MainWindow::closeProgram() {
    if (!ModeManager::getSave()) {
        SaveDialog dialog(this);
        const auto result = dialog.exec();

        if (result == QMessageBox::Yes) {
            fileSystems->slotSaveProject();
            return ModeManager::getSave();
        }

        if (result == QMessageBox::No)
            return true;

        return false; // Cancel
    }
    return true;
}


void MainWindow::closeEvent(QCloseEvent* event) {
    windowController->handleCloseEvent(event);
}


void MainWindow::mousePressEvent(QMouseEvent* event) {
    windowController->mousePress(event);
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    windowController->mouseMove(event);
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    windowController->mouseRelease(event);
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    windowController->mouseDoubleClick(event);
    QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::wheelEvent(QWheelEvent* event) {
    windowController->wheel(event);
}

bool MainWindow::event(QEvent* event) {
    if (windowController->handleEvent(event)) {
        return true;
    }
    return QMainWindow::event(event);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (windowController->handleEventFilter(obj, event)) {
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    windowController->keyPress(event);
    QMainWindow::keyPressEvent(event);
}


/// ***** SLOTS:

void MainWindow::slotOpenFile(const QString& fileName) {
    emit OpenFile(fileName);
}

void MainWindow::slotDeleteTab(const QString& tabName) {
    emit DeleteTab(tabName);
}

void MainWindow::slotRenameTab(const QString& oldName,const QString& newName) {
    if (ui->renameTab(oldName,newName)) {
        emit RenameTab(oldName,newName);
    }else {
        showError("Такой файл уже существует!");
    }
}

void MainWindow::slotSaveProject() {
    emit SaveProject();
}

void MainWindow::slotCreateNewProject(const QString& workDir) {
    emit OpenProject(workDir);
}

void MainWindow::slotOpenProject(const QString& workDir) {
    emit OpenProject(workDir);
}


void MainWindow::slotChangeTabs(const QString& tabName) {
    emit ChangeTabs(tabName);
}


void MainWindow::slotCreateNewTab(const QString& tabName) {
    emit CreateNewTab(tabName);
}


void MainWindow::buttonScript() {
    // Opening the project file selection dialog
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    QDir::homePath(),
                                                    tr("Project Files (*.txt);;All Files (*)"));

    if (!fileName.isEmpty()) {
        emit EmitScript(fileName);
    }
}



void MainWindow::Message() {
    if (const QString input = ui->messageConsole->text(); !input.isEmpty()) {
        ui->messageConsole->clear();
        emit EnterMessage(input);
    }
}


void MainWindow::Point() {
    ModeManager::setActiveMode(WorkModes::Point);
}


void MainWindow::Section() {
    ModeManager::setActiveMode(WorkModes::Section);
}


void MainWindow::Circle() {
    ModeManager::setActiveMode(WorkModes::Circle);
}


void MainWindow::Arc() {
    ModeManager::setActiveMode(WorkModes::Arc);
}


void MainWindow::ToolMoving() {
    ModeManager::setActiveMode(WorkModes::Move);
}


void MainWindow::ToolSelected() {
    ModeManager::setActiveMode(WorkModes::Selected);
}


void MainWindow::ToolShowSize() {
    ModeManager::setActiveMode(WorkModes::ShowSize);
}


void MainWindow::onExportJPG() {
  emit SaveProjectInFormat("DEFAULT",".jpg");
}


void MainWindow::onExportJPEG() {
  emit SaveProjectInFormat("DEFAULT",".jpeg");
}


void MainWindow::onExportPNG() {
  emit SaveProjectInFormat("DEFAULT",".png");
}


void MainWindow::onExportBMP() {
  emit SaveProjectInFormat("DEFAULT",".bmp");
}


void MainWindow::onExportTIFF() {
  emit SaveProjectInFormat("DEFAULT",".tiff");
}


void MainWindow::onExportPDF() {
  emit SaveProjectInFormat("DEFAULT",".pdf");
}


void MainWindow::onExportOURP() {
  emit SaveProjectInFormat("DEFAULT",".ourp");
}


void MainWindow::onExportSVG() {
    emit SaveProjectInFormat("DEFAULT",".svg");
}


void MainWindow::onLeftMenuRightClick(const QPoint &pos) {
    const QModelIndex index = ui->leftMenuView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const auto node = static_cast<TreeNode*>(index.internalPointer());
    if (!node) {
        return;
    }

    if (node == leftMenuBar->getProjectsNode()) {
        QMenu menu;
        const auto createTab = menu.addAction("Create tab");

        if (const auto chosen = menu.exec(ui->leftMenuView->viewport()->mapToGlobal(pos)); chosen == createTab) {
            const auto wind = new InputWindow("Name:",this);
            wind->show();

            connect(wind,&InputWindow::textEnter, [this](const QString& name) {
                if (!ui->isButtonNameExists(name)) {
                    fileSystems->slotCreateNewFile(name);
                }else {
                    showError("Файл с таким именем уже существует!");
                }
            });

        }
    }
}



///           ANOTHER:



void MainWindow::updateGrid(const bool checked) const {
    ModeManager::setCell(checked);
    ui->workWindow->update();
}


void MainWindow::updateAxis(const bool checked) const {
    ModeManager::setAxis(checked);
    ui->workWindow->update();
}


void MainWindow::commandsInConsole(){
    if (const QString input = ui->console->text(); !input.isEmpty()) {
        ui->console->pushBack(input);
        emit EnterCommand(input);
        ui->console->clear();
    }
}