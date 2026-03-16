#ifndef OURPAINT_CLOSEDIALOG_H
#define OURPAINT_CLOSEDIALOG_H


/*
*
* Custom window for close the programm
*
*
*/

#include <QDialog>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QLayout>
#include <QLabel>
#include <QWidget>

class CloseDialog : public QDialog {
private:
    bool mousePressed;
    QPoint mousePos;
public:

    CloseDialog(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;

    // Press
    void mousePressEvent(QMouseEvent* event) override;

    // Moving
    void mouseMoveEvent(QMouseEvent* event) override;

    // release
    void mouseReleaseEvent(QMouseEvent* event) override;

};


#endif //OURPAINT_CLOSEDIALOG_H