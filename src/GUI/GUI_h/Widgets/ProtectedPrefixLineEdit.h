#ifndef OURPAINT_PROTECTEDPREFIXLINEEDIT_H
#define OURPAINT_PROTECTEDPREFIXLINEEDIT_H

#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>

#include "QRegex.h"

// A class for managing data in the left menu

class ProtectedPrefixLineEdit : public QLineEdit {
Q_OBJECT

private:
    qint32 prefixLength;
    bool doubleNumber;

public:

    // To enter a new number
    explicit ProtectedPrefixLineEdit(const QString& prefix, QWidget* parent = nullptr);

    // To display when double-tapping
    void setDoubleNumber(bool flag);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

};

#endif //OURPAINT_PROTECTEDPREFIXLINEEDIT_H
