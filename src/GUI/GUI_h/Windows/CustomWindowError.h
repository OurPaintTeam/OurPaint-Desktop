#ifndef OURPAINT_HEADERS_GUI_WINDOWS_CATOMEWINDOWERROR_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_CATOMEWINDOWERROR_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QPropertyAnimation>
#include <QTimer>

class CustomWindowError : public QWidget {
Q_OBJECT

public:
    CustomWindowError(const QString &message, QWidget *parent );

public slots:
    void updatePosition(QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
};

#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_CATOMEWINDOWERROR_H_