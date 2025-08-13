#include "Mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent),
          ui(new Ui::MainWindow),
          helpWindow(nullptr),
          error(nullptr),
          warning(nullptr),
          success(nullptr),
          documentsPath(),
          projectsPath(),
          settingsPath(),
          commands(),
          Index(0),
          resizing(false),
          moving(false),
          dragStartPos(),
          originalGeometry(),
          currentRegion(None)
{
    ui->setupUi(this);
    setMouseTracking(true);
    setAllMouseTracking(this); // Mouse Tracking
    setAttribute(Qt::WA_OpaquePaintEvent);

    this->setFocusPolicy(Qt::StrongFocus);
    this->installEventFilter(this);
    this->setFocus();

    documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    projectsPath = QDir::cleanPath(documentsPath + "/OurPaint/projects");
    settingsPath = QDir::cleanPath(documentsPath + "/OurPaint/settings/settings.set");

    painter = new QTPainter(ui->workWindow);
    mouseWW = new MouseWorkWindow(ui->workWindow);
    keyWW = new KeyWorkWindow(ui->workWindow);
    settings = new Settings(settingsPath);

    loadSettings();
    initConnections(); // Initialization of signals
    setupLeftMenu();
}

MainWindow::~MainWindow() {
    delete ui;
    delete helpWindow;
    delete warning;
    delete success;
    delete error;
}

QTPainter* MainWindow::getQTPainter() const {
    return painter;
}

LeftMenuBar* MainWindow::getLeftMenuBar() const {
    return leftMenuBar;
}

