#include "Mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          ui(new Ui::MainWindow),
          moving(false),
          resizing(false),
          Index(0),
          helpWindow(nullptr),
          error(nullptr),
          warning(nullptr),
          success(nullptr) {

    ui->setupUi(this);
    setMouseTracking(true);
    setAllMouseTracking(this); // Отслеживание мыши
    setAttribute(Qt::WA_OpaquePaintEvent);

    painter = std::make_unique<QTPainter>(ui->workWindow); // Переопределяем метод рисования в виджете
    mouseWW = std::make_unique<MouseWorkWindow>(ui->workWindow);    // Переопределяем метод мышки в виджете
    keyWW = std::make_unique<KeyWorkWindow>(ui->workWindow);       // Переопределяем метод кнопок в виджете
   // leftMenuBar = std::make_unique<LeftMenuBar>(ui->leftMenu); // Класс для управления левым меню

    // painter->show();

    initConnections(); // инициализация сигналов

    this->setFocusPolicy(Qt::StrongFocus);
    this->installEventFilter(this);
    this->setFocus();

    ui->actionSave_project_to->installEventFilter(this);
    ui->projectButton->installEventFilter(this);
    ui->collaborationButton->installEventFilter(this);
    ui->menuProject->installEventFilter(this);
    ui->formatMenu->installEventFilter(this);
    ui->menuCollaboration->installEventFilter(this);
    setupLeftMenu();
}

void MainWindow::setupLeftMenu() {;
    leftMenuBar = std::make_unique<LeftMenuBar>(this);
    ui->leftMenuView->setModel(leftMenuBar->getTreeModel());
    ui->leftMenuView->setHeaderHidden(true);
}

// Инициализация всех сигналов
void MainWindow::initConnections() {
    // Кнопки режимов
    connect(ui->figurePoint, &QPushButton::clicked, this, &MainWindow::Point);
    connect(ui->figureCircle, &QPushButton::clicked, this, &MainWindow::Circle);
    connect(ui->figureSection, &QPushButton::clicked, this, &MainWindow::Section);
    connect(ui->figureSector, &QPushButton::clicked, this, &MainWindow::Sector);
    connect(ui->figureArc, &QPushButton::clicked, this, &MainWindow::Arc);
    connect(ui->figureMoving, &QPushButton::clicked, this, &MainWindow::FigMoving);
    connect(ui->toolMoving, &QPushButton::clicked, this, &MainWindow::ToolMoving);
    connect(ui->toolRotation, &QPushButton::clicked, this, &MainWindow::ToolRotation);
    connect(ui->toolResize, &QPushButton::clicked, this, &MainWindow::ToolResize);
    connect(ui->toolSelected, &QPushButton::clicked, this, &MainWindow::ToolSelected);

    // Кнопки Требований
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

    // Кнопки сохранение/импорт
    connect(ui->actionJPG,  &QToolButton::clicked, this, &MainWindow::onExportJPG);
    connect(ui->actionJPEG, &QToolButton::clicked, this, &MainWindow::onExportJPEG);
    connect(ui->actionPNG,  &QToolButton::clicked, this, &MainWindow::onExportPNG);
    connect(ui->actionBMP,  &QToolButton::clicked, this, &MainWindow::onExportBMP);
    connect(ui->actionTIFF, &QToolButton::clicked, this, &MainWindow::onExportTIFF);
    connect(ui->actionPDF,  &QToolButton::clicked, this, &MainWindow::onExportPDF);
    connect(ui->actionSVG,  &QToolButton::clicked, this, &MainWindow::onExportSVG);
    connect(ui->actionOURP, &QToolButton::clicked, this, &MainWindow::onExportOURP);
    connect(ui->actionImport_project, &QToolButton::clicked,this, &MainWindow::loadProjectFile);
    connect(ui->actionScript,&QToolButton::clicked, this, &MainWindow::buttonScript);

    // Кнопки сервера
    connect(ui->actionOpen_server, &QToolButton::clicked, this, &MainWindow::openServer);
    connect(ui->actionJoin_server, &QToolButton::clicked, this, &MainWindow::joinServer);
    connect(ui->actionJoin_local_server, &QToolButton::clicked, this, &MainWindow::joinLocalServer);
    connect(ui->actionExit_from_session, &QToolButton::clicked, this, &MainWindow::exitSession);

    // Кнопка помощь
    connect(ui->helpButton, &QPushButton::clicked, this, &MainWindow::showHelp);

    // Настройки имя
    connect(ui->nameUsers, &QLineEdit::returnPressed, this, [this]() {
        QString input = ui->nameUsers->text();
        if (!input.isEmpty()) {
            ui->nameUsers->setEnabled(false);
            emit NameUsers(input);
            emit changeSettings();
        }
        ui->nameUsers->setEnabled(true);
    });


    // Настройки сетка
    connect(ui->componentGrid, &QCheckBox::toggled, [&](bool checked) {
        ModeManager::setCell(checked);
        ui->workWindow->update();
    });
    connect(ui->componentAxis, &QCheckBox::toggled, [&](bool checked) {
        ModeManager::setAxis(checked);
        ui->workWindow->update();
    });

    // Обработка ввода в консоли
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

    QStringList commandList = {
            "circle ", "exit", "addreq ", "section ", "point ", "clear"
    };

    ui->console->setCommands(commandList);

    // Обработка ввода в консоль чата
    connect(ui->messageConsole, &QLineEdit::returnPressed, this, &MainWindow::Message);
    connect(ui->enterMes, &QPushButton::clicked, this, &MainWindow::Message);

    // При двойном нажатии на обьект откроется левое меню
  /*  connect(leftMenuBar.get(), &LeftMenuBar::showMenu, this, [this]() {
        ui->leftMenuContainer->show();
        ui->messageContainer->hide();
    });*/


   // connect(ui->leftMenu, &QTreeWidget::itemExpanded, leftMenuBar.get(), &LeftMenuBar::onItemExpanded);
    //connect(ui->leftMenu, &QTreeWidget::itemCollapsed, leftMenuBar.get(), &LeftMenuBar::onItemCollapsed);
    //  connect(ui->leftMenuElements, &QPushButton::clicked , leftMenuBar.get(),&LeftMenuBar::onItemCollapsed);

    connect(keyWW.get(), &KeyWorkWindow::UNDO,this, &MainWindow::undo);
    connect(keyWW.get(), &KeyWorkWindow::REDO,this, &MainWindow::redo);
    connect(keyWW.get(), &KeyWorkWindow::DELETE,this, &MainWindow::delet);

}


