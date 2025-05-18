#ifndef OURPAINT_REGEX_H
#define OURPAINT_REGEX_H
#include <QString>
#include <QChar>
#include <QDebug>

// A class for controlling the input of numbers

class Regex {
public:
    // A method for checking a floating-point number
    static bool parseDoubleNumber(const QString &text);
};
#endif //OURPAINT_REGEX_H