void MainWindow::initConnections() {
    // Mode buttons
    connect(ui->figurePoint, &QPushButton::clicked, this, &MainWindow::Point);
    connect(ui->figureCircle, &QPushButton::clicked, this, &MainWindow::Circle);
    connect(ui->figureSection, &QPushButton::clicked, this, &MainWindow::Section);
    connect(ui->figureArc, &QPushButton::clicked, this, &MainWindow::Arc);
    connect(ui->figureMoving, &QPushButton::clicked, this, &MainWindow::FigMoving);
    connect(ui->toolMoving, &QPushButton::clicked, this, &MainWindow::ToolMoving);
    connect(ui->toolSelected, &QPushButton::clicked, this, &MainWindow::ToolSelected);

    // Requirements Buttons
    connect(ui->oneReq, &QPushButton::clicked, this, &MainWindow::firstReq);
    connect(ui->twoReq, &QPushButton::clicked, this, &MainWindow::secondReq);
    connect(ui->threeReq, &QPushButton::clicked, this, &MainWindow::thirdReq);
    connect(ui->fourReq, &QPushButton::clicked, this, &MainWindow::fourthReq);
    connect(ui->fiveReq, &QPushButton::clicked, this, &MainWindow::fifthReq);
    connect(ui->sixReq, &QPushButton::clicked, this, &MainWindow::sixthReq);
    connect(ui->sevenReq, &QPushButton::clicked, this, &MainWindow::seventhReq);
    connect(ui->eightReq, &QPushButton::clicked, this, &MainWindow::eighthReq);
    connect(ui->nineReq, &QPushButton::clicked, this, &MainWindow::ninthReq);
    connect(ui->tenReq, &QPushButton::clicked, this, &MainWindow::tenthReq);

    connect(keyWW, &KeyWorkWindow::firstReq, this, &MainWindow::firstReq);
    connect(keyWW, &KeyWorkWindow::secondReq, this, &MainWindow::secondReq);
    connect(keyWW, &KeyWorkWindow::thirdReq, this, &MainWindow::thirdReq);
    connect(keyWW, &KeyWorkWindow::fourthReq, this, &MainWindow::fourthReq);
    connect(keyWW, &KeyWorkWindow::fifthReq, this, &MainWindow::fifthReq);
    connect(keyWW, &KeyWorkWindow::sixthReq, this, &MainWindow::sixthReq);
    connect(keyWW, &KeyWorkWindow::seventhReq, this, &MainWindow::seventhReq);
    connect(keyWW, &KeyWorkWindow::eighthReq, this, &MainWindow::eighthReq);
    connect(keyWW, &KeyWorkWindow::ninthReq, this, &MainWindow::ninthReq);
    connect(keyWW, &KeyWorkWindow::tenthReq, this, &MainWindow::tenthReq);

    // Save/import buttons
    connect(ui->actionJPG, &QToolButton::clicked, this, &MainWindow::onExportJPG);
    connect(ui->actionJPEG, &QToolButton::clicked, this, &MainWindow::onExportJPEG);
    connect(ui->actionPNG, &QToolButton::clicked, this, &MainWindow::onExportPNG);
    connect(ui->actionBMP, &QToolButton::clicked, this, &MainWindow::onExportBMP);
    connect(ui->actionTIFF, &QToolButton::clicked, this, &MainWindow::onExportTIFF);
    connect(ui->actionPDF, &QToolButton::clicked, this, &MainWindow::onExportPDF);
    connect(ui->actionSVG, &QToolButton::clicked, this, &MainWindow::onExportSVG);
    connect(ui->actionOURP, &QToolButton::clicked, this, &MainWindow::onExportOURP);
    connect(ui->actionImport_project, &QToolButton::clicked, this, &MainWindow::loadProjectFile);
    connect(ui->actionScript, &QToolButton::clicked, this, &MainWindow::buttonScript);

    // Server buttons
    connect(ui->actionOpen_server, &QToolButton::clicked, this, &MainWindow::openServer);
    connect(ui->actionJoin_server, &QToolButton::clicked, this, &MainWindow::joinServer);
    connect(ui->actionJoin_local_server, &QToolButton::clicked, this, &MainWindow::joinLocalServer);
    connect(ui->actionExit_from_session, &QToolButton::clicked, this, &MainWindow::exitSession);

    // Help button
    connect(ui->helpButton, &QPushButton::clicked, this, &MainWindow::showHelp);

    // Processing input to the chat console
    connect(ui->messageConsole, &QLineEdit::returnPressed, this, &MainWindow::Message);
    connect(ui->enterMes, &QPushButton::clicked, this, &MainWindow::Message);

    connect(keyWW, &KeyWorkWindow::UNDO, this, &MainWindow::undo);
    connect(keyWW, &KeyWorkWindow::REDO, this, &MainWindow::redo);
    connect(keyWW, &KeyWorkWindow::DELETE, this, &MainWindow::deleteButton);
    connect(keyWW, &KeyWorkWindow::CUT, this, &MainWindow::cut);
    connect(keyWW, &KeyWorkWindow::PASTE, this, &MainWindow::paste);
    connect(keyWW, &KeyWorkWindow::COPY, this, &MainWindow::copy);

    // Settings name
    connect(ui->nameUsers, &QLineEdit::returnPressed, this, [this]() {
        QString input = ui->nameUsers->text();
        if (!input.isEmpty()) {
            ui->nameUsers->setEnabled(false);
            emit NameUsers(input);
        }
        ui->nameUsers->setEnabled(true);
    });

    // Grid Settings
    connect(ui->componentGrid, &QCheckBox::toggled, [&](bool checked) {
        emit gridToggled(checked);
        ui->workWindow->update();
    });

    connect(ui->componentAxis, &QCheckBox::toggled, [&](bool checked) {
        ModeManager::setAxis(checked);
        ui->workWindow->update();
    });

    // Console input processing
    connect(ui->console, &QLineEdit::returnPressed, this, [this]() {
        QString input = ui->console->text();
        if (!input.isEmpty()) {
            commands.push_back(input);
            emit EnterPressed(input);
            ui->console->clear();
        }
    });
    connect(ui->enterConsole, &QPushButton::clicked, this, [this]() {
        QString input = ui->console->text();
        if (!input.isEmpty()) {
            commands.push_back(input);
            emit EnterPressed(input);
            ui->console->clear();
        }
    });


}


void MainWindow::setupConsoleCommands(const QStringList& commandList ) {
    ui->console->setCommands(commandList);
}


void MainWindow::setupLeftMenu() {
    leftMenuBar = new LeftMenuBar(this);
    ui->leftMenuView->setModel(leftMenuBar->getTreeModel());
    ui->leftMenuView->setHeaderHidden(true);
    ui->leftMenuView->setItemDelegate(new ParameterDelegate(leftMenuBar));
    connect(static_cast<ParameterDelegate*>(ui->leftMenuView->itemDelegate()),
            &ParameterDelegate::doubleClickOnID,
            leftMenuBar,
            &LeftMenuBar::doubleClickID);
}