// Добавление сообщений
void MainWindow::setMessage(const std::string &name, const std::string &message) {
    QString messageText = QString::fromStdString(name) + ": " + QString::fromStdString(message);

    QLabel *messageLabel = new QLabel(messageText);
    messageLabel->setStyleSheet("color: #D8D8F6;");
    messageLabel->setWordWrap(true);
    messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    ui->messageContentLayout->addWidget(messageLabel);
}


// Обработка кнопки помощи
void MainWindow::showHelp() {
    if (!helpWindow) {
        helpWindow = new Help(this);
    }
    helpWindow->show();
    helpWindow->raise();
    helpWindow->activateWindow();
}


// Отслеживание мыши
void MainWindow::setAllMouseTracking(QWidget *widget) {
    widget->setMouseTracking(true);
    for (QObject *child: widget->children()) {
        if (QWidget *childWidget = qobject_cast<QWidget *>(child)) {
            setAllMouseTracking(childWidget);
        }
    }
}


/************************************************/

// Загрузка файла проекта
void MainWindow::loadProjectFile() {
    if (!ModeManager::getSave()) {
        SaveDialog dialog(this);
        int result = dialog.exec(); // Показать диалог

        if (result == QMessageBox::Yes) {
            saveProjectToFile("ourp");
        } else if (result == QMessageBox::Cancel) {
            return;
        }
    }

    // Открытие диалога выбора файла проекта
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    QDir::homePath() + "/OurPaint/project",
                                                    tr("Project Files (*.ourp);;All Files (*)"));

    if (!fileName.isEmpty()) {
        QString binFileName = fileName;
        if (binFileName.endsWith("ourp")) {
            binFileName.chop(5);
            binFileName += ".bin";
        }

        leftMenuBar->loadFromBinaryFile(binFileName);
        emit LoadFile(fileName); // Сигнал
    }
}


