#ifndef OURPAINT_DREGEX_H
#define OURPAINT_DREGEX_H
#include <QString>
#include <QChar>
#include <QDebug>

// A class for controlling the input of numbers

class DRegex {
public:
    // A method for checking a floating-point number
    static bool parseDoubleNumber(const QString &text);
};

#endif //OURPAINT_DREGEX_H
