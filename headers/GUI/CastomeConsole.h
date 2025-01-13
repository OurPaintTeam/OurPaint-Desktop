/*
 *  Кастомная консоль с реализацией функции автоподсказка
 *
 * */

#ifndef OURPAINT_HEADERS_GUI_CASTOMELINEEDIT_H_
#define OURPAINT_HEADERS_GUI_CASTOMELINEEDIT_H_

#include <QLineEdit>
#include <QStringList>
#include <QKeyEvent>
#include <QPainter>
#include <QString>

class CastomeConsole : public QLineEdit
{
Q_OBJECT

private slots:
    void updateConsole(const QString &text);

private:
    QStringList _commands;
    QString _currentCommands;

public:
    explicit CastomeConsole(QWidget *parent = nullptr);

    void setCommands(const QStringList &commands);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool focusNextPrevChild(bool next) override;

};



#endif // ! OURPAINT_HEADERS_GUI_CASTOMELINEEDIT_H_