// Сохранение текущего проекта в файл
void MainWindow::saveProjectToFile(const QString& format) {
    QString baseName = "project";
    QString fileName;
    int index = 1; // Уникальности имени

    // Директория по умолчанию
    QString defaultDir = QDir::homePath() + "/OurPaint/project";

    QDir dir(defaultDir);
    if (!dir.exists()) {
        dir.mkpath("."); // Создание директории, если она не существует
    }


    fileName = QString("%1/%2").arg(defaultDir, baseName); // Формирование полного имени файла


    while (QFile::exists(fileName)) { // Проверка на существование файла
        fileName = QString("%1/%2_%3").arg(defaultDir).arg(baseName).arg(index);
        index++;
    }


    // Открытие диалога для сохранения файла
    QString selectedFileName = QFileDialog::getSaveFileName(this, tr("Save Project"), fileName,
                                                            tr("Project Files (*format);;All Files (*)"));


    if (!selectedFileName.isEmpty()) {
        ModeManager::setSave(true);
        leftMenuBar->saveToBinaryFile(selectedFileName+".bin");
        selectedFileName+=format;
        emit projectSaved(selectedFileName,format); //Сигнал
    } else {
       ModeManager::setSave(false);
    }
}


/********************************************/


void MainWindow::loadSettings(std::vector<bool> settings, const QString &name) {
    if (!settings.empty()) {
        ui->componentGrid->setChecked(settings[0]);
        ModeManager::setCell(settings[0]);
    }
    if (!name.isEmpty()) {
        ui->nameUsers->setText(name);
    }
}

void MainWindow::showError(const QString &text) {
    delete error;
    error = new CastomeWindowError(text, this);
    int x = this->width() - error->width() - 100;
    int y = this->height() - error->height() - 50;
    QPoint pos = this->mapToGlobal(QPoint(x, y));
    error->move(pos);
    error->show();
}

void MainWindow::showWarning(const QString &text) {
    delete warning;
    warning = new CastomeWindowWarning(text, this);
    int x = this->width() - warning->width() - 100;
    int y = this->height() - warning->height() - 50;
    QPoint pos = this->mapToGlobal(QPoint(x, y));
    warning->move(pos);
    warning->show();
}

void MainWindow::showSuccess(const QString &text) {
    delete success;
    success = new CastomeWindowSuccessful(text, this);
    int x = this->width() - success->width() - 150;
    int y = this->height() - success->height() - 50;
    QPoint pos = this->mapToGlobal(QPoint(x, y));
    success->move(pos);
    success->show();
}



/////////////////////////////////////////////////////////////
// Кнопки сервака

void MainWindow::buttonScript() {
    // Открытие диалога выбора файла проекта
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    QDir::homePath(),
                                                    tr("Project Files (*.txt);;All Files (*)"));

    if (!fileName.isEmpty()) {
        emit EmitScript(fileName); // Signal
    }
}

// Обработка кнопки сервера
void MainWindow::openServer() {
    WindowServer *windowServer = new WindowServer("Enter port: ");
    QObject::connect(windowServer, &WindowServer::textEnter, [this](const QString &text) {
        emit SigOpenServer(text);
    });
    windowServer->show();
}


// Обработка кнопки сервера
void MainWindow::joinServer() {
    WindowServer *windowServer = new WindowServer("Enter IP: ");
    QObject::connect(windowServer, &WindowServer::textEnter, [this](const QString &text) {
        emit SigJoinServer(text);
    });
    windowServer->show();
}


// Обработка кнопки сервера
void MainWindow::joinLocalServer() {
    QStringList IPs = PortScanner().scanNetwork();
    CastomeIPListWindow *windowServer = new CastomeIPListWindow(IPs, this);
    QObject::connect(windowServer, &CastomeIPListWindow::onConnectButtonClicked, [this](const QString &text) {
        emit SigJoinServer(text + ":2005");
    });
    windowServer->show();
}

///////////////////////////////////////////

MainWindow::~MainWindow() {
    delete ui;
    delete helpWindow;
    delete warning;
    delete success;
    delete error;
}

/****************************************************************************************
 *        // Protected
 *  Функции перемещения и масштабирования
 *
 ***************************************************************************************/