void MainWindow::selectLeftMenuElem(QModelIndex& index) {
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

void MainWindow::updateStyle() {
    if (!isMaximized() && !isFullScreen()) {
        ui->topBar->setStyleSheet("QWidget#topBar { "
                                  "background-color: #494850; "
                                  "color: #D8D8F6; "
                                  "border-top-left-radius: 10px; "
                                  "border-top-right-radius: 10px; "
                                  "border: none; "
                                  "border-bottom: 1px solid #262222; }");
        ui->collapsedPanel->setStyleSheet("QWidget#collapsedPanel { "
                                          "background-color: #494850;"
                                          "color: #D8D8F6;"
                                          "border-bottom-left-radius: 10px;"
                                          "}");
        ui->centralwindow->setStyleSheet("QWidget#centralwindow { "
                                         "background-color: #978897;"
                                         "color: #D8D8F6;"
                                         "border-top-left-radius: 10px; "
                                         "border-top-right-radius: 10px; "
                                         "border-bottom-left-radius: 10px; "
                                         "border-bottom-right-radius: 10px; "
                                         "border: none; }");
        ui->messageConsole->setStyleSheet(R"(
        background-color: #3e3d3d;
        color: #D8D8F6;
        border: 1px solid black;
        border-bottom-left-radius: 10px;
        border-bottom-right-radius: 0px;
    )");
        ui->leftMenuView->setStyleSheet(R"(
        QTreeView {
            background-color: #494850;
            color: #D8D8F6;
            border: none;
            border-bottom-left-radius: 10px;
            border-bottom-right-radius: 0px;
        }
    )");

    } else {
        ui->topBar->setStyleSheet("QWidget#topBar { "
                                  "background-color: #494850; "
                                  "color: #D8D8F6; "
                                  "border-top-left-radius: 0px; "
                                  "border-top-right-radius: 0px; "
                                  "border: none; "
                                  "border-bottom: 1px solid #262222; }");
        ui->collapsedPanel->setStyleSheet("QWidget#collapsedPanel { "
                                          "background-color: #494850;"
                                          "color: #D8D8F6;"
                                          "border-bottom-left-radius: 0px;"
                                          "}");
        ui->centralwindow->setStyleSheet("QWidget#centralwindow { "
                                         "background-color: #978897;"
                                         "color: #D8D8F6;"
                                         "border-top-left-radius: 0px; "
                                         "border-top-right-radius: 0px; "
                                         "border: none; }");
        ui->messageConsole->setStyleSheet(R"(
        background-color: #3e3d3d;
        color: #D8D8F6;
        border: 1px solid black;
        border-bottom-left-radius: 0px;
        border-bottom-right-radius: 0px;
    )");
        ui->leftMenuView->setStyleSheet(R"(
        QTreeView {
            background-color: #494850;
            color: #D8D8F6;
            border: none;
            border-bottom-left-radius: 0px;
            border-bottom-right-radius: 0px;
        }
    )");
    }
}

void MainWindow::updateExitServerStyle(bool connect) {
    if (!connect) {
        ui->actionExit_from_session->setStyleSheet(
                "QToolButton#actionExit_from_session {"
                "   background-color: transparent;"
                "   border: none;"
                "   font-size: 9pt;"
                "   color: #8888A0;"
                "   padding: 5px;"
                "}"
        );
    } else {
        ui->actionExit_from_session->setStyleSheet(
                "QToolButton#actionExit_from_session {"
                "   background-color: transparent;"
                "   border: none;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px;"
                "}"
                "QToolButton#actionExit_from_session:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
        );
    }
}

void MainWindow::updateShapeCursor(const QPoint& pos) {
    qint32 x = pos.x();
    qint32 y = pos.y();
    qint32 w = width();
    qint32 h = height();

    ResizeRegion region = None;

    if (x < edgeMargin && y < edgeMargin) region = TopLeft;
    else if (x > w - edgeMargin && y < edgeMargin) region = TopRight;
    else if (x < edgeMargin && y > h - edgeMargin) region = BottomLeft;
    else if (x > w - edgeMargin && y > h - edgeMargin) region = BottomRight;
    else if (x < edgeMargin) region = Left;
    else if (x > w - edgeMargin) region = Right;
    else if (y < edgeMargin) region = Top;
    else if (y > h - edgeMargin) region = Bottom;

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

void MainWindow::setMessage(const std::string& name, const std::string& message) {
    QString messageText = QString::fromStdString(name) + ": " + QString::fromStdString(message);

    QLabel* messageLabel = new QLabel(messageText);
    messageLabel->setStyleSheet("color: #D8D8F6;");
    messageLabel->setWordWrap(true);
    messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    ui->messageContentLayout->addWidget(messageLabel);
}

void MainWindow::setAllMouseTracking(QWidget* widget) {
    widget->setMouseTracking(true);
    for (QObject* child: widget->children()) {
        if (QWidget* childWidget = qobject_cast<QWidget*>(child)) {
            setAllMouseTracking(childWidget);
        }
    }
}

void MainWindow::showHelp(){
    if (!helpWindow) {
        helpWindow = new Help(this);
    }
    helpWindow->show();
    helpWindow->raise();
    helpWindow->activateWindow();
}

void MainWindow::showError(const QString& text) {
    delete error;
    error = new CustomWindowError(text, this);
    qint32 x = this->width() - error->width() - 100;
    qint32 y = this->height() - error->height() - 50;
    QPoint pos = this->mapToGlobal(QPoint(x, y));
    error->move(pos);
    error->show();
}

void MainWindow::showWarning(const QString& text) {
    delete warning;
    warning = new CustomWindowWarning(text, this);
    qint32 x = this->width() - warning->width() - 100;
    qint32 y = this->height() - warning->height() - 50;
    QPoint pos = this->mapToGlobal(QPoint(x, y));
    warning->move(pos);
    warning->show();
}

void MainWindow::showSuccess(const QString& text) {
    delete success;
    success = new CustomWindowSuccessful(text, this);
    qint32 x = this->width() - success->width() - 150;
    qint32 y = this->height() - success->height() - 50;
    QPoint pos = this->mapToGlobal(QPoint(x, y));
    success->move(pos);
    success->show();
}

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
    bool gridChecked=false;
    bool axisChecked=false;

    QString userName;

    settings->loadSettings(gridChecked, axisChecked, userName);

    ui->componentGrid->setChecked(gridChecked);
    ui->componentAxis->setChecked(axisChecked);
    ui->nameUsers->setText(userName);

    ModeManager::setCell(gridChecked);
    ModeManager::setAxis(axisChecked);
}

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

void MainWindow::resizeEvent(QResizeEvent* event) {
    updateStyle();
    emit resize();
    ui->workWindow->update();
    QMainWindow::resizeEvent(event);
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

    if (obj == ui->workWindow) {
        ui->workWindow->update();
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Tab) {
            event->accept();

            if (ui->workWindow) {
                if (keyWW) {
                    keyWW->handleKeyPress(keyEvent);
                }
            }
            return true;
        } else if (!ui->console->isActiveWindow() &&
                   keyEvent->key() != Qt::ControlModifier &&
                   (keyEvent->key() == Qt::Key_Up ||
                    keyEvent->key() == Qt::Key_Down ||
                    keyEvent->key() == Qt::Key_Right ||
                    keyEvent->key() == Qt::Key_Left)) {
            return true;
        }

    }


    return QObject::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {

    // // Processing ctrl + arrow keys -> moves the window to different positions
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


    // Command Buffer
    if (ui->console->isActiveWindow() && !commands.empty()) {
        if (event->key() == Qt::Key_Up) {
            Index = (Index - 1 + commands.size()) % commands.size();
            ui->console->setText(commands[Index]);
        } else if (event->key() == Qt::Key_Down) {
            Index = (Index + 1) % commands.size();
            ui->console->setText(commands[Index]);
        }
    }

    if (event->key() == Qt::Key_Tab) {
        event->ignore();
        ui->workWindow->setFocus();
        return;
    }

    QWidget::keyPressEvent(event);
}

void MainWindow::cut() {
    emit CUT();
};

void MainWindow::paste() {
    emit PASTE();
};

void MainWindow::copy() {
    emit COPY();
};

void MainWindow::redo() {
    emit REDO();
};

void MainWindow::undo() {
    emit UNDO();
};

void MainWindow::deleteButton() {
    emit DELETE();
};

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
        QString binFileName = fileName;
        if (binFileName.endsWith("ourp")) {
            binFileName.chop(5);
        }


        emit LoadFile(fileName);
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
        QString binFilePath;

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
    if (ModeManager::getConnection()) { emit SigExitSession(); }
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
};

