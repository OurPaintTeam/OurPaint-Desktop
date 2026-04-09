#ifndef OURPAINT_APPLICATION_QT_MAINWINDOW_BINDER_H_
#define OURPAINT_APPLICATION_QT_MAINWINDOW_BINDER_H_

#include <QObject>
#include "MainWindow.h"
#include "UIController.h"

class QtMainWindowBinder : public QObject {
    Q_OBJECT
public:
    explicit QtMainWindowBinder(UI::MainWindow& window,
                                UIController& controller,
                                QObject* parent = nullptr);

private:
    UI::MainWindow& window_;
    UIController& controller_;
};

#endif // ! OURPAINT_APPLICATION_QT_MAINWINDOW_BINDER_H_