void  MainWindow::updateStyle(){

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;

    if (!isMaximized() && !isFullScreen()) {

        path.addRoundedRect(0, 0, width(), height(), 10, 10);

        ui->topBar->setStyleSheet("QWidget#topBar { "
                                  "background-color: #494850; "
                                  "color: #D8D8F6; "
                                  "border-top-left-radius: 10px; "
                                  "border-top-right-radius: 10px; "
                                  "border-bottom-left-radius: 0px; "
                                  "border-bottom-right-radius: 0px; "
                                  "border: none; "
                                  "border-bottom: 1px solid #262222; }");

      /*  ui->leftMenu->setStyleSheet(QString::fromUtf8(R"(
        background: #494850;
        color: #D8D8F6;
        border: none; /* Убираем все границы */
      //  border-bottom-left-radius: 10px;   /* Закругление нижнего левого угла */
        //border-bottom-right-radius: 0px;
    //)"));

        ui->messageConsole->setStyleSheet(QString::fromUtf8(R"(
        background: "#3e3d3d";
        color: "#D8D8F6";
        border: 1px solid black;
        border-bottom-left-radius: 10px;
        border-bottom-right-radius: 0px;
    )"));
        ui->collapsedPanel->setStyleSheet("background-color: #494850; border-bottom-left-radius: 10px;\n"
                                          "border-bottom-right-radius: 0px;");
    } else {
        path.addRect(0, 0, width(), height());
        ui->topBar->setStyleSheet("QWidget#topBar { "
                                  "background-color: #494850; "
                                  "color: #D8D8F6; "
                                  "border-radius: 0px; "
                                  "border: none; "
                                  "border-bottom: 1px solid #262222; }");

       // ui->leftMenu->setStyleSheet(QString::fromUtf8(R"(
        //background: #494850;
        //color: #D8D8F6;
        //QHeaderView::section {
          //  background: #494850;
           // color: #D8D8F6;
       // }
   // )"));
        ui->messageConsole->setStyleSheet(QString::fromUtf8(R"(
        background: "#3e3d3d";
        color: "#D8D8F6";
        border: 1px solid black;
    )"));
        ui->collapsedPanel->setStyleSheet("background-color: #494850; ");
    }


    painter.setClipPath(path);
    painter.fillPath(path, QColor("#978897"));
    painter.drawPath(path);

}

// Отрисовка 60 раз в секундк
void MainWindow::resizeEvent(QResizeEvent *event) {
    updateStyle();
    emit resize();
    ui->workWindow->update();
    QMainWindow::resizeEvent(event);
}


// Обработка события закрытия окна
void MainWindow::closeEvent(QCloseEvent *event) {
    if (!ModeManager::getSave()) {
        SaveDialog dialog(this);
        dialog.setModal(true);
        int result = dialog.exec(); // Показать окно

        if (result == QMessageBox::Yes) {
            saveProjectToFile(".ourp");
            if (ModeManager::getSave()) {
                event->accept();
                close();
            } else {
                event->ignore();
            }
        } else if (result == QMessageBox::No) {
            event->accept();
            close();
        } else {
            event->ignore();
        }

    } else {
        event->accept();
        close();
    }
}

/****************************************************************/

///          Обработка мыши

void MainWindow::paintEvent(QPaintEvent *event) {

}

// Обработка нажатий мыши
void MainWindow::mousePressEvent(QMouseEvent *event) {
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


// Перемщение курсора
void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    QPoint globalPos = event->globalPosition().toPoint();

    if (resizing) {
        QRect geom = originalGeometry;
        QPoint delta = globalPos - dragStartPos;

        switch (currentRegion) {
            case Top: geom.setTop(geom.top() + delta.y()); break;
            case Bottom: geom.setBottom(geom.bottom() + delta.y()); break;
            case Left: geom.setLeft(geom.left() + delta.x()); break;
            case Right: geom.setRight(geom.right() + delta.x()); break;
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
            default: break;
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

void MainWindow::updateShapeCursor(const QPoint &pos) {
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

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
        case BottomRight: setCursor(Qt::SizeFDiagCursor); break;
        case TopRight:
        case BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
        case Left:
        case Right: setCursor(Qt::SizeHorCursor); break;
        case Top:
        case Bottom: setCursor(Qt::SizeVerCursor); break;
        default: setCursor(Qt::ArrowCursor); break;
    }
}



// Обработка отпускания мыши
void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ArrowCursor);
        resizing = false;
        moving = false;
        event->accept();
    } else {
        QMainWindow::mouseReleaseEvent(event);
    }
}


// Двойное нажатие мыши
void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        moving = true;
        dragStartPos = event->globalPosition().toPoint();
    }

    QMainWindow::mouseDoubleClickEvent(event);
}


