#include "Mainwindow.h"


MainWindow::MainWindow(QWidget* parent)
                        : QMainWindow(parent){

    ui->setupUi(this);
    setMouseTracking(true);

    this->setFocusPolicy(Qt::StrongFocus);
    this->installEventFilter(this);
    this->setFocus();

    loadSettings();
    initConnections(); // Initialization of signals
    setupLeftMenu();
    initListProjectStartWindow();
}


MainWindow::~MainWindow() {
    delete ui;
}


LeftMenuBar* MainWindow::getLeftMenuBar() const {
    return leftMenuBar;
}


QTPainter* MainWindow::getQTPainter() const {
    return ui->workWindow;
}



/// **** PRIVATE:



void MainWindow::openProject(const QString& dirPath) {
    QDir dir(dirPath);

    if (!dir.exists()) {
        showError("Директория не существует:" + dirPath);
        qDebug() << "Директория не существует:" << dirPath;
        return;
    }

    QDirIterator it(dirPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString fileP = it.next();
        QString name = QFileInfo(fileP).fileName();
        if(name.contains(".ourp")) {
            QPair<QPushButton*, QTPainter*> result = ui->createTabProject(name);
            QPushButton* tabButton = result.first;
            QTPainter* painter = result.second;

            connect(tabButton, &QPushButton::clicked, [this, painter]() {
                emit changeTabs(painter);
            });
            emit LoadFile(filePath, painter);
        }

    }

    ui->inProject();
}


void MainWindow::initListProjectStartWindow() {
    QStringList linesToKeep;
    QFile file(filePath);

    if (!file.exists()) {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Не удалось создать файл:" << filePath;
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

            qint32 sepIndex = line.indexOf(" - ");
            if (sepIndex != -1) {
                QString name = line.left(sepIndex).trimmed();
                QString path = line.mid(sepIndex + 3).trimmed();

                QString projectDir = QFileInfo(path).absolutePath();
                if (!name.isEmpty() && !path.isEmpty() && QDir(projectDir).exists()) {

                    QPushButton* button = ui->addProjectInListStartWindow(name,path);

                    connect(button, &QPushButton::clicked, [button, this]() {
                        openProject( button->text());
                    });

                    linesToKeep.append(line);
                } else {
                    qDebug() << "Error:" << name << path;
                }
            }
        }
        file.close();
    } else {
        qDebug() << "Не удалось открыть файл:" << filePath;
    }

    if (!linesToKeep.isEmpty()) {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            for (const QString &line : linesToKeep) {
                out << line << "\n";
            }
            file.close();
        } else {
            qDebug() << "Не удалось перезаписать файл:" << filePath;
        }
    }

}


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
    connect(ui->createProjectButton, &QPushButton::clicked, this, &MainWindow::onCreateProject);
    connect(ui->actionCreate_project_to, &QPushButton::clicked, this, &MainWindow::onCreateProject);
    connect(ui->actionOpen_project, &QPushButton::clicked, this, &MainWindow::loadProjectFile);
    connect(ui->loadProjectButton, &QPushButton::clicked, this, &MainWindow::loadProjectFile);
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
    connect(ui->actionOpen_server, &QToolButton::clicked, this, &MainWindow::openServer);
    connect(ui->actionJoin_server, &QToolButton::clicked, this, &MainWindow::joinServer);
    connect(ui->actionJoin_local_server, &QToolButton::clicked, this, &MainWindow::joinLocalServer);
    connect(ui->actionExit_from_session, &QToolButton::clicked, this, &MainWindow::exitSession);

    // Processing input to the chat console
    connect(ui->messageConsole, &QLineEdit::returnPressed, this, &MainWindow::Message);
    connect(ui->enterMes, &QPushButton::clicked, this, &MainWindow::Message);

    // Settings name
    connect(ui->nameUsers, &QLineEdit::returnPressed, this, &MainWindow::setNameUsers);

    // Grid Settings
    connect(ui->componentGrid, &QCheckBox::toggled,this,&MainWindow::updateGrid);
    connect(ui->componentAxis, &QCheckBox::toggled, this,&MainWindow::updateAxis);

    // Console input processing
    connect(ui->console, &QLineEdit::returnPressed, this, &MainWindow::commandsInConsole);
    connect(ui->enterConsole, &QPushButton::clicked, this, &MainWindow::commandsInConsole);
}


