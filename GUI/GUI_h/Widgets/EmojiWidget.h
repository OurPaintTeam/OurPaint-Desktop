#ifndef OURPAINT_HEADERS_GUI_EMOJIWIDGET_H_
#define OURPAINT_HEADERS_GUI_EMOJIWIDGET_H_

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QPainterPath>
#include <QPainter>

class EmojiWidget : public QWidget {
Q_OBJECT

public:
    explicit EmojiWidget(QWidget *parent = nullptr);

signals:
    void emojiSelected(const QString &emoji);

protected:
  //  void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onEmojiClicked();

private:
    QGridLayout *gridLayout;
    QStringList emojiList;
};

#endif // ! OURPAINT_HEADERS_GUI_EMOJIWIDGET_H_