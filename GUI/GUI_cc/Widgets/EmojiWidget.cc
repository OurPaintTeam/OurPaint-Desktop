#include "EmojiWidget.h"

EmojiWidget::EmojiWidget(QWidget* parent) : QWidget(parent) {
    gridLayout = new QGridLayout(this);

    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    constexpr quint16 MARGINS = 5;
    gridLayout->setContentsMargins(MARGINS, MARGINS, MARGINS, MARGINS);
    gridLayout->setSpacing(MARGINS);

    emojiList = QStringList() << "😀" << "😂" << "😍" << "😎" << "😭" << "😡" << "👍" << "🙏" << "💪" << "🔥";
    constexpr quint16 columns = 5;

    qint16 row = 0;
    qint16 column = 0;

    for (const QString& emoji: emojiList) {
        QPushButton* emojiButton = new QPushButton(emoji);

        constexpr quint16 SIZE = 30;
        emojiButton->setFixedSize(SIZE, SIZE);

        emojiButton->setStyleSheet(
                "QPushButton {"
                "   font-size: 16px;"
                "   background-color: transparent;"
                "   border: none;"
                "}"
                "QPushButton:hover {"
                "   background-color: #4DFFFFFF;"
                "   border-radius: 10px;"
                "}"
        );
        connect(emojiButton, &QPushButton::clicked, this, &EmojiWidget::onEmojiClicked);
        gridLayout->addWidget(emojiButton, row, column);

        column++;

        if (column >= columns) {
            column = 0;
            ++row;
        }

    }
}

// Redefine paintEvent for drawing with smooth edges
void EmojiWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    constexpr QColor backgroundColor(128, 128, 128, 255);
    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);
    painter.fillPath(path, backgroundColor);
}

void EmojiWidget::onEmojiClicked() {
    const QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString emoji = button->text();
        emit emojiSelected(emoji);
    }
}