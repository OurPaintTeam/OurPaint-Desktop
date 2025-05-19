#include "CustomConsole.h"

CustomConsole::CustomConsole(QWidget* parent) : QLineEdit(parent) {
    connect(this, &QLineEdit::textChanged, this, &CustomConsole::updateConsole);
}

void CustomConsole::setCommands(const QStringList& commands) {
    _commands = commands;
}

bool CustomConsole::focusNextPrevChild(bool next) {
    return false; // To prevent TAB from switching between widgets
}

void CustomConsole::paintEvent(QPaintEvent* event) {
    QLineEdit::paintEvent(event);

    if (!_currentCommands.isEmpty()) {
        QPainter painter(this);
        const QString Text = text();
        const QFontMetrics metrics(font());
        const int width = metrics.horizontalAdvance(Text);

        const QColor grey(128, 128, 128);
        painter.setPen(grey);
        const int y = (height() + metrics.ascent() - metrics.descent()) / 2;
        constexpr int MARGIN=2;
        constexpr int OFFSET=1;
        painter.drawText(metrics.leftBearing(Text.at(0)) + width + MARGIN, y + OFFSET, _currentCommands.mid(Text.length()));
    }
}

void CustomConsole::keyPressEvent(QKeyEvent* event) {
    setFocus();
    if (event->key() == Qt::Key_Tab && !_currentCommands.isEmpty()) {
        setText(_currentCommands);
        _currentCommands.clear();
        event->accept();
        return;
    }
    QLineEdit::keyPressEvent(event);
}

void CustomConsole::updateConsole(const QString& text) {

    if (text.isEmpty()) {
        _currentCommands.clear();
        update();
        return;
    }

    for (const QString& cmd: _commands) {
        if (cmd.startsWith(text, Qt::CaseInsensitive)) {
            _currentCommands = cmd;
            update();
            return;
        }
    }

    _currentCommands.clear();
    update();
}