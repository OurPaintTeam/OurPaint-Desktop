/*
*
*  Class for creating a frame overlay on a widget when moving or zooming
*      We first move it, then the window takes on the size or position of the frame
*
*
*/


#ifndef OURPAINT_HEADERS_GUI_FRAMEOVERLAY_H_
#define OURPAINT_HEADERS_GUI_FRAMEOVERLAY_H_

#include <QWidget>
#include <QRect>
#include <QColor>
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>


class [[maybe_unused]] FrameOverlay : public QWidget {
Q_OBJECT
public:
    [[maybe_unused]] explicit FrameOverlay(QWidget* parent = nullptr);
    [[maybe_unused]] void setFrameRect(const QRect& rect); // Sets the rectangle of the frame

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    [[maybe_unused]] QRect frameRect;  // Rectangle of the frame
    QColor frameColor;

    void updateFrameColor(); // Updates the frame color based on the background
};

#endif // ! OURPAINT_HEADERS_GUI_FRAMEOVERLAY_H_