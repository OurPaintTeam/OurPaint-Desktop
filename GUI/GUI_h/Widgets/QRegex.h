#ifndef OURPAINT_QREGEX_H
#define OURPAINT_QREGEX_H
#include <QString>
#include <QChar>

// A class for controlling the input of numbers

class QRegex {
public:
    // A method for checking a floating-point number
    static bool parseDoubleNumber(const QString &text);
};

#endif //OURPAINT_QREGEX_H
