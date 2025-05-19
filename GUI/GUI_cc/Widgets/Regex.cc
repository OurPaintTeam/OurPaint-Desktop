#include "Regex.h"

bool Regex::parseDoubleNumber(const QString& text) {
    if (text.isEmpty()) {
        return true; // An empty input is acceptable for now
    }

    QString trimmedText = text.trimmed();

    bool hasDot = false;
    bool hasDigit = false;

    int startIdx = 0;
    if (trimmedText.startsWith('-') || trimmedText.startsWith('+')) {
        startIdx = 1;
    }

    for (qsizetype i = startIdx; i < trimmedText.length(); ++i) {
        QChar c = trimmedText[i];

        if (c.isDigit()) {
            hasDigit = true;
        } else if (c == '.') {
            if (hasDot) {
                return false;
            }
            hasDot = true;
        } else {
            return false; // Invalid character
        }
    }

    // If only "-" or "-." is entered, we will allow it for now (intermediate input)
    if (!hasDigit) {
        if (trimmedText == "-" || trimmedText == "+") return true;
        if (trimmedText == "-." || trimmedText == "+.") return true;
        return false;
    }

    // Checking for a leading 0
    const QStringView view(trimmedText);
    if (view.mid(startIdx).startsWith(u"0") &&
        trimmedText.length() > startIdx + 1 &&
        trimmedText[startIdx + 1] != '.') {
        return false;
    }
    return true;
}
