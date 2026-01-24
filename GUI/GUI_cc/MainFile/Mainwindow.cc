#include "Mainwindow.h"

#include "CreateOpenSaveProject.h"
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
   // saveLoadProject->initListProjectStartWindow();
}


LeftMenuBar* MainWindow::getLeftMenuBar() const {
    return leftMenuBar;
}


QTPainter* MainWindow::getQTPainter() const {
    return ui->workWindow;
}


QString MainWindow::getProjectPath() const {
    return saveLoadProject->getProjectPath();
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
    connect(ui->createProjectButton, &QPushButton::clicked, saveLoadProject, &CreateOpenSaveProject::createProject);
    connect(ui->actionCreate_project_to, &QPushButton::clicked, saveLoadProject, &CreateOpenSaveProject::createProject);
    connect(ui->actionOpen_project, &QPushButton::clicked, saveLoadProject, &CreateOpenSaveProject::slotOpenProject);
    connect(ui->loadProjectButton, &QPushButton::clicked, saveLoadProject, &CreateOpenSaveProject::slotOpenProject);

    connect(saveLoadProject, &CreateOpenSaveProject::ChangeTabs, this, &MainWindow::slotChangeTabs);
    connect(saveLoadProject, &CreateOpenSaveProject::OpenProject, this, &MainWindow::slotOpenProject);
    connect(saveLoadProject, &CreateOpenSaveProject::SaveProject, this, &MainWindow::slotSaveProject);

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


void MainWindow::updateShapeCursor(const QPoint& pos) {
    const qint32 x = pos.x();
    const qint32 y = pos.y();
    const qint32 w = width();
    const qint32 h = height();

    ResizeRegion region = None;

    if (x < edgeMargin && y < edgeMargin) { region = TopLeft; }
    else if (x > w - edgeMargin && y < edgeMargin) { region = TopRight; }
    else if (x < edgeMargin && y > h - edgeMargin) { region = BottomLeft; }
    else if (x > w - edgeMargin && y > h - edgeMargin) { region = BottomRight; }
    else if (x < edgeMargin) { region = Left; }
    else if (x > w - edgeMargin) { region = Right; }
    else if (y < edgeMargin) { region = Top; }
    else if (y > h - edgeMargin) { region = Bottom; }

    currentRegion = region;

    switch (region) {
        case TopLeft:
        case BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case TopRight:
        case BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case Left:
        case Right:
            setCursor(Qt::SizeHorCursor);
            break;
        case Top:
        case Bottom:
            setCursor(Qt::SizeVerCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
    }
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


void MainWindow::closeProgram() {
    if (!ModeManager::getSave()) {
        SaveDialog dialog(this);
        dialog.setModal(true);

        if (const qint32 result = dialog.exec(); result == QMessageBox::Yes) {

            saveLoadProject->saveProject();

            if (ModeManager::getSave()) {
                close();
            }

        }else {
            saveLoadProject->deleteAllProject();
            close();
        }
    }

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



void MainWindow::closeEvent(QCloseEvent* event) {
    closeProgram();
    event->accept();
}


void MainWindow::mousePressEvent(QMouseEvent* event) {
    dragStartPos = event->globalPosition().toPoint();
    originalGeometry = geometry();

    if (event->button() == Qt::LeftButton) {
        if (currentRegion != None) {
            resizing = true;
        } else {
            moving = true;
        }
    }

    QMainWindow::mousePressEvent(event);
}


void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    const QPoint globalPos = event->globalPosition().toPoint();

    if (resizing) {
        QRect geom = originalGeometry;
        const QPoint delta = globalPos - dragStartPos;

        switch (currentRegion) {
            case Top:
                geom.setTop(geom.top() + delta.y());
                break;
            case Bottom:
                geom.setBottom(geom.bottom() + delta.y());
                break;
            case Left:
                geom.setLeft(geom.left() + delta.x());
                break;
            case Right:
                geom.setRight(geom.right() + delta.x());
                break;
            case TopLeft:
                geom.setTop(geom.top() + delta.y());
                geom.setLeft(geom.left() + delta.x());
                break;
            case TopRight:
                geom.setTop(geom.top() + delta.y());
                geom.setRight(geom.right() + delta.x());
                break;
            case BottomLeft:
                geom.setBottom(geom.bottom() + delta.y());
                geom.setLeft(geom.left() + delta.x());
                break;
            case BottomRight:
                geom.setBottom(geom.bottom() + delta.y());
                geom.setRight(geom.right() + delta.x());
                break;
            default:
                break;
        }

        setGeometry(geom);
    } else if (moving) {
        move(pos() + globalPos - dragStartPos);
        dragStartPos = globalPos;
    } else {
        updateShapeCursor(event->pos());
    }

    QMainWindow::mouseMoveEvent(event);
}


void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ArrowCursor);
        resizing = false;
        moving = false;
        event->accept();
    } else {
        QMainWindow::mouseReleaseEvent(event);
    }
}


void MainWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        moving = true;
        dragStartPos = event->globalPosition().toPoint();
    }

    QMainWindow::mouseDoubleClickEvent(event);
}


void MainWindow::wheelEvent(QWheelEvent* event) {
    if (ui->workWindow && ui->workWindow->underMouse()) {
        if (event->angleDelta().y() > 0) {
            Scaling::setZoomPlus();
        } else {
            Scaling::setZoomMinus();
        }
        ui->workWindow->update();
        event->accept();
    } else {
        QMainWindow::wheelEvent(event);
    }
}


bool MainWindow::event(QEvent* event) {

    if (event->type() == QEvent::Gesture) {
        const QGestureEvent* gestureEvent = static_cast<QGestureEvent*>(event);
        if (QGesture* pinch = gestureEvent->gesture(Qt::PinchGesture)) {
            if (const auto pinchGesture = static_cast<QPinchGesture*>(pinch); pinchGesture->changeFlags() &
                QPinchGesture::ScaleFactorChanged) {

                if (pinchGesture->scaleFactor() > 1.0) {
                    Scaling::setZoomPlus();
                } else {
                    Scaling::setZoomMinus();
                }
            }
            return true;
        }
    }

    return QMainWindow::event(event);
}


bool MainWindow::eventFilter(QObject* obj, QEvent* event) {

    if (event->type() == QEvent::KeyPress) {

        if (const auto keyEvent = static_cast<QKeyEvent*>(event); keyEvent->key() == Qt::Key_Tab) {

            event->accept();
        } else if (!ui->console->isActiveWindow() &&
                   keyEvent->key() != Qt::ControlModifier &&
                   (keyEvent->key() == Qt::Key_Up ||
                    keyEvent->key() == Qt::Key_Down ||
                    keyEvent->key() == Qt::Key_Right ||
                    keyEvent->key() == Qt::Key_Left)) {
        }

        return true;
    }

    return QObject::eventFilter(obj, event);
}


