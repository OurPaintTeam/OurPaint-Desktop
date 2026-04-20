#include "CommandConsole.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyle>
#include <QTextLayout>

#include "OverlayModel.h"

CommandConsole::CommandConsole(OverlayModel& overlayModel) : QLineEdit(nullptr), overlayModel_(overlayModel) {
    connect(this, &QLineEdit::textChanged, this, &CommandConsole::updateAutocomplete);
    connect(this, &QLineEdit::textChanged, this, &CommandConsole::onTextChanged);
    const QStringList list = {"LINE", "POINT", "CIRCLE", "ARC", "REQ"};
    setCommands(list);
    for (const auto& cmd : list) {
        commandsArray_.push_back(cmd);
    }
    setObjectName("CustomConsole");
    setFocusPolicy(Qt::StrongFocus);
}

void CommandConsole::setCommands(const QStringList& commands) { commands_ = commands; }

void CommandConsole::paintEvent(QPaintEvent* event) {
    QLineEdit::paintEvent(event);

    if (currentCommands_.isEmpty()) {
        return;
    }

    const auto textContent = text();
    if (textContent.isEmpty()) {
        return;
    }

    const auto suffix = currentCommands_.mid(textContent.length());
    if (suffix.isEmpty()) {
        return;
    }

    QPainter painter(this);
    constexpr auto color = QColor(128, 128, 128);
    painter.setPen(color);

    const QFontMetrics metrics(font());
    const auto cursor = cursorRect();
    const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    const int textMargin = frameWidth + 3;
    const int x = cursor.right() - textMargin;
    const int y = (height() + metrics.ascent() - metrics.descent()) / 2;

    painter.drawText(x, y, suffix);
}

void CommandConsole::keyPressEvent(QKeyEvent* event) {
    setFocus();

    switch (event->key()) {
        case Qt::Key_Tab:
            if (!currentCommands_.isEmpty()) {
                setText(currentCommands_);
                currentCommands_.clear();
            }
            event->accept();
            return;

        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (!text().isEmpty()) {
                commandsArray_.push_back(text());
                index_ = commandsArray_.size();
                // emit sentCommandTriggered(text());
                // clear();
            }
            QLineEdit::keyPressEvent(event);
            return;

        case Qt::Key_Up:
            if (!commandsArray_.isEmpty()) {
                if (index_ <= 0) {
                    index_ = commandsArray_.size() - 1;
                } else {
                    --index_;
                }
                setText(commandsArray_[index_]);
            }
            event->accept();
            return;

        case Qt::Key_Down:
            if (!commandsArray_.isEmpty()) {
                if (index_ >= commandsArray_.size() - 1) {
                    index_ = 0;
                } else {
                    ++index_;
                }
                setText(commandsArray_[index_]);
            }
            event->accept();
            return;

        default:
            break;
    }

    QLineEdit::keyPressEvent(event);
}

bool CommandConsole::focusNextPrevChild(const bool next) {
    if (next) {
        if (!currentCommands_.isEmpty()) {
            setText(currentCommands_);
            currentCommands_.clear();
            return true;
        }
        return QLineEdit::focusNextPrevChild(next);
    }
    return QLineEdit::focusNextPrevChild(next);
}

void CommandConsole::updateAutocomplete(const QString& text) {
    if (text.isEmpty()) {
        currentCommands_.clear();
        update();
        return;
    }

    for (const auto& cmd : commands_) {
        if (cmd.startsWith(text, Qt::CaseInsensitive)) {
            currentCommands_ = cmd;
            update();
            return;
        }
    }

    currentCommands_.clear();
    update();
}

void CommandConsole::onTextChanged(const QString& text) {
    parseReqInput(text);
}


void CommandConsole::parseReqInput(const QString& text) const {
    const auto parts = text.split(' ', Qt::SkipEmptyParts);

    if (parts.size() < 3) {
        overlayModel_.selection_.clear();
        return;
    }

    if (parts[0].compare("REQ", Qt::CaseInsensitive) != 0) {
        return;
    }

    overlayModel_.selection_.clear();

    std::vector<SceneObjects::ID> ids;

    for (int i = 2; i < parts.size(); ++i) {
        bool ok = false;
        const int id = parts[i].toInt(&ok);

        if (ok) {
            ids.push_back(SceneObjects::ID(id));
        }
    }

    overlayModel_.selection_.add(ids);
}