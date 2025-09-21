#include "AngleLineEdit.h"


AngleLineEdit::AngleLineEdit(QWidget* parent)
        : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);

    cursorTimer = new QTimer(this);
    cursorTimer->setInterval(500);
    connect(cursorTimer, &QTimer::timeout, this, [this]() {
        cursorVisible = !cursorVisible;
        update();
    });
}


void AngleLineEdit::setRotation(const qreal a) {
    angle = a;
    update();
}


void AngleLineEdit::setText(const QString& t) {
    text = t;
    cursorPos = text.length();
    update();
}


void AngleLineEdit::setFlagNumber(const bool t) {
    number = t;
}


QString AngleLineEdit::getText() const {
    return text;
}


void AngleLineEdit::focusInEvent(QFocusEvent* event) {
    cursorVisible = true;
    cursorTimer->start();
    QWidget::focusInEvent(event);
}


void AngleLineEdit::focusOutEvent(QFocusEvent* event) {
    cursorVisible = false;
    cursorTimer->stop();
    update();
    QWidget::focusOutEvent(event);
}


void AngleLineEdit::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.translate(width() / 2, height() - 10);
    p.rotate(angle);
    p.translate(-width() / 2, -height() / 2);

    QFontMetrics fm(font());

    qint32 textOffsetX = 0;
    qint32 textOffsetY = height() / 2 + fm.ascent() / 2;

    if (number) {
        if (!DRegex::parseDoubleNumber(text)) {
            text.remove(--cursorPos, 1);
        }
    }

    p.setPen(Qt::black);
    p.drawText(textOffsetX, textOffsetY, text);

    if (hasFocus() && cursorVisible) {
        qint32 cursorX = fm.horizontalAdvance(text.left(cursorPos));
        p.drawLine(cursorX + textOffsetX, 2,
                   cursorX + textOffsetX, height() - 2);
    }
}

void AngleLineEdit::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        emit editingFinished();
    } else if (e->key() == Qt::Key_Backspace && cursorPos > 0) {
        text.remove(cursorPos - 1, 1);
        cursorPos--;
    } else if (e->key() == Qt::Key_Left && cursorPos > 0) {
        cursorPos--;
    } else if (e->key() == Qt::Key_Right && cursorPos < text.length()) {
        cursorPos++;
    } else {
        QChar c = e->text().isEmpty() ? QChar() : e->text().at(0);
        if (!c.isNull() && c.isPrint()) {
            text.insert(cursorPos, c);
            cursorPos++;
        }
    }
    update();
}