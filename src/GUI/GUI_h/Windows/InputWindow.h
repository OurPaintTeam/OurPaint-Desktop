/*
*    Information entry window
*/

#ifndef OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>
#include <QKeyEvent>
#include <QEvent>

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
    InputWindow(const QString& message, QWidget* parent = nullptr);

    QString getText() const;

protected:
    void paintEvent(QPaintEvent* event) override;

    bool eventFilter(QObject* enter, QEvent* event) override;
};

#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_
