#ifndef OURPAINT_KEYWORKWINDOW_H
#define OURPAINT_KEYWORKWINDOW_H

#include <QObject>
#include <QWidget>
#include <QKeyEvent>
#include <QPushButton>
#include "GUI/control/Modes.h"
#include "GUI/control/scaling.h"

class KeyWorkWindow : public QObject
{
Q_OBJECT
private:
    QWidget *m_parent;

public:
    explicit KeyWorkWindow(QWidget *parent);
    void handleKeyPress(QKeyEvent *event);
    void handleKeyRelease(QKeyEvent *event);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

};

#endif // OURPAINT_KEYWORKWINDOW_H
