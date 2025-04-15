#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QPainter>
#include <QFrame>
#include <QTreeWidgetItem>
#include <vector>
#include <string>
#include <QWidget>
#include <QFileDialog>
#include <QPushButton>
#include <QDebug>
#include <QMessageBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QGestureEvent>

#include "help.h"
#include "CastomeWindowError.h"
#include "CastomeWindowSuccessful.h"
#include "CastomeWindowWarning.h"
#include "CastomeIpListWindow.h"
#include "LocalScanner.h"
#include "SaveDialog.h"
#include "WindowServer.h"
#include "ui_mainwindow.h"
#include "QTPainter.h"
#include "Modes.h"
#include "MouseEventWorkWindow.h"
#include "KeyWorkWindow.h"
#include "LeftMenuBar.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

private:
    std::unique_ptr<QTPainter> painter;
    std::unique_ptr<MouseWorkWindow> mouseWW;             //  для обработки событий мыши
    std::unique_ptr<KeyWorkWindow> keyWW;                 //  для обработки событий клавиш
    std::unique_ptr<LeftMenuBar> leftMenuBar; // Класс для управления левым меню


    CastomeWindowError *error;
    CastomeWindowWarning *warning;
    CastomeWindowSuccessful *success;
    Ui::MainWindow *ui;
    std::vector<QString> commands; // Буфер команд для консоли
    int Index; // Индекс для навигации по командам
    bool save; // Флаг сохранения проекта

    Help *helpWindow; // Окно справки

    enum ResizeRegion {
        None,
        Top, Bottom, Left, Right,
        TopLeft, TopRight, BottomLeft, BottomRight
    };

    bool resizing = false;
    bool moving = false;
    QPoint dragStartPos;
    QRect originalGeometry;
    ResizeRegion currentRegion = None;
    const int edgeMargin = 8;


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initConnections(); // Инициализация сигналов

    // Добавление сообщений
    void setMessage(const std::string &name, const std::string &message);

    void showHelp(); // Обработка кнопки помощи

    void setAllMouseTracking(QWidget *widget); // Отслеживание мыши

    /***********************/
    // левое меню


    std::tuple<std::vector<std::vector<QString>>, std::vector<std::vector<QString>>, std::vector<bool>, QString>
    saveSettings();

    ///////////////////////////////////////////////////////////////

    void loadSettings(std::vector<bool> settings, const QString &name);

    void showError(const QString &text);

    void showSuccess(const QString &text);

    void showWarning(const QString &text);

    void setSave(bool T) {
        save = T;
    }


    void WorkWindowClear(){
        painter->clear();
    }

    void Draw(){
        painter->draw();
    }

    QTPainter *getQTPainter(){
        return painter.get();
    }

    LeftMenuBar* getLeftMenuBar() const {
        return leftMenuBar.get();
    }

    void updateStyle();
    void updateShapeCursor(const QPoint &pos);

protected:

    // экран

    // Закрытие
    void closeEvent(QCloseEvent *event) override;

    // Изменение размеров
    void resizeEvent(QResizeEvent *event) override;

    // Обработка отрисовки окна
    void paintEvent(QPaintEvent *event) override;
    /////////////////////////////////////////////////

    // Мышь

    // Нажатие мыши
    void mousePressEvent(QMouseEvent *event) override;

    // Перемещение мыши
    void mouseMoveEvent(QMouseEvent *event) override;

    // Отпускание мыши
    void mouseReleaseEvent(QMouseEvent *event) override;

    // Двойное нажатие клавиши
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    // Кручение колёсиком или тачпадом
    void wheelEvent(QWheelEvent *event) override;

    // Жесты в тачпаде в области workWindow
    bool event(QEvent *event) override;
//////////////////////////////////////////////////////////////

// клава
    bool eventFilter(QObject *obj, QEvent *event) override;

    // Обработчики событий клавиатуры и мыши
    void keyPressEvent(QKeyEvent *event) override;



public slots:

    // Кнопки
    void loadProjectFile();
    void saveProjectToBMP();
    void loadProjectBMP();
    void saveProjectToFile();


    void buttonScript();
    void openServer();
    void joinServer();
    void joinLocalServer();


    void Message() {
        QString input = ui->messageConsole->text();
        if (!input.isEmpty()) {
            ui->messageConsole->clear();
            emit EnterMessage(input);
        }
    }

    void exitSession() {
        emit SigExitSession();
    }


    void Point() {
        ModeManager::setActiveMode(WorkModes::Point);
    };
    void Section() {
        ModeManager::setActiveMode(WorkModes::Section);
    };
    void Circle() {
        ModeManager::setActiveMode(WorkModes::Circle);
    };

    void FigMoving() {
        ModeManager::setActiveMode(WorkModes::Editor);
    };
    void ToolMoving() {
        ModeManager::setActiveMode(WorkModes::Move);
    };

    void ToolRotation() {
        ModeManager::setActiveMode(WorkModes::Rotate);
        //  painter->setMode(DrawMode::Rotate);
        // emit toolRotation();
    };
    void ToolResize() {
        ModeManager::setActiveMode(WorkModes::Resize);
        //  painter->setMode(DrawMode::Resize);
        emit toolResize();
    };

    ////////////////////////////////////////////////////
    // Требования

    void firstReq() {
        emit oneReqirements();
    }

    void secondReq() {
        emit twoReqirements();
    }

    void thirdReq() {
        emit threeReqirements();
    }

    void fourthReq() {
        emit fourReqirements();
    }

    void fifthReq() {
        emit fiveReqirements();
    }

    void sixthReq() {
        emit sixReqirements();
    }

    void seventhReq() {
        emit sevenReqirements();
    }

    void eighthReq() {
        emit eightReqirements();
    }

    void ninthReq() {
        emit nineReqirements();
    }

    void tenthReq() {
        emit tenReqirements();
    }

signals:

    void EnterPressed(const QString &command); // Сигнал при нажатии Enter
    void resized(); // Сигнал при изменении размера окна
    void projectSaved(const QString &fileName); // Сигнал о сохранении проекта
    void LoadFile(const QString &fileName); // Сигнал для загрузки файла
    void EmitScript(const QString &fileName);

    void KeyPlus(); // Сигнал увелечения при тачпаде,колёсике и ctrl +
    void KeyMinus(); // Сигнал уменьшения при тачпаде,колёсике и ctrl -
    void KeyZero(); // Обнуление
    void REDO(); // Сигнал для повторения действия
    void UNDO(); // Сигнал для отмены действия



    // Кнопки сервера
    void SigOpenServer(const QString &text);
    void SigJoinServer(const QString &text);
    void SigJoinLocalServer(const QString &text);
    void SigExitSession();

    // Чат
    void EnterMessage(const QString &text);


    void toolMoving();
    void toolRotation();
    void toolResize();

    void NameUsers(const QString &text);
    void changeSettings();

    void loadBMP(const QString &fileName);
    void saveBMP(const QString &fileName);

    void oneReqirements();
    void twoReqirements();
    void threeReqirements();
    void fourReqirements();
    void fiveReqirements();
    void sixReqirements();
    void sevenReqirements();
    void eightReqirements();
    void nineReqirements();
    void tenReqirements();


};

#endif // MAINWINDOW_H