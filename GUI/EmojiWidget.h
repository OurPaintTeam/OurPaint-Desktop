#ifndef EMOJIWIDGET_H
#define EMOJIWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QStringList>

class EmojiWidget : public QWidget
{
Q_OBJECT
private slots:
    void onEmojiClicked();

private:
    QGridLayout *gridLayout;
    QStringList emojiList;
public:
    explicit EmojiWidget(QWidget *parent = nullptr);

signals:
    void emojiSelected(const QString &emoji);


};

#endif // EMOJIWIDGET_H