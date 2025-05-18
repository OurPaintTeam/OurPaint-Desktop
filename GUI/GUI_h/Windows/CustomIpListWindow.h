#ifndef OURPAINT_HEADERS_GUI_WINDOWS_CASTOMEIPLISTWINDOW_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_CASTOMEIPLISTWINDOW_H_

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QIcon>

class CustomIPListWindow : public QWidget {
Q_OBJECT

public:
    explicit CustomIPListWindow(const QStringList& ipAddresses, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

signals:

    void onConnectButtonClicked(const QString& ip);
};

#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_CASTOMEIPLISTWINDOW_H_