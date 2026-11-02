#ifndef OURPAINT_APPLICATION_QT_MAINWINDOW_BINDER_H_
#define OURPAINT_APPLICATION_QT_MAINWINDOW_BINDER_H_

#include <QObject>
#include <functional>

#include "Lib/Core/ProjectManager.h"
#include "UIController.h"

class QtMainWindowBinder : public QObject {
    Q_OBJECT
public:
    explicit QtMainWindowBinder(UI::ProjectManager& window,
                                UIController& controller,
                                QObject* parent = nullptr);

private:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void invoke(const std::function<void()>& action);
    void installProjectActions();

    UI::ProjectManager& window_;
    UIController& controller_;
};

#endif // ! OURPAINT_APPLICATION_QT_MAINWINDOW_BINDER_H_
