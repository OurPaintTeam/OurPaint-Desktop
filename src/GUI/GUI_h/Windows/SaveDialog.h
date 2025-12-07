#ifndef OURPAINT_HEADERS_GUI_WINDOWS_SAVEDIALOG_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_SAVEDIALOG_H_

/*
*
* Custom window for saving the window
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

class SaveDialog : public QDialog {
private:
    QString file;
    bool mousePressed;
    QPoint mousePos;
public:

    SaveDialog(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;

    // Press
    void mousePressEvent(QMouseEvent* event) override;

    // Moving
    void mouseMoveEvent(QMouseEvent* event) override;

    // release
    void mouseReleaseEvent(QMouseEvent* event) override;

};


#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_SAVEDIALOG_H_
