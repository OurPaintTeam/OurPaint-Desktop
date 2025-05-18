/*
*    Information entry window
*/

#ifndef OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QKeyEvent>
#include <QHBoxLayout>

class InputWindow : public QDialog {
Q_OBJECT
private:
    QLineEdit* lineEdit;
    QPushButton* okButton;
    QPushButton* closeButton;
signals:

    void textEnter(const QString& text);

private slots:

    void OkClicked();

    void CloseClicked();

public:
    InputWindow(const QString& message, QWidget* parent);

    QString getText() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* enter, QEvent* event) override;
};

#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_
