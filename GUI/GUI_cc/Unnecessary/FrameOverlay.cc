#include "FrameOverlay.h"

[[maybe_unused]] FrameOverlay::FrameOverlay(QWidget* parent)
        : QWidget(parent), frameColor(Qt::white) {

    setAttribute(Qt::WA_TransparentForMouseEvents); // Ignore mouse events
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool); // Sets the frame always on top
    setAttribute(Qt::WA_NoSystemBackground);  // Transparent Background
    setAttribute(Qt::WA_TranslucentBackground);
}


// Sets the rectangle of the frame and updates the color
[[maybe_unused]] void FrameOverlay::setFrameRect(const QRect& rect) {
    frameRect = rect; // Saving a new rectangle
    updateFrameColor(); // Updating the frame color
    update();
}


// Updates the color of the frame depending on the background
void FrameOverlay::updateFrameColor() {
    QScreen* screen = QGuiApplication::primaryScreen(); // Fixing the frame area
    QPixmap screenPixmap = screen->grabWindow(0, frameRect.x(), frameRect.y(), frameRect.width(), frameRect.height());
    QImage image = screenPixmap.toImage();

    int totalPixels = image.width() * image.height();

    if (totalPixels == 0) { // Setting the color
        frameColor = Qt::white;
        return;
    }

    qint64 sumR = 0, sumG = 0, sumB = 0;

    // Calculating the color
    int step = qMax(1, qMin(image.width(), image.height()) / 100);

    for (qsizetype y = 0; y < image.height(); y += step) {
        for (qsizetype x = 0; x < image.width(); x += step) {
            QColor color = image.pixelColor(x, y);// Getting the pixel color
            sumR += color.red();
            sumG += color.green();
            sumB += color.blue();
        }
    }

    // Number of pixels
    int samplePixels = ((image.width() + step - 1) / step) * ((image.height() + step - 1) / step);

    // Calculating the average value
    int avgR = sumR / samplePixels;
    int avgG = sumG / samplePixels;
    int avgB = sumB / samplePixels;

    // Brightness calculation
    const qreal backgroundLuminance = (0.2126 * avgR + 0.7152 * avgG + 0.0722 * avgB) / 255.0;

    // Brightness of black and white colors
    constexpr qreal blackLuminance = 0.0;
    constexpr qreal whiteLuminance = 1.0;

    // Contrast calculation
    qreal contrastWithBlack = (backgroundLuminance + 0.05) / (blackLuminance + 0.05);
    qreal contrastWithWhite = (whiteLuminance + 0.05) / (backgroundLuminance + 0.05);

    // Setting the color of the frame depending on the contrast
    if (contrastWithBlack > contrastWithWhite) {
        frameColor = Qt::black;
    } else {
        frameColor = Qt::white;
    }
}


// Handler for the drawing event
void FrameOverlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event) // Ignor

    QPainter painter(this);
    QPen pen(frameColor, 3, Qt::DashLine);
    painter.setPen(pen);

    // Drawing the rectangle of the frame
    painter.drawRect(frameRect.translated(-geometry().topLeft()));
}