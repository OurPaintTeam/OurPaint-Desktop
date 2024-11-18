#include "EmojiWidget.h"

EmojiWidget::EmojiWidget(QWidget *parent) : QWidget(parent) {
    gridLayout = new QGridLayout(this);

    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    gridLayout->setContentsMargins(5, 5, 5, 5);
    gridLayout->setSpacing(5);

    emojiList = QStringList() << "😀" << "😂" << "😍" << "😎" << "😭" << "😡" << "👍" << "🙏" << "💪" << "🔥";
    int columns = 5;
    int row = 0;
    int column = 0;

    for (const QString &emoji: emojiList) {
        QPushButton *emojiButton = new QPushButton(emoji);
        emojiButton->setFixedSize(30, 30);
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
            row++;
        }
    }
}

// Переопределяем paintEvent для рисования с гладкими краями
void EmojiWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor backgroundColor(128, 128, 128, 255);
    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);
    painter.fillPath(path, backgroundColor);
}

void EmojiWidget::onEmojiClicked() {
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        QString emoji = button->text();
        emit emojiSelected(emoji);
    }
}