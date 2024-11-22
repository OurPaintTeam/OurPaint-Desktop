/*
 *  Кастомная консоль с реализацией функции автоподсказка
 *
 * */

#ifndef OURPAINT_CASTOMELINEEDIT_H
#define OURPAINT_CASTOMELINEEDIT_H

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



#endif //OURPAINT_CASTOMELINEEDIT_H
