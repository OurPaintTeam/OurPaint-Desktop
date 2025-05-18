#ifndef OURPAINT_HEADERS_GUI_WINDOWS_CUSTOMWINDOWWARNING_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_CUSTOMWINDOWWARNING_H_

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QPropertyAnimation>
#include <QMessageBox>

class CustomWindowWarning : public QWidget {
Q_OBJECT

public:
    CustomWindowWarning(const QString& message, QWidget* parent);

public slots:

    void updatePosition(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
};

#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_CUSTOMWINDOWWARNING_H_