// Кручение колёсиком или тачпадом
void MainWindow::wheelEvent(QWheelEvent *event) {
    if (ui->workWindow && ui->workWindow->underMouse()) { // В области workWindow
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


// Жесты в тачпаде в области workWindow
bool MainWindow::event(QEvent *event) {

    if (event->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(event);
        if (QGesture *pinch = gestureEvent->gesture(Qt::PinchGesture)) {
            QPinchGesture *pinchGesture = static_cast<QPinchGesture *>(pinch);
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

///////////////////////////////////////////////////////////////
// клава


bool MainWindow::eventFilter(QObject *obj, QEvent *event) {

    if (obj == ui->workWindow) {
        ui->workWindow->update();
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Tab) {
            event->accept();

            if (ui->workWindow) {
                if (keyWW) {
                    keyWW->handleKeyPress(keyEvent);
                }
            }
            return true;
            // Я красивый белорус!!!
        }
    }else if (obj == ui->actionSave_project_to) {
        if (event->type() == QEvent::Enter) {
            QPoint pos = ui->actionSave_project_to->mapToGlobal(QPoint(ui->actionSave_project_to->width(), 0));
            ui->formatMenu->popup(pos);
            return true;
        }
    }else if (obj == ui->projectButton && event->type() == QEvent::Enter) {
        QPoint pos = ui->projectButton->mapToGlobal(QPoint(0, ui->projectButton->height()));
        ui->menuProject->exec(pos);
        return true;
    }
    else if (obj == ui->collaborationButton && event->type() == QEvent::Enter) {
        QPoint pos = ui->collaborationButton->mapToGlobal(QPoint(0, ui->collaborationButton->height()));
        ui->menuCollaboration->exec(pos);
        return true;
    }
    else if (obj == ui->menuCollaboration && event->type() == QEvent::Leave) {
        QTimer::singleShot(100, this, [this]() {
            if (!ui->menuCollaboration->underMouse()) {
                ui->menuCollaboration->close();
            }
        });
        return true;
    }
    else if (obj == ui->menuProject && event->type() == QEvent::Leave) {
        QTimer::singleShot(100, this, [this]() {
            if (!ui->menuProject->underMouse() && !ui->formatMenu->underMouse()) {
                ui->menuProject->close();
                ui->formatMenu->close();
            }
        });
        return true;
    }
    else if (obj == ui->formatMenu && event->type() == QEvent::Leave) {
        QTimer::singleShot(100, this, [this]() {
            if (!ui->formatMenu->underMouse()) {
                ui->formatMenu->close();
                if (!ui->menuProject->underMouse()) {
                    ui->menuProject->close();
                }
            }
        });
    }

    return QObject::eventFilter(obj, event);
}


// Обработка нажатий клавиш
void MainWindow::keyPressEvent(QKeyEvent *event) {

    // Обрадотка комбинаций ctrl + стрелочки -> перемещает окно в разные положения
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


    // Буфер команд
    if (ui->console->isActiveWindow()) { // Если консоль активна
        if (event->key() == Qt::Key_Up) { // Кнопка вверх
            if (Index == 0) {
                Index = static_cast<int>(commands.size()) - 1;
            } else {
                Index = (Index + 1) % commands.size();
            }
            ui->console->setText(commands[Index]);
        } else if (event->key() == Qt::Key_Down) { // Кнопка вниз
            Index = (Index - 1 + commands.size()) % commands.size();
            ui->console->setText(commands[Index]);
        }
    }



    if (event->key() == Qt::Key_Tab) {
        event->ignore();
        ui->workWindow->setFocus();
        return;
    }

    QWidget::keyPressEvent(event); // Вызов базового обработчика
}







