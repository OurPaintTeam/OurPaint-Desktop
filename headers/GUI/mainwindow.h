#ifndef OURPAINT_HEADERS_GUI_MAINWINDOW_H_
#define OURPAINT_HEADERS_GUI_MAINWINDOW_H_

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
#include "FrameOverlay.h"


QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

private:
    CastomeWindowError *error;
    CastomeWindowWarning *warning;
    CastomeWindowSuccessful *success;
    Ui::MainWindow *ui;
    std::vector<QString> commands; // Буфер команд для консоли
    const int resizeMargin = 10; // Коэф для рисования закруглений
    int Index; // Индекс для навигации по командам
    bool moving; // Флаг перемещения окна
    bool drawingFrame; // Флаг отображения рамки
    bool save; // Флаг сохранения проекта
    bool resizing; // Флаг изменения размера окна
    bool addElem; // Флаг для изменеия обьектов левого меню
    Qt::Edges resizingEdges; // Грани, которые изменяются при изменении размера
    QPoint lastMousePosition; // Последняя позиция мыши
    QRect frameRect; // Геометрия рамки
    FrameOverlay *frameOverlay; // Объект наложения рамки
    Help *helpWindow; // Окно справки


public:

    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();


    void setAllMouseTracking(QWidget *widget); // Отслеживание мыши

    // Добавление элементов в левое меню
    void Print_LeftMenu(unsigned long long id, const std::string &text, const std::vector<double> &object);

    // Добавление требований в левое меню
    void Requar_LeftMenu(unsigned long long id, const std::string &text, unsigned long long id1, unsigned long long id2,
                         double parametr);

    QWidget *getWorkWindow() const { return ui->workWindow; }

    Ui::MainWindow *getUi() const { return ui; }

    void setSave(bool T) {
        save = T;
    }

    void showHelp();

    void showError(const QString &text);

    void showSuccess(const QString &text);

    void showWarning(const QString &text);

// Добавление сообщений
    void setMessage(const std::string &name, const std::string &message);

    std::tuple<std::vector<std::vector<QString>>, std::vector<std::vector<QString>>, std::vector<bool>, QString>
    saveSettings();

    void loadSettings(std::vector<bool> settings, const QString &name);

    void FocusOnItemById(unsigned long long id);
    QTreeWidgetItem* findItemById(QTreeWidgetItem *item, unsigned long long id);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    QList<QTreeWidgetItem*> getAllChildItems(QTreeWidgetItem *item);
    void processChildItems(const QList<QTreeWidgetItem*> &childItems);

protected:
    // Обработчики событий клавиатуры и мыши
    void keyPressEvent(QKeyEvent *event) override;

    // Закрытие
    void closeEvent(QCloseEvent *event) override;

    // Нажатие мыши
    void mousePressEvent(QMouseEvent *event) override;

    // Перемещение мыши
    void mouseMoveEvent(QMouseEvent *event) override;

    // Отпускание мыши
    void mouseReleaseEvent(QMouseEvent *event) override;

    // Двойное нажатие клавиши
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    // Изменение размеров
    void resizeEvent(QResizeEvent *event) override;

    // Обработка отрисовки окна
    void paintEvent(QPaintEvent *event) override;

    // Кручение колёсиком или тачпадом
    void wheelEvent(QWheelEvent *event) override;

    // Жесты в тачпаде в области workWindow
    bool event(QEvent *event) override;

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

    void ChangeLeftMenu(); // Сигнал изменения параметров левого меню
    void parameterChanged(unsigned long long id, const std::vector<double> &parameters);

    // Кнопки сервера
    void SigOpenServer(const QString &text);

    void SigJoinServer(const QString &text);

    void SigJoinLocalServer(const QString &text);

    void SigExitSession();

    // Чат
    void EnterMessage(const QString &text);

    //Настройки
    void GridOn(bool T);


    void SigMoving();

    void SigSection();

    void SigCircle();

    void SigPoint();

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

public slots:

    // Кнопки
    void saveProjectToFile();

    void loadProjectFile();

    void saveProjectToBMP();

    void loadProjectBMP();

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

    void LeftMenuChanged(QTreeWidgetItem *item);

    void Point() { emit SigPoint(); };
    void Section() { emit SigSection(); };
    void Circle() { emit SigCircle(); };

    void FigMoving() { emit SigMoving(); };
    void ToolMoving() { emit toolMoving(); };
    void ToolRotation() { emit toolRotation(); };
    void ToolResize() { emit toolResize(); };
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


};

#endif // ! OURPAINT_HEADERS_GUI_MAINWINDOW_H_