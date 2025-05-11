#ifndef OURPAINT_HEADERS_GUI_WINDOWS_UIHELP_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_UIHELP_H_

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QSizePolicy>
#include <QDesktopServices>
#include <QScrollArea>
#include <QSizePolicy>
#include <QDesktopServices>
#include <QTextBrowser>

QT_BEGIN_NAMESPACE

class Ui_Help
{
public:
    // Main widgets
    QWidget *centralwidget;
    QGridLayout *mainLayout;
    QFrame *topBar;
    QHBoxLayout *topBarLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;

    // Left menu
    QWidget *leftMenu;
    QVBoxLayout *leftMenuLayout;
    QPushButton *commandsButton;
    QPushButton *requirementsButton;
    QPushButton *aboutButton;
    QPushButton *guide;
    QPushButton *supportButton;

    // Content area
    QFrame *contentFrame;
    QVBoxLayout *contentLayout;
    QTextBrowser *helpContentBrowser;

    void setupUi(QMainWindow *HelpWindow)
    {
        HelpWindow->setObjectName("HelpWindow");
        HelpWindow->resize(600, 400);
        HelpWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
        HelpWindow->setAttribute(Qt::WA_TranslucentBackground);
        HelpWindow->setStyleSheet("QWidget { color: #D8D8F6; }");

        centralwidget = new QWidget(HelpWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setStyleSheet("QWidget#centralwidget { "
                                     "background-color: #494850;"
                                     "border-radius: 10px; }");
        mainLayout = new QGridLayout(centralwidget);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        setupTopBar(HelpWindow);

        mainLayout->addWidget(topBar, 0, 0, 1, 2);

        setupLeftMenu();

        mainLayout->addWidget(leftMenu, 1, 0, 1, 1);

        setupContent();

        mainLayout->addWidget(contentFrame, 1, 1, 1, 1);

        // Adjust spacings to zero
        mainLayout->setHorizontalSpacing(0);
        mainLayout->setVerticalSpacing(0);

        mainLayout->setColumnStretch(0, 0);
        mainLayout->setColumnStretch(1, 1);
        mainLayout->setRowStretch(1, 1);

        HelpWindow->setCentralWidget(centralwidget);

        retranslateUi(HelpWindow);

        QMetaObject::connectSlotsByName(HelpWindow);
    }

    void setupTopBar(QMainWindow *HelpWindow)
    {
        topBar = new QFrame(centralwidget);
        topBar->setObjectName("topBar");
        topBar->setStyleSheet("QFrame#topBar { "
                              "background-color: #494850; "
                              "color: #D8D8F6; "
                              "border-top-left-radius: 10px; "
                              "border-top-right-radius: 10px; "
                              "border-bottom: 1px solid #262222; }");
        topBar->setFixedHeight(30);

        topBarLayout = new QHBoxLayout(topBar);
        topBarLayout->setContentsMargins(5, 0, 5, 0);
        topBarLayout->setSpacing(0);
        titleLabel = new QLabel("Справка", topBar);
        titleLabel->setStyleSheet("QLabel { color: #D8D8F6; font-size: 14px; }");

        setupWindowControlButtons(HelpWindow);

        topBarLayout->addWidget(titleLabel);
        topBarLayout->addStretch();
        topBarLayout->addWidget(closeButton);
    }

    void setupWindowControlButtons(QMainWindow *HelpWindow)
    {
        closeButton = new QPushButton("", topBar);
        closeButton->setFixedSize(25, 25);
        closeButton->setIcon(QIcon("../Static/icons/topRight/Close.png"));
        closeButton->setStyleSheet("QPushButton { background: none; border: none; color: white; border-radius: 5px; }"
                                   "QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }");
        QObject::connect(closeButton, &QPushButton::clicked, HelpWindow, &QMainWindow::close);
    }

    void setupLeftMenu()
    {
        leftMenu = new QWidget(centralwidget);
        leftMenu->setObjectName("leftMenu");
        leftMenu->setStyleSheet("QWidget#leftMenu { "
                                "background-color: #494850; "
                                "color: #D8D8F6; "
                                "border: none;"
                                "border-bottom-left-radius: 10px; "
                                "border-top-left-radius: 0px; "
                                "border-top-right-radius: 0px; "
                                "border-bottom-right-radius: 0px; }");
        leftMenu->setMinimumWidth(150);

        leftMenuLayout = new QVBoxLayout(leftMenu);
        leftMenuLayout->setContentsMargins(0, 0, 0, 0);
        leftMenuLayout->setSpacing(0);

        QFont buttonFont;
        buttonFont.setPointSize(10);

        commandsButton = new QPushButton("Команды", leftMenu);
        commandsButton->setObjectName("commandsButton");
        commandsButton->setFont(buttonFont);
        commandsButton->setStyleSheet(buttonStyle());
        leftMenuLayout->addWidget(commandsButton);

        requirementsButton = new QPushButton("Требования", leftMenu);
        requirementsButton->setObjectName("requirementsButton");
        requirementsButton->setFont(buttonFont);
        requirementsButton->setStyleSheet(buttonStyle());
        leftMenuLayout->addWidget(requirementsButton);

        aboutButton = new QPushButton("О программе", leftMenu);
        aboutButton->setObjectName("aboutButton");
        aboutButton->setFont(buttonFont);
        aboutButton->setStyleSheet(buttonStyle());
        leftMenuLayout->addWidget(aboutButton);

        guide = new QPushButton("Руководство", leftMenu);
        guide->setObjectName("guideButton");
        guide->setFont(buttonFont);
        guide->setStyleSheet(buttonStyle());
        leftMenuLayout->addWidget(guide);

        supportButton = new QPushButton("Поддержка", leftMenu);
        supportButton->setObjectName("supportButton");
        supportButton->setFont(buttonFont);
        supportButton->setStyleSheet(buttonStyle());
        leftMenuLayout->addWidget(supportButton);

        leftMenuLayout->addStretch();

        QObject::connect(supportButton, &QPushButton::clicked, []() {
            QDesktopServices::openUrl(QUrl("https://t.me/+Du5DMLT9QukxZjky"));
        });
    }

    QString buttonStyle()
    {
        return QString(
                "QPushButton { "
                "background-color: #494850; "
                "color: #D8D8F6; "
                "border: none; "
                "padding: 10px; "
                "text-align: left; "
                "}"
                "QPushButton:hover { "
                "background-color: rgba(255, 255, 255, 0.1); "
                "}"
                "QPushButton:checked { "
                "background-color: rgba(255, 255, 255, 0.2); "
                "}"
        );
    }

    void setupContent()
    {
        contentFrame = new QFrame(centralwidget);
        contentFrame->setObjectName("contentFrame");
        contentFrame->setStyleSheet("QFrame#contentFrame { "
                                    "background-color: #3e3d3d; "
                                    "color: #D8D8F6; "
                                    "border-bottom-right-radius: 10px; "
                                    "border-top-right-radius: 0px; }");

        contentLayout = new QVBoxLayout(contentFrame);
        contentLayout->setContentsMargins(15, 15, 15, 15);
        contentLayout->setSpacing(0);

        helpContentBrowser = new QTextBrowser(contentFrame);
        helpContentBrowser->setObjectName("helpContentBrowser");
        helpContentBrowser->setStyleSheet("QTextBrowser { "
                                          "background-color: #3e3d3d; "
                                          "color: #D8D8F6; "
                                          "font-size: 12px; "
                                          "border: none; }");
        helpContentBrowser->setReadOnly(true);
        helpContentBrowser->setOpenExternalLinks(false); // Чтобы ссылки обрабатывались внутри приложения

        contentLayout->addWidget(helpContentBrowser);
    }


    void retranslateUi(QMainWindow *HelpWindow)
    {
        HelpWindow->setWindowTitle(QCoreApplication::translate("HelpWindow", "Помощь", nullptr));
        titleLabel->setText(QCoreApplication::translate("HelpWindow", "Помощь", nullptr));

        commandsButton->setText(QCoreApplication::translate("HelpWindow", "Команды", nullptr));
        requirementsButton->setText(QCoreApplication::translate("HelpWindow", "Требования", nullptr));
        aboutButton->setText(QCoreApplication::translate("HelpWindow", "О программе", nullptr));
        guide->setText(QCoreApplication::translate("HelpWindow", "Руководство", nullptr));
        supportButton->setText(QCoreApplication::translate("HelpWindow", "Поддержка", nullptr));
    }
};

namespace Ui {
    class Help: public Ui_Help {};
} // namespace Ui

QT_END_NAMESPACE

#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_UIHELP_H_