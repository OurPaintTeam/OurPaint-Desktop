#ifndef OURPAINT_KEYWORKWINDOW_H
#define OURPAINT_KEYWORKWINDOW_H

#include <QObject>
#include <QWidget>
#include <QKeyEvent>

#include "Modes.h"
#include "Scaling.h"

class KeyWorkWindow : public QObject {
Q_OBJECT
private:
    QWidget* m_parent;

public:
    explicit KeyWorkWindow(QWidget* parent);
    void handleKeyPress(QKeyEvent* event);
    void handleKeyRelease(QKeyEvent* event);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

signals:

    void REDO();
    void UNDO();
    void DELETE();
    void COPY();
    void CUT();
    void PASTE();

    void firstReq();
    void secondReq();
    void thirdReq();
    void fourthReq();
    void fifthReq();
    void sixthReq();
    void seventhReq();
    void eighthReq();
    void ninthReq();
    void tenthReq();
    void eleventhReq();
    void twelfthReq();

};

#endif // OURPAINT_KEYWORKWINDOW_H