void MainWindow::setupConsoleCommands(const QStringList& commandList ) {
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
    qint32 x = pos.x();
    qint32 y = pos.y();
    qint32 w = width();
    qint32 h = height();

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



[[maybe_unused]] void MainWindow::selectLeftMenuElem(QModelIndex& index) {
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


void MainWindow::updateExitServerStyle(bool connect) {
    ui->updateExitServerStyle(connect);
}


void MainWindow::setMessage(const QString& name, const QString& message) {
    QString messageText = name + ": " + message;

    QLabel* messageLabel = new QLabel(messageText);
    messageLabel->setStyleSheet("color: #D8D8F6;");
    messageLabel->setWordWrap(true);
    messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    ui->messageContentLayout->addWidget(messageLabel);
}


/***    Custom windows      ***/


void MainWindow::showError(const QString& text) {
   ui->showError(text);
}


void MainWindow::showWarning(const QString& text) {
    ui->showWarning(text);
}


void MainWindow::showSuccess(const QString& text) {
    ui->showSuccess(text);
}


/***     Save/import settings       ***/


QString MainWindow::getUserName() {
    return ui->nameUsers->text();
}


void MainWindow::saveSettings() {
    settings->saveSettings(
            ui->componentGrid->isChecked(),
            ui->componentAxis->isChecked(),
            ui->nameUsers->text()
    );
}


void MainWindow::loadSettings() {
    bool gridChecked = false;
    bool axisChecked = false;

    QString userName;

    settings->loadSettings(gridChecked, axisChecked, userName);

    ui->componentGrid->setChecked(gridChecked);
    ui->componentAxis->setChecked(axisChecked);
    ui->nameUsers->setText(userName);

    ModeManager::setCell(gridChecked);
    ModeManager::setAxis(axisChecked);
}




QPushButton* MainWindow::getFirstBut() {
    return ui->oneReq;
}


QPushButton* MainWindow::getSecondBut() {
    return ui->twoReq;
}


QPushButton* MainWindow::getThirdBut() {
    return ui->threeReq;
}


QPushButton* MainWindow::getFourthBut() {
    return ui->fourReq;
}


QPushButton* MainWindow::getFifthBut() {
    return ui->fiveReq;
}


QPushButton* MainWindow::getSixthBut() {
    return ui->sixReq;
}


QPushButton* MainWindow::getSeventhBut() {
    return ui->sevenReq;
}


QPushButton* MainWindow::getEighthBut() {
    return ui->eightReq;
}


QPushButton* MainWindow::getNinthBut() {
    return ui->nineReq;
}


QPushButton* MainWindow::getTenthBut() {
   return ui->tenReq;
}


/// ***** PROTECTED:


void MainWindow::closeEvent(QCloseEvent* event) {

    if (!ModeManager::getSave()) {
        SaveDialog dialog(this);
        dialog.setModal(true);
        qint32 result = dialog.exec();  // Show the window

        if (result == QMessageBox::Yes) {
            saveProjectToFile(".ourp");
            if (ModeManager::getSave()) {
                saveSettings();
                event->accept();
                close();
            } else {
                event->ignore();
            }
        } else if (result == QMessageBox::No) {
            saveSettings();
            event->accept();
            close();
        } else {
            event->ignore();
        }

    } else {
        saveSettings();
        event->accept();
        close();
    }
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
    QPoint globalPos = event->globalPosition().toPoint();

    if (resizing) {
        QRect geom = originalGeometry;
        QPoint delta = globalPos - dragStartPos;

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
        QGestureEvent* gestureEvent = static_cast<QGestureEvent*>(event);
        if (QGesture* pinch = gestureEvent->gesture(Qt::PinchGesture)) {
            QPinchGesture* pinchGesture = static_cast<QPinchGesture*>(pinch);
            if (pinchGesture->changeFlags() & QPinchGesture::ScaleFactorChanged) {
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
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Tab) {
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
        QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
        if (event->key() == Qt::Key_Left) {
            bool isRightDownHalf = this->geometry() == QRect(screenGeometry.left() + screenGeometry.width() / 2,
                                                             screenGeometry.height() / 2,
                                                             screenGeometry.width() / 2,
                                                             screenGeometry.height() / 2);
            bool isRightTop = this->geometry() ==
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
            bool isLeftDownHalf = this->geometry() ==
                                  QRect(screenGeometry.left(), screenGeometry.height() / 2,
                                        screenGeometry.width() / 2,
                                        screenGeometry.height() / 2);
            bool isLeftUpHalf = this->geometry() ==
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
            bool isLeftDownHalf = this->geometry() ==
                                  QRect(screenGeometry.left(), screenGeometry.height() / 2,
                                        screenGeometry.width() / 2,
                                        screenGeometry.height() / 2);
            bool isRightDownHalf = this->geometry() == QRect(screenGeometry.left() + screenGeometry.width() / 2,
                                                             screenGeometry.height() / 2,
                                                             screenGeometry.width() / 2,
                                                             screenGeometry.height() / 2);
            bool isLeft = this->geometry() ==
                          QRect(screenGeometry.left(), screenGeometry.top(), screenGeometry.width() / 2,
                                screenGeometry.height());
            bool isRight = this->geometry() ==
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
            bool isOnRight =
                    this->geometry() ==
                    QRect(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.top(),
                          screenGeometry.width() / 2, screenGeometry.height());
            bool isOnLeft = this->geometry() ==
                            QRect(screenGeometry.left(), screenGeometry.top(), screenGeometry.width() / 2,
                                  screenGeometry.height());
            bool isRightTop = this->geometry() ==
                              QRect(screenGeometry.left() + screenGeometry.width() / 2, screenGeometry.top(),
                                    screenGeometry.width() / 2, screenGeometry.height() / 2);
            bool isLeftTop = this->geometry() ==
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


void MainWindow::loadProjectFile() {
    if (!ModeManager::getSave()) {
        SaveDialog dialog(this);
        qint32 result = dialog.exec();

        if (result == QMessageBox::Yes) {
            saveProjectToFile("ourp");
        } else if (result == QMessageBox::Cancel) {
            return;
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    projectsPath,
                                                    tr("Project Files (*.ourp);;All Files (*)"));

    if (!fileName.isEmpty()) {
        openProject(fileName);
    }
}


void MainWindow::saveProjectToFile(const QString& format) {
    QString extension = format.startsWith('.') ? format : "." + format;
    QString defaultName = "project";

    QDir baseDir(projectsPath);
    if (!baseDir.exists()) {
        QDir().mkpath(projectsPath);
    }

    QString defaultPath = QDir(projectsPath).filePath(defaultName + extension);
    QString selectedPath;

    while (true) {
        selectedPath = QFileDialog::getSaveFileName(
                this,
                tr("Save Project As"),
                defaultPath,
                tr("Project Files (*.ourp *.jpg *.jpeg *.png *.bmp *.svg *.tiff *.pdf);;All Files (*)"));

        if (selectedPath.isEmpty()) {
            ModeManager::setSave(false);
            return;
        }

        QFileInfo fileInfo(selectedPath);
        QString projectName = fileInfo.completeBaseName();
        QString selectedDir = fileInfo.absolutePath();
        QString mainFilePath;

        if (extension == ".ourp") {
            QString projectFolder = QDir(selectedDir).filePath(projectName);
            QDir dir(projectFolder);

            if (dir.exists()) {
                QMessageBox::StandardButton reply = QMessageBox::question(
                        this,
                        tr("Folder Exists"),
                        tr("The project folder already exists:\n%1\nDo you want to overwrite it?")
                                .arg(projectFolder),
                        QMessageBox::Yes | QMessageBox::No);

                if (reply != QMessageBox::Yes) {
                    continue;
                }

                if (!dir.removeRecursively()) {
                    QMessageBox::critical(this, tr("Error"),
                                          tr("Failed to remove existing project folder:\n%1").arg(projectFolder));
                    ModeManager::setSave(false);
                    return;
                }
            }

            if (!QDir().mkpath(projectFolder)) {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Failed to create project folder:\n%1").arg(projectFolder));
                ModeManager::setSave(false);
                return;
            }

            mainFilePath = QDir(projectFolder).filePath(projectName + extension);

            QFile file(mainFilePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::critical(this, tr("Error"), tr("Failed to create file:\n%1").arg(mainFilePath));
                ModeManager::setSave(false);
                return;
            }
            file.close();

        } else {
            mainFilePath = fileInfo.absoluteFilePath();
        }

        ModeManager::setSave(true);
        emit projectSaved(mainFilePath, extension);
        return;
    }
}


void MainWindow::buttonScript() {
    // Opening the project file selection dialog
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    QDir::homePath(),
                                                    tr("Project Files (*.txt);;All Files (*)"));

    if (!fileName.isEmpty()) {
        emit EmitScript(fileName);
    }
}


void MainWindow::openServer() {
    InputWindow* windowServer = new InputWindow("Enter port: ", this);
    QObject::connect(windowServer, &InputWindow::textEnter, [this](const QString& text) {
        emit SigOpenServer(text);
    });
    windowServer->show();
}


void MainWindow::joinServer() {
    InputWindow* windowServer = new InputWindow("Enter IP: ", this);
    QObject::connect(windowServer, &InputWindow::textEnter, [this](const QString& text) {
        emit SigJoinServer(text);
    });
    windowServer->show();
}


void MainWindow::joinLocalServer() {
    QStringList IPs = PortScanner().scanNetwork();
    CustomIPListWindow* windowServer = new CustomIPListWindow(IPs, this);
    QObject::connect(windowServer, &CustomIPListWindow::onConnectButtonClicked, [this](const QString& text) {
        emit SigJoinServer(text + ":2005");
    });
    windowServer->show();
}


void MainWindow::exitSession() {
    if (ModeManager::getConnection()) {
        emit SigExitSession();
    }
}


void MainWindow::Message() {
    QString input = ui->messageConsole->text();
    if (!input.isEmpty()) {
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
    saveProjectToFile(QString(".jpg"));
}


void MainWindow::onExportJPEG() {
    saveProjectToFile(".jpeg");
}


void MainWindow::onExportPNG() {
    saveProjectToFile(".png");
}


void MainWindow::onExportBMP() {
    saveProjectToFile(".bmp");
}


void MainWindow::onExportTIFF() {
    saveProjectToFile(".tiff");
}


void MainWindow::onExportPDF() {
    saveProjectToFile(".pdf");
}


void MainWindow::onExportOURP() {
    saveProjectToFile(".ourp");
}


void MainWindow::onExportSVG() {
    saveProjectToFile(".svg");
}


void MainWindow::onCreateProject() {
    InputWindow* inputWindow = new InputWindow("Path: ", this);
    inputWindow->setText(projectsPath + "/name");
    inputWindow->show();

    connect(inputWindow, &InputWindow::textEnter, this, [this](const QString& text) {

        if (text.contains('.')) {
            showError("Ошибка: имя проекта не должно содержать '.'");
            qDebug() << "Ошибка: имя проекта не должно содержать '.'";
            return;
        }

        if (text.isEmpty()) {
            showError("Отсутствие пути.");
            qDebug() << "Отсутствие пути.";
            return;
        }

        if (text.endsWith('/') || text.endsWith('\\')) {
            showError( "Ошибка: имя проекта не должно заканчиваться на '/' или '\\'");
            qDebug() << "Ошибка: имя проекта не должно заканчиваться на '/' или '\\'";
            return;
        }


        QDir dir(text);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                showError("Не удалось создать директорию: " + text);
                qDebug() << "Не удалось создать директорию:" << text;
                return;
            } else {
                qDebug() << "Директория создана:" << text;
            }
        } else {
            showError("Директория уже существует: " + text);
            qDebug() << "Директория уже существует:" << text;
            return;
        }

        QString name = QFileInfo(text).fileName();

        QString fileP = dir.filePath(name  + ".ourp");
        QFile file(fileP);

        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.close();
            qDebug() << "Файл создан:" << fileP;


            QString projectsFilePath = QDir::cleanPath(documentsPath + "/OurPaint/settings/projects.set");
            QFile fileSet(projectsFilePath);

            if (fileSet.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&fileSet);
                out << name << " - " << text << "\n";
                fileSet.close();
                qDebug() << "Проект записан в файл:" << projectsFilePath;
            } else {
                qDebug() << "Не удалось открыть файл для записи:" << projectsFilePath;
            }


            QString newText = name + ".ourp";

            QPair<QPushButton*, QTPainter*> result = ui->createTabProject(name);
            QPushButton* tabButton = result.first;
            QTPainter* painter = result.second;

            connect(tabButton, &QPushButton::clicked, [this, painter]() {
                emit changeTabs(painter);
            });

            emit createFile(painter);
            ui->inProject();
        } else {
            showError("Ошибка создания файла: " + file.errorString());
            qDebug() << "Ошибка создания файла:" << file.errorString();
        }
    });

}


void MainWindow::onLeftMenuRightClick(const QPoint &pos) {
    QModelIndex index = ui->leftMenuView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
    if (!node) {
        return;
    }

    if (node == leftMenuBar->getProjectsNode()) {
        QMenu menu;
        QAction* createTab = menu.addAction("Create tab");

        QAction* chosen = menu.exec(ui->leftMenuView->viewport()->mapToGlobal(pos));
        if (chosen == createTab) {
            InputWindow* wind = new InputWindow("Name:",this);
            wind->show();

            connect(wind,&InputWindow::textEnter, [this](const QString& text) {
                QString newText= text + ".ourp";
                QPair<QPushButton*, QTPainter*> result = ui->createTabProject(newText);
                QPushButton* tabButton = result.first;
                QTPainter* painter = result.second;

                connect(tabButton, &QPushButton::clicked, [this, painter]() {
                    emit changeTabs(painter);
                });
                emit createFile(painter);
            });

        }
    }
}


void MainWindow::setNameUsers(){
    QString input = ui->nameUsers->text();
    if (!input.isEmpty()) {
        ui->nameUsers->setEnabled(false);
        emit NameUsers(input);
    }
    ui->nameUsers->setEnabled(true);
}


void MainWindow::updateGrid(const bool checked){
    ModeManager::setCell(checked);
    ui->workWindow->update();
}


void MainWindow::updateAxis(const bool checked){
    ModeManager::setAxis(checked);
    ui->workWindow->update();
}


void MainWindow::commandsInConsole(){
    QString input = ui->console->text();
    if (!input.isEmpty()) {
        ui->console->pushBack(input);
        emit EnterPressed(input);
        ui->console->clear();
    }
}








