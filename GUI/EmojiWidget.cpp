#include "EmojiWidget.h"

EmojiWidget::EmojiWidget(QWidget *parent) : QWidget(parent) {
    gridLayout = new QGridLayout(this);
    setWindowFlags(Qt::FramelessWindowHint);
    gridLayout->setContentsMargins(5, 5, 5, 5);
    gridLayout->setSpacing(5);
//C:\Users\Tim\Downloads\complete_emoji_collection_by_dabolus_d96k3
    emojiList = QStringList() << "😀" << "😂" << "😍" << "😎" << "😭" << "😡" << "👍" << "🙏" << "💪" << "🔥";
    int columns = 5;
    int row = 0;
    int column = 0;
    for (const QString &emoji: emojiList) {
        QPushButton *emojiButton = new QPushButton(emoji);
        emojiButton->setFixedSize(30, 30);
        emojiButton->setStyleSheet("font-size:16px;");
        connect(emojiButton, &QPushButton::clicked, this, &EmojiWidget::onEmojiClicked);
        gridLayout->addWidget(emojiButton, row, column);
        column++;
        if (column >= columns) {
            column = 0;
            row++;
        }
    }
}

void EmojiWidget::onEmojiClicked() {
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        QString emoji = button->text();
        emit emojiSelected(emoji);
    }
}