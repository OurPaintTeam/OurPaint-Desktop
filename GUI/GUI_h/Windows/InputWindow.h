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

public:
    InputWindow(const QString& message, QWidget* parent = nullptr);
    QString getText() const;
    void setText(const QString& text);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* enter, QEvent* event) override;

signals:
    void textEnter(const QString& text);

private slots:
    void OkClicked();
    void CloseClicked();
};

#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_