void MainWindow::Section() {
    ModeManager::setActiveMode(WorkModes::Section);
};

void MainWindow::Circle() {
    ModeManager::setActiveMode(WorkModes::Circle);
};

void MainWindow::Arc() {
    ModeManager::setActiveMode(WorkModes::Arc);
};

void MainWindow::FigMoving() {
    ModeManager::setActiveMode(WorkModes::Editor);
};

void MainWindow::ToolMoving() {
    ModeManager::setActiveMode(WorkModes::Move);
};

void MainWindow::ToolSelected() {
    ModeManager::setActiveMode(WorkModes::Selected);
};

void MainWindow::onWorkWindowResized() {
    emit resize();
};

void MainWindow::firstReq() {
    emit oneRequirements();
}

void MainWindow::secondReq() {
    emit twoRequirements();
}

void MainWindow::thirdReq() {
    emit threeRequirements();
}

void MainWindow::fourthReq() {
    emit fourRequirements();
}

void MainWindow::fifthReq() {
    emit fiveRequirements();
}

void MainWindow::sixthReq() {
    emit sixRequirements();
}

void MainWindow::seventhReq() {
    emit sevenRequirements();
}

void MainWindow::eighthReq() {
    emit eightRequirements();
}

void MainWindow::ninthReq() {
    emit nineRequirements();
}

void MainWindow::tenthReq() {
    emit tenRequirements();
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
void MainWindow::onGridToggled(bool state) {
    ui->componentGrid->setChecked(state);
    ui->workWindow->update();
}