void MainWindow::keyPressEvent(QKeyEvent* event) {

    // Processing ctrl + arrow keys -> moves the window to different positions
    if (event->modifiers() & Qt::ControlModifier) {
        const QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
        if (event->key() == Qt::Key_Left) {
            const bool isRightDownHalf = this->geometry() == QRect(screenGeometry.left() + screenGeometry.width() / 2,
                                                             screenGeometry.height() / 2,
                                                             screenGeometry.width() / 2,
                                                             screenGeometry.height() / 2);
            const bool isRightTop = this->geometry() ==
                              QRect(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.top(),
                                    screenGeometry.width() / 2, screenGeometry.height() / 2);
            if (isRightTop) {
                this->setGeometry(screenGeometry.left(), screenGeometry.top(),
                                  screenGeometry.width() / 2, screenGeometry.height() / 2);
            } else if (isRightDownHalf) {
                this->setGeometry(screenGeometry.left(), screenGeometry.height() / 2,
                                  screenGeometry.width() / 2, screenGeometry.height() / 2);
            } else {
                this->setGeometry(screenGeometry.left(), screenGeometry.top(),
                                  screenGeometry.width() / 2, screenGeometry.height());
            }
        } else if (event->key() == Qt::Key_Right) {
            const bool isLeftDownHalf = this->geometry() ==
                                  QRect(screenGeometry.left(), screenGeometry.height() / 2,
                                        screenGeometry.width() / 2,
                                        screenGeometry.height() / 2);
            const bool isLeftUpHalf = this->geometry() ==
                                QRect(screenGeometry.left(), screenGeometry.height() / 2,
                                      screenGeometry.width() / 2,
                                      screenGeometry.height() / 2);
            if (isLeftUpHalf) {
                this->setGeometry(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.top(),
                                  screenGeometry.width() / 2, screenGeometry.height() / 2);
            } else if (isLeftDownHalf) {
                this->setGeometry(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.height() / 2,
                                  screenGeometry.width() / 2, screenGeometry.height() / 2);
            } else {
                this->setGeometry(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.top(),
                                  screenGeometry.width() / 2, screenGeometry.height());
            }
        } else if (event->key() == Qt::Key_Up) {
            const bool isLeftDownHalf = this->geometry() ==
                                  QRect(screenGeometry.left(), screenGeometry.height() / 2,
                                        screenGeometry.width() / 2,
                                        screenGeometry.height() / 2);
            const bool isRightDownHalf = this->geometry() == QRect(screenGeometry.left() + screenGeometry.width() / 2,
                                                             screenGeometry.height() / 2,
                                                             screenGeometry.width() / 2,
                                                             screenGeometry.height() / 2);
            const bool isLeft = this->geometry() ==
                          QRect(screenGeometry.left(), screenGeometry.top(), screenGeometry.width() / 2,
                                screenGeometry.height());
            const bool isRight = this->geometry() ==
                           QRect(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.top(),
                                 screenGeometry.width() / 2, screenGeometry.height());
            if (isLeft || isLeftDownHalf) {
                this->setGeometry(screenGeometry.left(), screenGeometry.top(), screenGeometry.width() / 2,
                                  screenGeometry.height() / 2);
            } else if (isRight || isRightDownHalf) {
                this->setGeometry(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.top(),
                                  screenGeometry.width() / 2, screenGeometry.height() / 2);
            } else {
                this->showMaximized();
            }
        } else if (event->key() == Qt::Key_Down) {
            const bool isOnRight =
                    this->geometry() ==
                    QRect(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.top(),
                          screenGeometry.width() / 2, screenGeometry.height());
            const bool isOnLeft = this->geometry() ==
                            QRect(screenGeometry.left(), screenGeometry.top(), screenGeometry.width() / 2,
                                  screenGeometry.height());
            const bool isRightTop = this->geometry() ==
                              QRect(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.top(),
                                    screenGeometry.width() / 2, screenGeometry.height() / 2);
            const bool isLeftTop = this->geometry() ==
                             QRect(screenGeometry.left(), screenGeometry.top(), screenGeometry.width() / 2,
                                   screenGeometry.height() / 2);
            if (this->isMaximized()) {
                this->showNormal();
            } else if (isOnRight || isRightTop) {
                this->setGeometry(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.height() / 2,
                                  screenGeometry.width() / 2, screenGeometry.height() / 2);
            } else if (isOnLeft || isLeftTop) {
                this->setGeometry(screenGeometry.left(), screenGeometry.height() / 2, screenGeometry.width() / 2,
                                  screenGeometry.height() / 2);
            } else {
                this->showMinimized();
            }

        }
    }



    QWidget::keyPressEvent(event);
}



/// ***** SLOTS:



void MainWindow::slotSaveProject(const QString& fileName) {
    emit OpenProject(fileName);
}


void MainWindow::slotOpenProject(const QString& fileName) {
    emit OpenProject(fileName);
}


void MainWindow::slotChangeTabs(const QString& tabName) {
    emit ChangeTabs(tabName);
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

            connect(wind,&InputWindow::textEnter, [this](const QString& text) {
                const QString projectName= text + ".ourp";
                QPushButton* tabButton = ui->createTabProject(projectName);

                connect(tabButton, &QPushButton::clicked, [this, tabButton]() {
                    emit ChangeTabs(tabButton->objectName());
                });

                saveLoadProject->createFile(tabButton->objectName());
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