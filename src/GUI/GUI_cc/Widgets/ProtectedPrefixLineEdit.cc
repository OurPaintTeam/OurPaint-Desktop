#include "ProtectedPrefixLineEdit.h"

ProtectedPrefixLineEdit::ProtectedPrefixLineEdit(const QString& prefix, QWidget* parent)
        : QLineEdit(parent), prefixLength(prefix.length()) {
    setText(prefix);                  // We put a prefix
    setCursorPosition(prefixLength); // Cursor immediately after the prefix
    doubleNumber = false;
}

void ProtectedPrefixLineEdit::setDoubleNumber(bool flag) {
    doubleNumber = flag;
}

void ProtectedPrefixLineEdit::keyPressEvent(QKeyEvent* event) {
    const qint16 pos = cursorPosition();

    // Protection against prefix deletion
    if ((event->key() == Qt::Key_Backspace && pos <= prefixLength) ||
        (event->key() == Qt::Key_Delete && pos < prefixLength && selectedText().isEmpty())) {
        return;
    }

    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_A) {
        // We select only the number (the part after the prefix)
        setSelection(prefixLength, text().length() - prefixLength);
        return;
    }

    // Blocking the selection with a prefix
    if (hasSelectedText()) {
        int selStart = selectionStart();
        if (selStart < prefixLength) {
            return;
        }
    }

    const QString input = event->text();
    if (doubleNumber && !input.isEmpty() && input[0].isPrint()) {
        const QString fullText = this->text();
        const QString numberPart = fullText.mid(prefixLength);
        const QString beforeCursor = numberPart.left(pos - prefixLength);
        const QString afterCursor = numberPart.mid(pos - prefixLength);

        QString simulated = beforeCursor;

        if (hasSelectedText()) {
            const int selStart = selectionStart();
            if (selStart >= prefixLength) {
                const int relative = selStart - prefixLength;
                simulated = numberPart.left(relative); // from start to highlight
            }
        }

        simulated += input;
        simulated += afterCursor;

        if (!DRegex::parseDoubleNumber(simulated)) {
            return;  // we forbid the input
        }
    }

    QLineEdit::keyPressEvent(event);
}

void ProtectedPrefixLineEdit::mousePressEvent(QMouseEvent* event) {
    QLineEdit::mousePressEvent(event);

    // Protecting the cursor from entering the prefix
    if (cursorPosition() < prefixLength) {
        setCursorPosition(prefixLength);
    }
}