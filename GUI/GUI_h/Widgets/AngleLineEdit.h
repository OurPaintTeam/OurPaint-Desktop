#ifndef ANGLELINEEDIT_H
#define ANGLELINEEDIT_H

#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>

#include "QRegex.h"

// This class is for editing text at a given angle.
// There is a flag for tracking only numbers.

class AngleLineEdit : public QWidget {
Q_OBJECT

private:
    QString text;
    qint32 cursorPos = 0;
    qreal angle = 0.0;
    bool number = false;
    QTimer* cursorTimer = nullptr;
    bool cursorVisible = true;

public:
    explicit AngleLineEdit(QWidget* parent = nullptr);

    void setRotation(const qreal a);
    void setText(const QString& t);
    void setFlagNumber(const bool t);
    QString getText() const;

protected:
    void focusInEvent(QFocusEvent* event) override ;
    void focusOutEvent(QFocusEvent* event) override;
    void paintEvent(QPaintEvent*) override ;
    void keyPressEvent(QKeyEvent* e) override ;


signals:
    void editingFinished();

};



#endif // ANGLELINEEDIT_H
