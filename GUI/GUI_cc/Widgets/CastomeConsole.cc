#include "CastomeConsole.h"

CastomeConsole::CastomeConsole(QWidget *parent) : QLineEdit(parent) {
    connect(this, &QLineEdit::textChanged,this, &CastomeConsole::updateConsole);
}

void CastomeConsole::setCommands(const QStringList &commands) {
    _commands = commands;
}

bool CastomeConsole::focusNextPrevChild(bool next) {
    (void)next;
    return false; // Для того, чтобы TAB не переключался между виджетами
}

void CastomeConsole::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);

    if (!_currentCommands.isEmpty())
    {
        QPainter painter(this);
        QString Text = text();
        QFontMetrics metrics(font());
        int width = metrics.horizontalAdvance(Text);

        painter.setPen(QColor(128, 128, 128));
        int y = (height() + metrics.ascent() - metrics.descent()) / 2;
        painter.drawText(metrics.leftBearing(Text.at(0)) + width + 2, y +1, _currentCommands.mid(Text.length()));
    }
}

void CastomeConsole::keyPressEvent(QKeyEvent *event){
    setFocus();
    if(event->key() == Qt::Key_Tab && !_currentCommands.isEmpty()){
        setText(_currentCommands);
        _currentCommands.clear();
        event->accept();
        return;
    }
    QLineEdit::keyPressEvent(event);
}

void CastomeConsole::updateConsole(const QString &text){

    if(text.isEmpty()){
        _currentCommands.clear();
        update();
        return;
    }

    for(const QString &cmd : _commands){
        if(cmd.startsWith(text,Qt::CaseInsensitive)){
            _currentCommands=cmd;
            update();
            return;
        }
    }

    _currentCommands.clear();
    update();
};