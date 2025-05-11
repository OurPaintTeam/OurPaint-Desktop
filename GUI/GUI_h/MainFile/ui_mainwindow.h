#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QTextBrowser>
#include <QRegion>
#include <QPainterPath>
#include <QBitmap>
#include <QPainter>
#include <QIcon>
#include <QCheckBox>
#include <QPropertyAnimation>
#include <QTextEdit>
#include <QLabel>
#include <QScrollArea>
#include <QToolButton>
#include <QWidgetAction>

#include "CastomeConsole.h"
#include "SmileRightClickFilter.h"
#include "EmojiWidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow {
public:
    // Actions
    QToolButton *actionSave_project_to;
    QToolButton* actionJPG;
    QToolButton* actionJPEG;
    QToolButton* actionPNG;
    QToolButton* actionBMP;
    QToolButton* actionTIFF;
    QToolButton* actionPDF;
    QToolButton* actionSVG;
    QToolButton* actionOURP;
    QToolButton *actionImport_project;
    QToolButton *actionScript;

    QWidgetAction* saveAction;
    QWidgetAction* widgetJPG;
    QWidgetAction* widgetJPEG;
    QWidgetAction* widgetPNG;
    QWidgetAction* widgetBMP;
    QWidgetAction* widgetTIFF;
    QWidgetAction* widgetPDF;
    QWidgetAction* widgetOURP;
    QWidgetAction* widgetSVG;
    QWidgetAction *widgetImport_project;
    QWidgetAction *widgetScript;

    QToolButton *actionOpen_server;
    QToolButton *actionJoin_server;
    QToolButton *actionJoin_local_server;
    QToolButton *actionExit_from_session;
    QAction *action_help;

    QWidgetAction *widgetOpen_server;
    QWidgetAction *widgetJoin_server;
    QWidgetAction *widgetJoin_local_server;
    QWidgetAction *widgetExit_from_session;

    // Main widgets
    QWidget *centralwindow;
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    CastomeConsole *console;
    QFrame *workWindow;
    QWidget *topBar;
    QHBoxLayout *topBarLayout;
    QPushButton *enterConsole;
    QHBoxLayout *layoutConsole;
    QTreeView* leftMenuView;


    // Menu
    QMenu *menuProject;
    QMenu* formatMenu;
    QMenu *menuCollaboration;

    // Menu buttons
    QPushButton *projectButton;
    QPushButton *collaborationButton;
    QPushButton *helpButton;
    QPushButton *settings;

    // Window control buttons
    QPushButton *closeButton;
    QPushButton *minimizeButton;
    QPushButton *maximizeButton;

    // Left menu
    QPushButton *leftMenuElements;
    QWidget *leftMenuContainer;
    QGridLayout *leftMenuLayout;
    QWidget *collapsedPanel;
    QPushButton *collapseButton;
    QVBoxLayout *collapsedPanelLayout;
    QPushButton *leftMenuMessage;
    QPushButton *Figures;
    QPushButton *Tools;
    QPushButton *Req;

    // Panel and figures buttons
    QWidget *figuresPanel;
    QPushButton *figurePoint;
    QPushButton *figureSection;
    QPushButton *figureCircle;
 //   QPushButton *figureSector;
    QPushButton *figureArc;

    // Tools panel
    QWidget *toolPanel;
    QPushButton *figureMoving;
    QPushButton *toolMoving;
   // QPushButton *toolRotation;
    //QPushButton *toolResize;
    QPushButton *toolSelected;

    // Panel for requirements
    QWidget *reqPanel;
    QPushButton *oneReq;
    QPushButton *twoReq;
    QPushButton *threeReq;
    QPushButton *fourReq;
    QPushButton *fiveReq;
    QPushButton *sixReq;
    QPushButton *sevenReq;
    QPushButton *eightReq;
    QPushButton *nineReq;
    QPushButton *tenReq;

    // Animation for panels
    QPropertyAnimation *figuresPanelAnimation;
    QPropertyAnimation *toolsPanelAnimation;
    QPropertyAnimation *reqPanelAnimation;

    // Chat
    QFrame *message;
    QLineEdit *messageConsole;
    QPushButton *messageCollapseButton;
    QGridLayout *messageLayout;
    QWidget *messageContainer;
    QGridLayout *messageContainerLayout;
    QWidget *messageContent;
    QVBoxLayout *messageContentLayout;
    QScrollArea *messageScrollArea;
    QHBoxLayout *layoutMessage;
    QPushButton *enterMes;
    QPushButton *smile;
    EmojiWidget *emojiWidget;
    QHBoxLayout *nameLayout;
    QLabel *nameLabel;

    // Settings panel
    QWidget *settingsPanel;
    QVBoxLayout *settingsLayout;
    QCheckBox *componentGrid;
    QCheckBox *componentAxis;
    QLineEdit *nameUsers;

    void setupUi(QMainWindow *MainWindow) {

        // Установка флагов окна
        MainWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
        MainWindow->setAttribute(Qt::WA_TranslucentBackground); // Прозрачность
//
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("OurPaint");

        MainWindow->setStyleSheet(
                "QWidget { "
                "color: #D8D8F6; "
                "}"
        );

        MainWindow->resize(960, 540);
        MainWindow->setMinimumSize(QSize(672, 378));

        // Создание центрального виджета и основного макета
        centralwindow = new QWidget(MainWindow);
        centralwindow->setObjectName("centralwindow");
        centralwindow->setStyleSheet(
                "#centralwindow {"
                "background-color: #978897;"
                "border-radius: 10px;"
                "color: #D8D8F6;"
                "}"

        );

        gridLayout_2 = new QGridLayout(centralwindow);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        gridLayout_2->setSpacing(0);

        // Создание верхней панели с меню и кнопками управления окном
        setupTopBar(MainWindow);


        // Добавление верхней панели в основной макет
        gridLayout_2->addWidget(topBar, 0, 0, 1, 1, Qt::AlignTop);

        // Создание основного макета
        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setVerticalSpacing(0);

        // Настройка левого меню
        setupLeftMenu();

        // Настройка чата
        setupMessage();

        // Настройка редактора
        setupButtonFigures();
        setupButtonTool();
        setupButtonReq();

        // Настройка свернутой панели
        setupCollapsedPanel();

        // Настройка рабочего окна
        setupWorkWindow();

        // Настройка консоли
        setupConsole();

        // Настройка панели настроек
        setupSettingsPanel();

        // Настройка соединений
        setupConnections();

        // Создание правого вертикального макета
        QVBoxLayout *rightLayout = new QVBoxLayout();
        rightLayout->setObjectName("rightLayout");
        rightLayout->setContentsMargins(0, 15, 15, 0);
        rightLayout->setSpacing(5); // Добавляем отступ между console и workWindow

        // Добавляем workWindow в rightLayout
        rightLayout->addWidget(workWindow);

        // Добавляем console в rightLayout под workWindow
        rightLayout->addWidget(console);

        // Устанавливаем коэффициенты растяжения для rightLayout
        rightLayout->setStretch(0, 1); // workWindow растягивается по высоте

        // Создание горизонтального spacer между leftMenu и workWindow
        QSpacerItem *horizontalSpacer = new QSpacerItem(15, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);

        // Добавление виджетов и spacer в основной макет
        gridLayout->addWidget(leftMenuContainer, 0, 0, 1, 1);
        gridLayout->addWidget(messageContainer, 0, 0, 1, 1);
        gridLayout->addWidget(collapsedPanel, 0, 0, 1, 1); // Они находятся в одной позиции, но одна из них скрыта
        gridLayout->addItem(horizontalSpacer, 0, 1, 1, 1); // Spacer во второй колонке
        gridLayout->addLayout(rightLayout, 0, 2, 1, 1);    // Правый макет с рабочим окном и консолью в третьей колонке

        // Установка коэффициентов растяжения
        gridLayout->setColumnStretch(0, 0); // leftMenu не растягивается по ширине
        gridLayout->setColumnStretch(1, 0);
        gridLayout->setColumnStretch(2, 1); // rightLayout растягивается по ширине
        gridLayout->setRowStretch(0, 1);

        gridLayout_2->addLayout(gridLayout, 1, 0, 1, 1);

        MainWindow->setCentralWidget(centralwindow);

        reTranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    }



    void setupWorkWindow() {
        // Создание и настройка рабочего окна
        workWindow = new QFrame(centralwindow);
        workWindow->setObjectName("workWindow");
        workWindow->setFrameShape(QFrame::Shape::NoFrame);
        workWindow->setFrameShadow(QFrame::Shadow::Plain);
        workWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        workWindow->resize(674, 460);
    }

    void setupConsole() {
        // Создание и настройка консоли
        console = new CastomeConsole(centralwindow);
        console->setObjectName("console");
        QFont font2;
        font2.setPointSize(12);
        console->setFont(font2);
        console->setCursor(QCursor(Qt::CursorShape::IBeamCursor));
        console->setAutoFillBackground(false);
        console->setMinimumHeight(30);
        console->setStyleSheet(R"(
        background-color: #3e3d3d;
        color: #D8D8F6;
        border: 1px solid black;
        border-top-left-radius: 10px;
        border-top-right-radius: 10px;
        border-bottom-left-radius: 0px;
        border-bottom-right-radius: 0px;
    )");

        enterConsole = new QPushButton("", console);
        enterConsole->setObjectName("enterConsole");
        enterConsole->setFixedSize(25, 25);
        enterConsole->setStyleSheet("QPushButton#enterConsole {"
                                    "background-color: transparent;"
                                    "color: black;"
                                    "font-size: 4px;"
                                    "border: none;"
                                    "}");
        enterConsole->setToolTip("Message");


        QIcon fileIn("../Static/icons/enter.ico");
        enterConsole->setIcon(fileIn);
        enterConsole->setIconSize(QSize(20, 20));

        layoutConsole = new QHBoxLayout(console);
        layoutConsole->setObjectName("layoutConsole");
        layoutConsole->setContentsMargins(0, 0, 0, 0);
        layoutConsole->setSpacing(5);

        layoutConsole->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

        layoutConsole->addWidget(enterConsole);
        layoutConsole->addSpacerItem(new QSpacerItem(5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));

        enterConsole->setStyleSheet(
                "QPushButton { background: none; border: none; color: #D8D8F6; border-radius: 5px; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 5px; }"
        );
        enterConsole->setCursor(Qt::PointingHandCursor);
        console->setLayout(layoutConsole);
    }



    void setupTopBar(QMainWindow *MainWindow) {
        // Создание верхней панели
        topBar = new QWidget(MainWindow);
        topBar->setObjectName("topBar");
        topBar->setStyleSheet("QWidget#topBar { "
                              "background-color: #494850; "
                              "color: #D8D8F6; "
                              "border-top-left-radius: 10px; "
                              "border-top-right-radius: 10px; "
                              "border-bottom-left-radius: 0px; "
                              "border-bottom-right-radius: 0px; "
                              "border: none; "
                              "border-bottom: 1px solid #262222; }");
        topBar->setFixedHeight(30);

        // Макет для верхней панели
        topBarLayout = new QHBoxLayout(topBar);
        topBarLayout->setObjectName("topBarLayout");
        topBarLayout->setContentsMargins(5, 0, 5, 0);
        topBarLayout->setSpacing(5);

        // Создание меню
        setupMenus(MainWindow);

        // Создание кнопок меню
        setupMenuButtons();

        // Добавление кнопок меню в верхнюю панель
        topBarLayout->addWidget(projectButton);
        topBarLayout->addSpacing(10);
        topBarLayout->addWidget(collaborationButton);
        topBarLayout->addSpacing(10); // Пробел
        topBarLayout->addWidget(helpButton);

        // Кнопки управления окном
        setupWindowControlButtons(MainWindow);

        // Добавление кнопок управления окном в верхнюю панель
        topBarLayout->addStretch();
        topBarLayout->addWidget(settings);
        topBarLayout->addWidget(minimizeButton);
        topBarLayout->addWidget(maximizeButton);
        topBarLayout->addWidget(closeButton);
    }

    void setupActionsSaveLoad(QMainWindow *MainWindow) {
        saveAction = new QWidgetAction(menuProject);
        widgetImport_project = new QWidgetAction(menuProject);
        widgetScript = new QWidgetAction(menuProject);
        widgetPDF = new QWidgetAction(formatMenu);
        widgetSVG = new QWidgetAction(formatMenu);
        widgetOURP = new QWidgetAction(formatMenu);
        widgetTIFF = new QWidgetAction(formatMenu);
        widgetBMP = new QWidgetAction(formatMenu);
        widgetPNG = new QWidgetAction(formatMenu);
        widgetJPG = new QWidgetAction(formatMenu);
        widgetJPEG = new QWidgetAction(formatMenu);

        // Инициализация действий меню
        QFont font;
        font.setPointSize(9);

        QIcon fileIn("../Static/icons/filein.ico");
        QIcon fileOn("../Static/icons/fileon.ico");

        actionSave_project_to = new QToolButton(MainWindow);
        actionSave_project_to->setObjectName("actionSave_project_to");


        saveAction->setDefaultWidget(actionSave_project_to);
        saveAction->setObjectName("saveAction");

        // Создание действий для каждого формата
        actionJPG = new QToolButton(MainWindow);
        actionJPG->setText("JPG");
        actionJPEG = new QToolButton(MainWindow);
        actionJPEG->setText("JPEG");
        actionPNG = new QToolButton(MainWindow);
        actionPNG->setText("PNG");
        actionBMP = new QToolButton(MainWindow);
        actionBMP->setText("BMP");
        actionTIFF = new QToolButton(MainWindow);
        actionTIFF->setText("TIFF");
        actionPDF = new QToolButton(MainWindow);
        actionPDF->setText("PDF");
        actionSVG = new QToolButton(MainWindow);
        actionSVG->setText("SVG");
        actionOURP = new QToolButton(MainWindow);
        actionOURP->setText("OURP");

        // Создание действий для каждого формата
        actionJPG->setObjectName("actionJPG");
        actionJPEG->setObjectName("actionJPEG");
        actionPNG->setObjectName("actionPNG");
        actionBMP->setObjectName("actionBMP");
        actionTIFF->setObjectName("actionTIFF");;
        actionPDF->setObjectName("actionPDF");
        actionSVG->setObjectName("actionSVG");
        actionOURP->setObjectName("actionOURP");


        actionOURP->setStyleSheet(
                "QToolButton {"
                "   background-color: transparent;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px 10px;"
                "   text-align: left;"
                "   border: none;"
                "}"
             /*   "QToolButton:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
                "QToolButton:pressed {"
                "   background-color: rgba(255, 255, 255, 0.1);"
                "}"*/
        );
        actionSVG->setStyleSheet(
                "QToolButton {"
                "   background-color: transparent;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px 10px;"
                "   text-align: left;"
                "   border: none;"
                "}"
          /*      "QToolButton:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
                "QToolButton:pressed {"
                "   background-color: rgba(255, 255, 255, 0.1);"
                "}"*/
        );
        actionPDF->setStyleSheet(
                "QToolButton {"
                "   background-color: transparent;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px 10px;"
                "   text-align: left;"
                "   border: none;"
                "}"
          /*      "QToolButton:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
                "QToolButton:pressed {"
                "   background-color: rgba(255, 255, 255, 0.1);"
                "}"*/
        );
        actionTIFF->setStyleSheet(
                "QToolButton {"
                "   background-color: transparent;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px 10px;"
                "   text-align: left;"
                "   border: none;"
                "}"
          /*      "QToolButton:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
                "QToolButton:pressed {"
                "   background-color: rgba(255, 255, 255, 0.1);"
                "}"*/
        );
        actionBMP->setStyleSheet(
                "QToolButton {"
                "   background-color: transparent;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px 10px;"
                "   text-align: left;"
                "   border: none;"
                "}"
          /*      "QToolButton:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
                "QToolButton:pressed {"
                "   background-color: rgba(255, 255, 255, 0.1);"
                "}"*/
        );
        actionJPG->setStyleSheet(
                "QToolButton {"
                "   background-color: transparent;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px 10px;"
                "   text-align: left;"
                "   border: none;"
                "}"
           /*     "QToolButton:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
                "QToolButton:pressed {"
                "   background-color: rgba(255, 255, 255, 0.1);"
                "}"*/
        );
        actionJPEG->setStyleSheet(
                "QToolButton {"
                "   background-color: transparent;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px 10px;"
                "   text-align: left;"
                "   border: none;"
                "}"
          /*      "QToolButton:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
                "QToolButton:pressed {"
                "   background-color: rgba(255, 255, 255, 0.1);"
                "}"*/
        );
        actionPNG->setStyleSheet(
                "QToolButton {"
                "   background-color: transparent;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px 10px;"
                "   text-align: left;"
                "   border: none;"
                "}"
            //    "QToolButton:hover {"
             //   "   background-color: rgba(255, 255, 255, 0.2);"
               // "   border-radius: 4px;"
               // "}"
               // "QToolButton:pressed {"
               // "   background-color: rgba(255, 255, 255, 0.1);"
               // "}"
        );


        widgetJPG->setDefaultWidget(actionJPG);
        widgetJPG->setObjectName("widgetJPG");

        widgetJPEG->setDefaultWidget(actionJPEG);
        widgetJPEG->setObjectName("widgetJPEG");


        widgetPNG->setDefaultWidget(actionPNG);
        widgetPNG->setObjectName("widgetPNG");


        widgetBMP->setDefaultWidget(actionBMP);
        widgetBMP->setObjectName("widgetBMP");


        widgetTIFF->setDefaultWidget(actionTIFF);
        widgetTIFF->setObjectName("widgetTIFF");


        widgetPDF->setDefaultWidget(actionPDF);
        widgetPDF->setObjectName("widgetPDF");

        widgetSVG->setDefaultWidget(actionSVG);
        widgetSVG->setObjectName("widgetSVG");

        widgetOURP->setDefaultWidget(actionOURP);
        widgetOURP->setObjectName("widgetOURP");

        actionSave_project_to->setIcon(fileIn);
        actionSave_project_to->setFont(font);
        actionSave_project_to->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        actionSave_project_to->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        // Используем стиль, чтобы расположить иконку справа и текст слева
        actionSave_project_to->setStyleSheet(
                "QToolButton#actionSave_project_to {"
                "   background-color: transparent;"  // Прозрачная кнопка
                "   border: none;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px;"
                "   padding-left: 10px;"  // Отступ текста от левого края
                "   text-align: left;"
                "   qproperty-iconSize: 16px;"  // Размер иконки
                "   display: flex;"  // Добавляем flex для управления расположением
                "   justify-content: space-between;"  // Раздвигаем текст и иконку
                "}"
                "QToolButton#actionSave_project_to::icon {"
                "   margin-left: 10px;"  // Отступ между текстом и иконкой
                "   width: 16px; height: 16px;"  // Размер иконки
                "}"
                "QToolButton#actionSave_project_to:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"  // Подсветка при наведении
                "   border-radius: 4px;"
                "}"
        );


        actionImport_project = new QToolButton(MainWindow);

        widgetImport_project->setDefaultWidget(actionImport_project);
        widgetImport_project->setObjectName("widgetImport_project");

        actionImport_project->setFont(font);
        actionImport_project->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        actionImport_project->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        actionImport_project->setObjectName("actionImport_project");
        actionImport_project->setIcon(fileOn);

        actionImport_project->setStyleSheet(
                "QToolButton#actionImport_project {"
                "   background-color: transparent;"  // Прозрачная кнопка
                "   border: none;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px;"
                "   padding-left: 10px;"  // Отступ текста от левого края
                "   text-align: left;"
                "   qproperty-iconSize: 16px;"  // Размер иконки
                "   display: flex;"  // Добавляем flex для управления расположением
                "   justify-content: space-between;"  // Раздвигаем текст и иконку
                "}"
                "QToolButton#actionImport_project::icon {"
                "   margin-left: 10px;"  // Отступ между текстом и иконкой
                "   width: 16px; height: 16px;"  // Размер иконки
                "}"
                "QToolButton#actionImport_project:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"  // Подсветка при наведении
                "   border-radius: 4px;"
                "}"
        );


        actionScript = new QToolButton(MainWindow);
        actionScript->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        actionScript->setToolButtonStyle(Qt::ToolButtonTextOnly);
        actionScript->setLayoutDirection(Qt::LeftToRight);

        widgetScript->setDefaultWidget(actionScript);
        widgetScript->setObjectName("widgetScript");
        actionScript->setObjectName("actionScript");
        actionScript->setStyleSheet(
                "QToolButton#actionScript {"
                "   background-color: transparent;"
                "   border: none;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px;"
                "}"
                "QToolButton#actionScript:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
        );

    }
    void setupServerActions(QMainWindow *MainWindow){
        widgetOpen_server = new QWidgetAction(menuCollaboration);
        widgetJoin_server = new QWidgetAction(menuCollaboration);
        widgetJoin_local_server = new QWidgetAction(menuCollaboration);
        widgetExit_from_session= new QWidgetAction(menuCollaboration);
        actionOpen_server = new QToolButton(MainWindow);
        actionOpen_server->setObjectName("actionOpen_server");
        actionOpen_server->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        actionOpen_server->setToolButtonStyle(Qt::ToolButtonTextOnly);
        actionOpen_server->setLayoutDirection(Qt::LeftToRight);

        widgetOpen_server->setDefaultWidget(actionOpen_server);
        widgetOpen_server->setObjectName("widgetOpen_server");
        actionOpen_server->setStyleSheet(
                "QToolButton#actionOpen_server {"
                "   background-color: transparent;"
                "   border: none;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px;"
                "}"
                "QToolButton#actionOpen_server:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
        );

        actionJoin_server = new QToolButton(MainWindow);
        actionJoin_server->setObjectName("actionJoin_server");

        actionJoin_server->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        actionJoin_server->setToolButtonStyle(Qt::ToolButtonTextOnly);
        actionJoin_server->setLayoutDirection(Qt::LeftToRight);

        widgetJoin_server->setDefaultWidget(actionJoin_server);
        widgetJoin_server->setObjectName("widgetJoin");
        actionJoin_server->setStyleSheet(
                "QToolButton#actionJoin_server {"
                "   background-color: transparent;"
                "   border: none;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px;"
                "}"
                "QToolButton#actionJoin_server:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
        );

        actionJoin_local_server = new QToolButton(MainWindow);
        actionJoin_local_server->setObjectName("actionJoin_local_server");

        actionJoin_local_server->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        actionJoin_local_server->setToolButtonStyle(Qt::ToolButtonTextOnly);
        actionJoin_local_server->setLayoutDirection(Qt::LeftToRight);

        widgetJoin_local_server->setDefaultWidget(actionJoin_local_server);
        widgetJoin_local_server->setObjectName("widgetJoin_local_server");
        actionJoin_local_server->setStyleSheet(
                "QToolButton#actionJoin_local_server {"
                "   background-color: transparent;"
                "   border: none;"
                "   font-size: 9pt;"
                "   color: #D8D8F6;"
                "   padding: 5px;"
                "}"
                "QToolButton#actionJoin_local_server:hover {"
                "   background-color: rgba(255, 255, 255, 0.2);"
                "   border-radius: 4px;"
                "}"
        );

        actionExit_from_session = new QToolButton(MainWindow);
        actionExit_from_session->setObjectName("actionExit_from_session");

        actionExit_from_session->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        actionExit_from_session->setToolButtonStyle(Qt::ToolButtonTextOnly);
        actionExit_from_session->setLayoutDirection(Qt::LeftToRight);

        widgetExit_from_session->setDefaultWidget(actionExit_from_session);
        widgetExit_from_session->setObjectName("widgetExitS");
        actionExit_from_session->setObjectName("actionExit_from_session");
        actionExit_from_session->setStyleSheet(
                "QToolButton#actionExit_from_session {"
                "   background-color: transparent;"
                "   border: none;"
                "   font-size: 9pt;"
                "   color: #8888A0;"
                "   padding: 5px;"
                "}"
        );

        action_help = new QAction(MainWindow);
        action_help->setObjectName("action_help");
    }

    void setupMenus(QMainWindow *MainWindow) {
        QFont font;
        font.setPointSize(9);

        // Меню "Project"
        menuProject = new QMenu(MainWindow);
        menuProject->setObjectName("menuProject");
        menuProject->setStyleSheet(
                "QMenu#menuProject { background-color: #494850; color: #D8D8F6; border: 1px solid #443d3c; border-radius: 5px; }"
                "QMenu#menuProject::indicator { image: none; width: 0px; height: 0px; }"
                "::menu-indicator{ image: none; }"
                "QMenu#formatMenu::item:hover {"
                "   background-color: rgba(255, 255, 255, 0.3); "  // Подсветка при наведении
                "}"
        );

        menuProject->setFont(font);




        // Меню "Format"
        formatMenu = new QMenu(MainWindow);
        formatMenu->setObjectName("formatMenu");
        formatMenu->setStyleSheet(
                "QMenu#formatMenu { background-color: #494850; color: #D8D8F6; border: 1px solid #443d3c; border-radius: 5px; }"
                "QMenu#formatMenu::indicator { image: none; width: 0px; height: 0px; }"
                "::menu-indicator{ image: none; }"
                "QMenu#formatMenu::item:hover {"
                "   background-color: rgba(255, 255, 255, 0.3); "  // Подсветка при наведении
                "}"
        );
        formatMenu->setFont(font);
        setupActionsSaveLoad(MainWindow);


        menuProject->addAction(saveAction);
        menuProject->addAction(widgetImport_project);
        menuProject->addAction(widgetScript);

        formatMenu->addAction(widgetOURP);
        formatMenu->addAction(widgetJPG);
        formatMenu->addAction(widgetJPEG);
        formatMenu->addAction(widgetPNG);
        formatMenu->addAction(widgetBMP);
        formatMenu->addAction(widgetTIFF);
        formatMenu->addAction(widgetPDF);
        formatMenu->addAction(widgetSVG);


        // Меню "Collaboration"
        menuCollaboration = new QMenu(MainWindow);
        menuCollaboration->setObjectName("menuCollaboration");
        menuCollaboration->setStyleSheet(
                "QMenu#menuCollaboration { background-color: #494850; color: #D8D8F6; border: 1px solid #443d3c; border-radius: 5px; }"
                "QMenu#menuCollaboration::indicator { image: none; width: 0px; height: 0px; }"
                "QMenu#formatMenu::item:hover {"
                "   background-color: rgba(255, 255, 255, 0.3); "  // Подсветка при наведении
                "}"
        );
        menuCollaboration->setFont(font);
        setupServerActions(MainWindow);

        menuCollaboration->addAction(widgetOpen_server);
        menuCollaboration->addAction(widgetJoin_server);
        menuCollaboration->addAction(widgetJoin_local_server);
        menuCollaboration->addSeparator();
        menuCollaboration->addAction(widgetExit_from_session);


    }

    void setupMenuButtons() {
        // Создание кнопки "Project"
        projectButton = new QPushButton("Project", topBar);
        projectButton->setObjectName("projectButton");
        projectButton->setToolTip("Project");
        projectButton->setStyleSheet(
                "QPushButton#projectButton { "
                "background-color: #494850; "
                "color: #D8D8F6; "
                "border: none; "
                "border-radius: 5px; "
                "padding: 5px 10px; "
                "}"
                "QPushButton#projectButton::menu-indicator { "
                "image: none; "
                "width: 0px; "
                "}"
                "QPushButton#projectButton:hover { "
                "background-color: rgba(255, 255, 255, 0.3); "
                "}"
        );
        projectButton->setIcon(QIcon("../Static/icons/ChevronDown.ico"));
        projectButton->setLayoutDirection(Qt::RightToLeft);
        projectButton->setMenu(menuProject);

        // Создание кнопки "Collaboration"
        collaborationButton = new QPushButton("Collaboration", topBar);
        collaborationButton->setObjectName("collaborationButton");
        collaborationButton->setToolTip("Collaboration");
        collaborationButton->setStyleSheet(
                "QPushButton#collaborationButton { "
                "background-color: #494850; "
                "color: #D8D8F6; "
                "border: none; "
                "border-radius: 5px; "
                "padding: 5px 10px; "
                "}"
                "QPushButton#collaborationButton::menu-indicator { "
                "image: none; "
                "width: 0px; "
                "}"
                "QPushButton#collaborationButton:hover { "
                "background-color: rgba(255, 255, 255, 0.3); "
                "}"
        );
        collaborationButton->setIcon(QIcon("../Static/icons/ChevronDown.ico"));
        collaborationButton->setLayoutDirection(Qt::RightToLeft);
        collaborationButton->setMenu(menuCollaboration);

        // Создание кнопки "Help"
        helpButton = new QPushButton("Help", topBar);
        helpButton->setObjectName("helpButton");
        helpButton->setToolTip("Help");
        helpButton->setStyleSheet(
                "QPushButton#helpButton { "
                "background-color: #494850; "
                "color: #D8D8F6; "
                "border: none; "
                "border-radius: 5px; "
                "padding: 5px 10px; "
                "}"
                "QPushButton#helpButton:hover { "
                "background-color: rgba(255, 255, 255, 0.3); "
                "}"
        );

        // helpButton->setIcon(QIcon(":/icons/Chevron_down.ico"));
        helpButton->setLayoutDirection(Qt::RightToLeft);
    }

    void setupWindowControlButtons(QMainWindow *MainWindow) {
        // Кнопка "Закрыть"
        closeButton = new QPushButton("", topBar);
        closeButton->setObjectName("closeButton");
        closeButton->setFixedSize(25, 25);
        closeButton->setToolTip("Close");
        closeButton->setIcon(QIcon("../Static/icons/topRight/Close.png")); // Установка иконки
        closeButton->setStyleSheet("QPushButton { background: none; border: none; color: white; border-radius: 5px;}"
                                   "QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }"); // Подсветка при наведении
        QObject::connect(closeButton, &QPushButton::clicked, MainWindow, &QMainWindow::close);

        // Кнопка "Свернуть"
        minimizeButton = new QPushButton("", topBar);
        minimizeButton->setObjectName("minimizeButton");
        minimizeButton->setToolTip("Roll");
        minimizeButton->setFixedSize(25, 25);
        minimizeButton->setIcon(QIcon("../Static/icons/topRight/Minimize.png"));
        minimizeButton->setStyleSheet(
                "QPushButton { background: none; border: none; color: white; border-radius: 5px; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }"); // Подсветка при наведении
        QObject::connect(minimizeButton, &QPushButton::clicked, MainWindow, &QMainWindow::showMinimized);

        // Кнопка "Развернуть"
        maximizeButton = new QPushButton("", topBar);
        maximizeButton->setObjectName("maximizeButton");
        maximizeButton->setFixedSize(25, 25);
        maximizeButton->setToolTip("Unwrap");
        maximizeButton->setIcon(QIcon("../Static/icons/topRight/Maximize.png"));
        maximizeButton->setStyleSheet(
                "QPushButton { background: none; border: none; color: white; border-radius: 5px; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }"); // Подсветка при наведении
        QObject::connect(maximizeButton, &QPushButton::clicked, [=]() {
            if (MainWindow->isMaximized() || MainWindow->isFullScreen()) {
                MainWindow->showNormal(); // Возвращаем к обычному размеру
            } else {
                MainWindow->showMaximized(); // Разворачиваем окно
                MainWindow->update();        // Обновить стиль
            }
        });

        // Кнопка "Настройки"
        settings = new QPushButton("", topBar);
        settings->setObjectName("settings");
        settings->setToolTip("Settings");
        settings->setFixedSize(25, 25);
        settings->setIcon(QIcon("../Static/icons/topRight/Settings.png"));
        settings->setStyleSheet("QPushButton { background: none; border: none; color: white; border-radius: 5px; }"
                                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }"); // Подсветка при наведении
    }

    void setupSettingsPanel() {
        // Создание панели настроек
        settingsPanel = new QWidget(nullptr, Qt::Popup);
        settingsPanel->setObjectName("settingsPanel");
        settingsPanel->setStyleSheet("background-color: #494850; border: 1px solid #443d3c; border-radius: 5px;");

        // Создание макета
        settingsLayout = new QVBoxLayout(settingsPanel);
        settingsLayout->setObjectName("settingsLayout");
        settingsLayout->setContentsMargins(10, 10, 10, 10);
        settingsLayout->setSpacing(10);

        componentGrid = new QCheckBox("Grid", settingsPanel);
        componentGrid->setObjectName("componentGrid");
        componentGrid->setStyleSheet("color: #D8D8F6;background-color: #494850; border: none");
        componentGrid->setChecked(true);
        settingsLayout->addWidget(componentGrid);

        componentAxis = new QCheckBox("Axis", settingsPanel);
        componentAxis->setObjectName("componentAxis");
        componentAxis->setStyleSheet("color: #D8D8F6;background-color: #494850; border: none");
        componentAxis->setChecked(true);
        settingsLayout->addWidget(componentAxis);

        nameLayout = new QHBoxLayout();
        nameLayout->setObjectName("nameLayout");
        nameLabel = new QLabel("Имя:", settingsPanel);
        nameLabel->setObjectName("nameLabel");
        QFont font = nameLabel->font();
        font.setPointSize(9);
        nameLabel->setFont(font);
        nameLabel->setStyleSheet("color: #D8D8F6;border: none");

        nameUsers = new QLineEdit("", settingsPanel);
        nameUsers->setObjectName("nameUsers");
        // nameUsers->setFixedSize(50, 20);
        nameUsers->setStyleSheet("color: #D8D8F6;background-color: #494850; border: 1px solid #000000");

        nameLayout->addWidget(nameLabel);
        nameLayout->addWidget(nameUsers);

        settingsLayout->addLayout(nameLayout);
        settingsPanel->adjustSize();
    }




  void setupLeftMenu() {
      // Создание и настройка QTreeView
      leftMenuView = new QTreeView();
      leftMenuView->setObjectName("leftMenuView");
      leftMenuView->setHeaderHidden(true);
      leftMenuView->setIndentation(16);
      leftMenuView->setStyleSheet(R"(
        QTreeView {
            background-color: #494850;
            color: #D8D8F6;
            border: none;
            border-bottom-left-radius: 10px;
            border-bottom-right-radius: 0px;
        }
    )");
      leftMenuView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      leftMenuView->setSelectionMode(QAbstractItemView::NoSelection);
      // leftMenuView->setFocusPolicy(Qt::NoFocus); // опционально



      // Установка модели

      leftMenuView->expandAll(); // по желанию

      // Кнопка "свернуть"
      collapseButton = new QPushButton("");
      collapseButton->setObjectName("collapseButton");
      collapseButton->setToolTip("Hide");
      collapseButton->setIcon(QIcon("../Static/icons/LeftIco.ico"));
      collapseButton->setFixedSize(30, 30);
      collapseButton->setStyleSheet(
              "QPushButton { background: none; border: none; color: #D8D8F6; }"
              "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); }"
      );

      // Layout
      leftMenuLayout = new QGridLayout();
      leftMenuLayout->setObjectName("leftMenuLayout");
      leftMenuLayout->setContentsMargins(0, 0, 0, 0);
      leftMenuLayout->setSpacing(0);
      leftMenuLayout->addWidget(leftMenuView, 0, 0);
      leftMenuLayout->addWidget(collapseButton, 0, 0, Qt::AlignTop | Qt::AlignRight);

      // Контейнер
      leftMenuContainer = new QWidget();
      leftMenuContainer->setObjectName("leftMenuContainer");
      leftMenuContainer->setLayout(leftMenuLayout);
      leftMenuContainer->setFixedWidth(200);
      leftMenuContainer->hide();
  }


    void setupCollapsedPanel() {
        // Создание свернутой панели
        collapsedPanel = new QWidget(centralwindow);
        collapsedPanel->setFocusPolicy(Qt::NoFocus);
        collapsedPanel->setObjectName("coolapsed");
        collapsedPanel->setFixedWidth(40);
        collapsedPanel->setStyleSheet("background-color: #494850; border-bottom-left-radius: 10px;\n"
                                      "border-bottom-right-radius: 0px;");

        // Создание вертикального макета с выравниванием по верхней части
        collapsedPanelLayout = new QVBoxLayout(collapsedPanel);
        collapsedPanelLayout->setObjectName("collapsedPanelLayout");
        collapsedPanelLayout->setContentsMargins(0, 0, 0, 0); // Убираем отступы
        collapsedPanelLayout->setSpacing(0); // Убираем промежутки между кнопками
        collapsedPanelLayout->setAlignment(Qt::AlignTop); // Выравниваем виджеты по верхней части

        // Создание кнопки для разворачивания меню элементов
        leftMenuElements = new QPushButton("", collapsedPanel);
        leftMenuElements->setObjectName("leftMenuElem");
        leftMenuElements->setToolTip("Information");
        leftMenuElements->setFixedSize(40, 40);
        QIcon IcoElem("../Static/icons/LeftBar/Info.png");
        leftMenuElements->setIcon(IcoElem);
        leftMenuElements->setStyleSheet(
                "QPushButton { background: none; border: none; color: #D8D8F6; border-radius: 0; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 0; }"
        );
        collapsedPanelLayout->addWidget(leftMenuElements);

        // Создание кнопки для открытия чата
        leftMenuMessage = new QPushButton("", collapsedPanel);
        leftMenuMessage->setObjectName("mes");
        leftMenuMessage->setToolTip("Message");
        leftMenuMessage->setFixedSize(40, 40);
        QIcon IcoMes("../Static/icons/leftBar/Message.png");
        leftMenuMessage->setIcon(IcoMes);
        leftMenuMessage->setStyleSheet(
                "QPushButton { background: none; border: none; color: #D8D8F6; border-radius: 0; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 0; }"
        );
        collapsedPanelLayout->addWidget(leftMenuMessage);

        // Создание кнопки для открытия панели фигур
        Figures = new QPushButton("", collapsedPanel);
        Figures->setObjectName("Fig");
        Figures->setFocusPolicy(Qt::NoFocus);
        Figures->setToolTip("Primitives");
        Figures->setFixedSize(40, 40);
        QIcon IcoM("../Static/icons/leftBar/Primitives.png");
        Figures->setIcon(IcoM);
        Figures->setStyleSheet(
                "QPushButton { background: none; border: none; color: #D8D8F6; border-radius: 0; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 0; }"
        );
        collapsedPanelLayout->addWidget(Figures);

        // Создание кнопки для открытия панели инструментов
        Tools = new QPushButton("", collapsedPanel);
        Tools->setObjectName("tool");
        // Tools->setFocusPolicy(Qt::NoFocus);
        Tools->setToolTip("Tools");
        Tools->setFixedSize(40, 40);
        QIcon IcoT("../Static/icons/leftBar/Pointer.png");
        Tools->setIcon(IcoT);
        Tools->setStyleSheet(
                "QPushButton { background: none; border: none; color: #D8D8F6; border-radius: 0; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 0; }"
        );
        collapsedPanelLayout->addWidget(Tools);

        // Создание кнопки для открытия панели требований
        Req = new QPushButton("", collapsedPanel);
        Req->setObjectName("req");
        Req->setToolTip("Requirements");
        Req->setFixedSize(40, 40);
        QIcon IcoR("../Static/icons/leftBar/Req.png");
        Req->setIcon(IcoR);
        Req->setStyleSheet(
                "QPushButton { background: none; border: none; color: #D8D8F6; border-radius: 0; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 0; }"
        );
        collapsedPanelLayout->addWidget(Req);
    }

    void setupMessage() {
        // Создание и настройка message
        message = new QFrame();
        message->setObjectName("message");

        message->setStyleSheet(R"(
        QFrame {
            background-color: #494850;
            color: #D8D8F6;
            border: none;
            border-bottom-left-radius: 10px;
            border-bottom-right-radius: 0px;
        }
    )");

        message->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // Создание консоли для ввода внутри message
        messageConsole = new QLineEdit();
        messageConsole->setObjectName("messageConsole");

        // Установка стилей для поля ввода
        messageConsole->setStyleSheet(R"(
        background-color: #3e3d3d;
        color: #D8D8F6;
        border: 1px solid black;
        border-radius: 5px; /* Закругление углов */
        padding: 5px; /* Отступ внутри поля ввода */
    )");

        enterMes = new QPushButton("", messageConsole);
        enterMes->setToolTip("Message");
        enterMes->setFixedSize(25, 25);
        enterMes->setIcon(QIcon("../Static/icons/enter.ico"));
        enterMes->setIconSize(QSize(20, 20));
        enterMes->setStyleSheet(
                "QPushButton { background: none; border: none; color: #D8D8F6; border-radius: 5px; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 5px; }"
        );
        enterMes->setCursor(Qt::PointingHandCursor);

        smile = new QPushButton("", messageConsole);
        smile->setObjectName("smile");
        smile->setFixedSize(25, 25);
        smile->setIcon(QIcon("../Static/icons/smile.ico"));
        smile->setIconSize(QSize(20, 20));
        smile->setStyleSheet(
                "QPushButton { background: none; border: none; color: #D8D8F6; border-radius: 5px; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 5px; }"
        );
        smile->setCursor(Qt::PointingHandCursor);

        enterMes->setVisible(true);
        smile->setVisible(false);

        layoutMessage = new QHBoxLayout(messageConsole);
        layoutMessage->setObjectName("layoutMessage");
        layoutMessage->setContentsMargins(0, 0, 0, 0);
        layoutMessage->setSpacing(5);
        layoutMessage->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        layoutMessage->addWidget(enterMes);
        layoutMessage->addWidget(smile);
        layoutMessage->addSpacerItem(new QSpacerItem(5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));
        messageConsole->setLayout(layoutMessage);

        emojiWidget = nullptr;

        QFont font2; // Шрифт
        font2.setPointSize(12);
        messageConsole->setFont(font2);
        messageConsole->setCursor(QCursor(Qt::CursorShape::IBeamCursor));
        messageConsole->setMinimumHeight(30);
        messageConsole->setStyleSheet(R"(
        background-color: #3e3d3d;
        color: #D8D8F6;
        border: 1px solid black;
        border-bottom-left-radius: 10px;
        border-bottom-right-radius: 0px;
    )");

        // Создание кнопки свернуть
        messageCollapseButton = new QPushButton("");
        messageCollapseButton->setObjectName("messageCollapseButton");
        messageCollapseButton->setToolTip("Hide");
        messageCollapseButton->setIcon(QIcon("../Static/icons/LeftIco.ico"));
        messageCollapseButton->setFixedSize(30, 30);
        messageCollapseButton->setStyleSheet(
                "QPushButton { background: none; border: none; color: #D8D8F6; }"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); }"
        );

        // Создание макета для message
        messageLayout = new QGridLayout();
        messageLayout->setObjectName("messageLayout");
        messageLayout->setContentsMargins(0, 0, 0, 0);
        messageLayout->setSpacing(0);

        // Установка макета для message
        message->setLayout(messageLayout);

        // Создание контейнера для message
        messageContainer = new QWidget();
        messageContainer->setObjectName("messageContainer");
        messageContainerLayout = new QGridLayout();
        messageContainerLayout->setObjectName("messageContainerLayout");
        messageContainerLayout->setContentsMargins(0, 0, 0, 0);
        messageContainerLayout->setSpacing(0);
        messageContainerLayout->addWidget(message);
        messageContainer->setLayout(messageContainerLayout);
        messageContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        messageContainer->setFixedWidth(200);

        // Для сообщений
        messageContent = new QWidget();
        messageContent->setObjectName("messageContent");
        messageContentLayout = new QVBoxLayout(messageContent);
        messageContentLayout->setObjectName("messageContentLayout");
        messageContentLayout->setContentsMargins(10, 1, 1, 1);
        messageContentLayout->setSpacing(5);
        messageContentLayout->setAlignment(Qt::AlignTop);

        // Прокрутка
        messageScrollArea = new QScrollArea();
        messageScrollArea->setObjectName("messageScrollArea");
        messageScrollArea->setWidgetResizable(true);
        messageScrollArea->setWidget(messageContent);
        messageScrollArea->setStyleSheet("background-color: #494850; border: none;");

        // Добавление элементов в макет message
        messageLayout->addWidget(messageCollapseButton, 0, 0, Qt::AlignTop | Qt::AlignRight);
        messageLayout->addWidget(messageScrollArea, 1, 0);
        messageLayout->addWidget(messageConsole, 2, 0);

        // Изначально скрываем messageContainer
        messageContainer->hide();
    }

    void setupButtonFigures() {
        // Создаем панель для кнопок фигур
        figuresPanel = new QWidget(nullptr, Qt::Popup);
        figuresPanel->setObjectName("figuresPanel");
        figuresPanel->setFixedHeight(40);
        figuresPanel->setStyleSheet("background-color: #494850; border: none;");
        figuresPanel->hide();

        QHBoxLayout *figuresLayout = new QHBoxLayout(figuresPanel);
        figuresLayout->setObjectName("figuresLayout");
        figuresLayout->setContentsMargins(0, 0, 0, 0);
        figuresLayout->setSpacing(0);

        figurePoint = new QPushButton("", figuresPanel);
        figureSection = new QPushButton("", figuresPanel);
        figureCircle = new QPushButton("", figuresPanel);
      //  figureSector = new QPushButton("", figuresPanel);
        figureArc = new QPushButton("", figuresPanel);

        figurePoint->setToolTip("Point");
        figurePoint->setObjectName("Point");

        figureSection->setToolTip("Section");
        figureSection->setObjectName("Section");

        figureCircle->setToolTip("Circle");
        figureCircle->setObjectName("Circle");

       // figureSector->setToolTip("Sector");
        //figureSector->setObjectName("Sector");

        figureArc->setToolTip("Arc");
        figureArc->setObjectName("Arc");

        QIcon IcoCircle("../Static/icons/LeftBar/Primitives/Circle.png");
        QIcon IcoPoint("../Static/icons/LeftBar/Primitives/Point.png");
        QIcon IcoLine("../Static/icons/LeftBar/Primitives/Line.png");
     //   QIcon IcoSector("../Static/icons/LeftBar/Primitives/Line.png");
        QIcon IcoArc("../Static/icons/LeftBar/Primitives/Arc.png");
        //  TODO  Добавить иконки

        figurePoint->setIcon(IcoPoint);
        figureSection->setIcon(IcoLine);
        figureCircle->setIcon(IcoCircle);
     //   figureSector->setIcon(IcoSector);
        figureArc->setIcon(IcoArc);

        figurePoint->setFixedSize(40, 40);
        figureSection->setFixedSize(40, 40);
        figureCircle->setFixedSize(40, 40);
      //  figureSector->setFixedSize(40, 40);
        figureArc->setFixedSize(40, 40);

        QString buttonStyle = "QPushButton { background: none; color: #D8D8F6;border: none; }"
                              "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); }";

        figurePoint->setStyleSheet(buttonStyle);
        figureSection->setStyleSheet(buttonStyle);
        figureCircle->setStyleSheet(buttonStyle);
       // figureSector->setStyleSheet(buttonStyle);
        figureArc->setStyleSheet(buttonStyle);

        figuresLayout->addWidget(figurePoint);
        figuresLayout->addWidget(figureSection);
        figuresLayout->addWidget(figureCircle);
       // figuresLayout->addWidget(figureSector);
        figuresLayout->addWidget(figureArc);

        figuresPanelAnimation = new QPropertyAnimation(figuresPanel, "maximumWidth");
        figuresPanelAnimation->setObjectName("figuresPanelAnimation");
        figuresPanelAnimation->setDuration(200);
        figuresPanelAnimation->setEasingCurve(QEasingCurve::OutQuad);

        figuresPanel->setMaximumWidth(0);
    }

    void setupButtonTool() {
        // Создаем панель для кнопок инструментов
        toolPanel = new QWidget(nullptr, Qt::Popup);
        toolPanel->setObjectName("toolPanel");
        toolPanel->setFixedHeight(40);
        toolPanel->setStyleSheet("background-color: #494850; border: none;");
        toolPanel->hide();

        QHBoxLayout *toolsLayout = new QHBoxLayout(toolPanel);
        toolsLayout->setObjectName("toolsLayout");
        toolsLayout->setContentsMargins(0, 0, 0, 0);
        toolsLayout->setSpacing(0);

        QString buttonStyle = "QPushButton { background: none; color: #D8D8F6;border: none; }"
                              "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); }";

        // Simple
        figureMoving = new QPushButton("", toolPanel);
        figureMoving->setToolTip("Mouse");
        figureMoving->setObjectName("Mouse");
        QIcon IcoMoving("../Static/icons/cursorTool.png");
        figureMoving->setIcon(IcoMoving);
        figureMoving->setFixedSize(40, 40);
        figureMoving->setStyleSheet(buttonStyle);
        toolsLayout->addWidget(figureMoving);

        // MOVING
        toolMoving = new QPushButton("", toolPanel);
        toolMoving->setToolTip("Moving");
        toolMoving->setObjectName("Moving");
        QIcon IcoMove("../Static/icons/MoveTool.png");
        toolMoving->setIcon(IcoMove);
        toolMoving->setFixedSize(40, 40);
        toolMoving->setStyleSheet(buttonStyle);
        toolsLayout->addWidget(toolMoving);

        // ROTATION
       /* toolRotation = new QPushButton("", toolPanel);
        toolRotation->setToolTip("Rotation");
        toolRotation->setObjectName("Rotation");
        QIcon IcoR("../Static/icons/icoRotation.ico");
        toolRotation->setIcon(IcoR);
        toolRotation->setFixedSize(40, 40);
        toolRotation->setStyleSheet(buttonStyle);
        toolsLayout->addWidget(toolRotation);

        // RESIZE
        toolResize = new QPushButton("", toolPanel);
        toolResize->setToolTip("Resize");
        toolResize->setObjectName("Resize");
        QIcon IcoRes("../Static/icons/icoResize.ico");
        toolResize->setIcon(IcoRes);
        toolResize->setFixedSize(40, 40);
        toolResize->setStyleSheet(buttonStyle);
        toolsLayout->addWidget(toolResize);*/

        // TODO add ico!!!
        // SELECTED
        toolSelected = new QPushButton("", toolPanel);
        toolSelected->setToolTip("Selected");
        toolSelected->setObjectName("Resize");
        QIcon IcoSel("../Static/icons/SelectionTool.png");
        toolSelected->setIcon(IcoSel);
        toolSelected->setFixedSize(40, 40);
        toolSelected->setStyleSheet(buttonStyle);
        toolsLayout->addWidget(toolSelected);

        toolsPanelAnimation = new QPropertyAnimation(toolPanel, "maximumWidth");
        toolsPanelAnimation->setObjectName("toolsPanelAnimation");
        toolsPanelAnimation->setDuration(200);
        toolsPanelAnimation->setEasingCurve(QEasingCurve::OutQuad);



        toolPanel->setMaximumWidth(0);
    }

    void setupButtonReq(){
        // Создаем панель для кнопок требований
        reqPanel = new QWidget(nullptr, Qt::Popup);
        reqPanel->setObjectName("reqPanel");
        reqPanel->setFixedHeight(40);
        reqPanel->setStyleSheet("background-color: #494850; border: none;");
        reqPanel->hide();

        QHBoxLayout *reqLayout = new QHBoxLayout(reqPanel);
        reqLayout->setObjectName("reqLayout");
        reqLayout->setContentsMargins(0, 0, 0, 0);
        reqLayout->setSpacing(0);

        QString buttonStyle = "QPushButton { background: none; color: #D8D8F6;border: none; }"
                              "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); }";

        // Создание кнопок требований
        oneReq = new QPushButton("1", reqPanel);
        twoReq = new QPushButton("2", reqPanel);
        threeReq = new QPushButton("3", reqPanel);
        fourReq = new QPushButton("4", reqPanel);
        fiveReq = new QPushButton("5", reqPanel);
        sixReq = new QPushButton("6", reqPanel);
        sevenReq = new QPushButton("7", reqPanel);
        eightReq = new QPushButton("8", reqPanel);
        nineReq = new QPushButton("9", reqPanel);
        tenReq = new QPushButton("10", reqPanel);

        oneReq->setToolTip("Request 1");
        twoReq->setToolTip("Request 2");
        threeReq->setToolTip("Request 3");
        fourReq->setToolTip("Request 4");
        fiveReq->setToolTip("Request 5");
        sixReq->setToolTip("Request 6");
        sevenReq->setToolTip("Request 7");
        eightReq->setToolTip("Request 8");
        nineReq->setToolTip("Request 9");
        tenReq->setToolTip("Request 9");

        oneReq->setObjectName("Request 1");
        twoReq->setObjectName("Request 2");
        threeReq->setObjectName("Request 3");
        fourReq->setObjectName("Request 4");
        fiveReq->setObjectName("Request 5");
        sixReq->setObjectName("Request 6");
        sevenReq->setObjectName("Request 7");
        eightReq->setObjectName("Request 8");
        nineReq->setObjectName("Request 9");
        tenReq->setObjectName("Request 10");

        // Настройка кнопок
        QPushButton* buttons[] = {oneReq, twoReq, threeReq, fourReq, fiveReq, sixReq, sevenReq, eightReq, nineReq,tenReq};
        for (auto *button : buttons) {
            button->setFixedSize(40, 40);
            button->setStyleSheet(buttonStyle);
            reqLayout->addWidget(button);
        }

        reqPanelAnimation = new QPropertyAnimation(reqPanel, "maximumWidth");
        reqPanelAnimation->setObjectName("reqPanelAnimation");
        reqPanelAnimation->setDuration(200);
        reqPanelAnimation->setEasingCurve(QEasingCurve::OutQuad);

        reqPanel->setMaximumWidth(0);
    }


    void setupConnections() {
        QObject::connect(actionSave_project_to, &QPushButton::clicked, [this]() {
            QPoint pos = actionSave_project_to->mapToGlobal(QPoint(actionSave_project_to->width(),0));
            formatMenu->popup(pos);
        });

        QObject::connect(settings, &QPushButton::clicked, [&]() {
            if (settingsPanel->isVisible()) {
                settingsPanel->hide();  // Скрыть панель
            }else {
                QPoint buttonPos = settings->mapToGlobal(QPoint(0, 0));
                QPoint pos = QPoint(buttonPos.x() - settingsPanel->width(), buttonPos.y() + settings->height());
                settingsPanel->move(pos);
                settingsPanel->adjustSize();
                settingsPanel->show();
            }
        });

        QObject::connect(helpButton, &QPushButton::clicked, action_help, &QAction::trigger);


        // Left Menu
        auto *rightClickFilter = new SmileRightClickFilter(messageConsole);
        rightClickFilter->setObjectName("rightClickFilter");
        smile->installEventFilter(rightClickFilter);
        enterMes->installEventFilter(rightClickFilter);

        QObject::connect(rightClickFilter, &SmileRightClickFilter::rightClicked, [=, this](QObject *obj) {
            if (obj == smile) {
                enterMes->setVisible(true);
                smile->setVisible(false);
            } else if (obj == enterMes) {
                enterMes->setVisible(false);
                smile->setVisible(true);
            }
        });

        QObject::connect(smile, &QPushButton::clicked, [this]() {
            if (!emojiWidget) {
                emojiWidget = new EmojiWidget(centralwindow);
                emojiWidget->setObjectName("emojiwidget");
                emojiWidget->hide();
                QObject::connect(emojiWidget, &EmojiWidget::emojiSelected, [this](const QString &emoji) {
                    this->messageConsole->insert(emoji);
                    emojiWidget->hide();
                });
            }

            if (emojiWidget->isVisible()) {
                emojiWidget->hide();
            } else {
                QPoint buttonPos = smile->mapToGlobal(QPoint(0, -2));
                emojiWidget->adjustSize();
                emojiWidget->move(buttonPos.x(), buttonPos.y() - emojiWidget->height());
                emojiWidget->show();
            }
        });

        QObject::connect(collapseButton, &QPushButton::clicked, [&]() {
            leftMenuContainer->hide();
            collapsedPanel->show();
        });

        QObject::connect(leftMenuElements, &QPushButton::clicked, [&]() {
            collapsedPanel->hide();
            leftMenuContainer->show();
            messageContainer->hide();
        });

        QObject::connect(leftMenuMessage, &QPushButton::clicked, [&]() {
            collapsedPanel->hide();
            messageContainer->show();
            leftMenuContainer->hide();
        });

        QObject::connect(messageCollapseButton, &QPushButton::clicked, [&]() {
            messageContainer->hide();
            collapsedPanel->show();
        });



        // Figures
        QObject::connect(Figures, &QPushButton::clicked, [&]() {
            if (figuresPanel->isVisible()) {
                figuresPanelAnimation->stop();
                figuresPanelAnimation->setStartValue(figuresPanel->maximumWidth());
                figuresPanelAnimation->setEndValue(0);
                figuresPanelAnimation->start();
            } else if (toolPanel->isVisible()) {
                toolsPanelAnimation->stop();
                toolsPanelAnimation->setStartValue(toolPanel->maximumWidth());
                toolsPanelAnimation->setEndValue(0);
                toolsPanelAnimation->start();
            }else  if (reqPanel->isVisible()) {
                reqPanelAnimation->stop();
                reqPanelAnimation->setStartValue(reqPanel->maximumWidth());
                reqPanelAnimation->setEndValue(0);
                reqPanelAnimation->start();
            }  else {
                QPoint globalPos = Figures->mapToGlobal(QPoint(0, 0));
                figuresPanel->move(globalPos.x() + Figures->width(), globalPos.y());

                figuresPanel->show();
                figuresPanel->raise(); // Поднимаем панель на передний план

                figuresPanelAnimation->stop();
                figuresPanelAnimation->setStartValue(0);
                figuresPanelAnimation->setEndValue(3 * 40); // 3 кнопки по 40 пикселей
                figuresPanelAnimation->start();
            }
        });

        QObject::connect(figurePoint, &QPushButton::clicked, [&]() {
            if (figuresPanel->isVisible()) {
                figuresPanelAnimation->stop();
                figuresPanelAnimation->setStartValue(figuresPanel->maximumWidth());
                figuresPanelAnimation->setEndValue(0);
                figuresPanelAnimation->start();
            }
        });

        QObject::connect(figureSection, &QPushButton::clicked, [&]() {
            if (figuresPanel->isVisible()) {
                figuresPanelAnimation->stop();
                figuresPanelAnimation->setStartValue(figuresPanel->maximumWidth());
                figuresPanelAnimation->setEndValue(0);
                figuresPanelAnimation->start();
            }
        });

        QObject::connect(figureCircle, &QPushButton::clicked, [&]() {
            if (figuresPanel->isVisible()) {
                figuresPanelAnimation->stop();
                figuresPanelAnimation->setStartValue(figuresPanel->maximumWidth());
                figuresPanelAnimation->setEndValue(0);
                figuresPanelAnimation->start();
            }
        });

     /*   QObject::connect(figureSector, &QPushButton::clicked, [&]() {
            if (figuresPanel->isVisible()) {
                figuresPanelAnimation->stop();
                figuresPanelAnimation->setStartValue(figuresPanel->maximumWidth());
                figuresPanelAnimation->setEndValue(0);
                figuresPanelAnimation->start();
            }
        });*/

        QObject::connect(figureArc, &QPushButton::clicked, [&]() {
            if (figuresPanel->isVisible()) {
                figuresPanelAnimation->stop();
                figuresPanelAnimation->setStartValue(figuresPanel->maximumWidth());
                figuresPanelAnimation->setEndValue(0);
                figuresPanelAnimation->start();
            }
        });

        QObject::connect(figuresPanelAnimation, &QPropertyAnimation::finished, [&]() {
            if (figuresPanel->maximumWidth() == 0) {
                figuresPanel->hide();
            }
        });





        // Tools
        QObject::connect(Tools, &QPushButton::clicked, [&]() {
            if (toolPanel->isVisible()) {
                toolsPanelAnimation->stop();
                toolsPanelAnimation->setStartValue(toolPanel->maximumWidth());
                toolsPanelAnimation->setEndValue(0);
                toolsPanelAnimation->start();
            }else  if (figuresPanel->isVisible()) {
                figuresPanelAnimation->stop();
                figuresPanelAnimation->setStartValue(figuresPanel->maximumWidth());
                figuresPanelAnimation->setEndValue(0);
                figuresPanelAnimation->start();
            } else  if (reqPanel->isVisible()) {
                reqPanelAnimation->stop();
                reqPanelAnimation->setStartValue(reqPanel->maximumWidth());
                reqPanelAnimation->setEndValue(0);
                reqPanelAnimation->start();
            } else {
                QPoint globalPos = Tools->mapToGlobal(QPoint(0, 0));
                toolPanel->move(globalPos.x() + Tools->width(), globalPos.y());

                toolPanel->show();
                toolPanel->raise(); // Поднимаем панель на передний план

                toolsPanelAnimation->stop();
                toolsPanelAnimation->setStartValue(0);
                toolsPanelAnimation->setEndValue(4 * 40); // 4 кнопки по 40 пикселей
                toolsPanelAnimation->start();
            }
        });

        QObject::connect(figureMoving, &QPushButton::clicked, [&]() {
            if (toolPanel->isVisible()) {
                toolsPanelAnimation->stop();
                toolsPanelAnimation->setStartValue(toolPanel->maximumWidth());
                toolsPanelAnimation->setEndValue(0);
                toolsPanelAnimation->start();
            }
        });

        QObject::connect(toolMoving, &QPushButton::clicked, [&]() {
            if (toolPanel->isVisible()) {
                toolsPanelAnimation->stop();
                toolsPanelAnimation->setStartValue(toolPanel->maximumWidth());
                toolsPanelAnimation->setEndValue(0);
                toolsPanelAnimation->start();
            }
        });

/*        QObject::connect(toolRotation, &QPushButton::clicked, [&]() {
            if (toolPanel->isVisible()) {
                toolsPanelAnimation->stop();
                toolsPanelAnimation->setStartValue(toolPanel->maximumWidth());
                toolsPanelAnimation->setEndValue(0);
                toolsPanelAnimation->start();
            }
        });

        QObject::connect(toolResize, &QPushButton::clicked, [&]() {
            if (toolPanel->isVisible()) {
                toolsPanelAnimation->stop();
                toolsPanelAnimation->setStartValue(toolPanel->maximumWidth());
                toolsPanelAnimation->setEndValue(0);
                toolsPanelAnimation->start();
            }
        });*/

        QObject::connect(toolSelected, &QPushButton::clicked, [&]() {
            if (toolPanel->isVisible()) {
                toolsPanelAnimation->stop();
                toolsPanelAnimation->setStartValue(toolPanel->maximumWidth());
                toolsPanelAnimation->setEndValue(0);
                toolsPanelAnimation->start();
            }
        });

        QObject::connect(toolsPanelAnimation, &QPropertyAnimation::finished, [&]() {
            if (toolPanel->maximumWidth() == 0) {
                toolPanel->hide();
            }
        });





        // Requirements
        QObject::connect(Req, &QPushButton::clicked, [&]() {
            if (reqPanel->isVisible()) {
                reqPanelAnimation->stop();
                reqPanelAnimation->setStartValue(reqPanel->maximumWidth());
                reqPanelAnimation->setEndValue(0);
                reqPanelAnimation->start();
            } else  if (toolPanel->isVisible()) {
                toolsPanelAnimation->stop();
                toolsPanelAnimation->setStartValue(toolPanel->maximumWidth());
                toolsPanelAnimation->setEndValue(0);
                toolsPanelAnimation->start();
            }else  if (figuresPanel->isVisible()) {
                figuresPanelAnimation->stop();
                figuresPanelAnimation->setStartValue(figuresPanel->maximumWidth());
                figuresPanelAnimation->setEndValue(0);
                figuresPanelAnimation->start();
            }else {
                QPoint globalPos = Req->mapToGlobal(QPoint(0, 0));
                reqPanel->move(globalPos.x() + Req->width(), globalPos.y());

                reqPanel->show();
                reqPanel->raise(); // Поднимаем панель на передний план

                reqPanelAnimation->stop();
                reqPanelAnimation->setStartValue(0);
                reqPanelAnimation->setEndValue(9 * 40); // 9 кнопок по 40 пикселей
                reqPanelAnimation->start();
            }
        });

        QPushButton* reqButtons[] = {oneReq, twoReq, threeReq, fourReq, fiveReq, sixReq, sevenReq, eightReq, nineReq,tenReq};
        for (QPushButton* button : reqButtons) {
            QObject::connect(button, &QPushButton::clicked, [&]() {
                if (reqPanel->isVisible()) {
                    reqPanelAnimation->stop();
                    reqPanelAnimation->setStartValue(reqPanel->maximumWidth());
                    reqPanelAnimation->setEndValue(0);
                    reqPanelAnimation->start();
                }
            });
        }

        QObject::connect(reqPanelAnimation, &QPropertyAnimation::finished, [&]() {
            if (reqPanel->maximumWidth() == 0) {
                reqPanel->hide();
            }
        });

        QObject::connect(projectButton, &QPushButton::clicked, [&]() {
            if (menuProject->isVisible()) {
                menuProject->close();
               formatMenu->close(); // если открыто
            }
        });

        QObject::connect(collaborationButton, &QPushButton::clicked, [&]() {
            if (menuCollaboration->isVisible()) {
                menuCollaboration->close();
            }
        });


    }

    void reTranslateUi(QMainWindow *MainWindow) const {
// Установка текста для элементов интерфейса
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));

        actionSave_project_to->setText(QCoreApplication::translate("MainWindow", "Save project to...", nullptr));
        actionImport_project->setText(QCoreApplication::translate("MainWindow", "Import project", nullptr));
        actionScript->setText(QCoreApplication::translate("MainWindow", "Script                        ", nullptr));

        actionOpen_server->setText(QCoreApplication::translate("MainWindow", "Open server           ", nullptr));
        actionJoin_server->setText(QCoreApplication::translate("MainWindow", "Join server             ", nullptr));
        actionJoin_local_server->setText(QCoreApplication::translate("MainWindow", "Join local server     ", nullptr));
        actionExit_from_session->setText(QCoreApplication::translate("MainWindow", "Exit from session   ", nullptr));
        action_help->setText(QCoreApplication::translate("MainWindow", "Help", nullptr));

// Очистка консоли
        console->setText(QCoreApplication::translate("MainWindow", "", nullptr));

// Установка текста для кнопок меню
        projectButton->setText(QCoreApplication::translate("MainWindow", "Project", nullptr));
        collaborationButton->setText(QCoreApplication::translate("MainWindow", "Collaboration", nullptr));
        helpButton->setText(QCoreApplication::translate("MainWindow", "Help", nullptr));

// Установка текста для настроек
        componentGrid->setText(QCoreApplication::translate("MainWindow", "Grid", nullptr));
        componentAxis->setText(QCoreApplication::translate("MainWindow", "Axis", nullptr));
    }
};

namespace Ui {
    class MainWindow : public Ui_MainWindow {
    };
}

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H