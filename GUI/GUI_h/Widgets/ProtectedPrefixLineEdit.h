#ifndef OURPAINT_PROTECTEDPREFIXLINEEDIT_H
#define OURPAINT_PROTECTEDPREFIXLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QMouseEvent>

#include "Regex.h"

// A class for managing data in the left menu

class ProtectedPrefixLineEdit : public QLineEdit {
Q_OBJECT
public:

    // To enter a new number
    explicit ProtectedPrefixLineEdit(const QString& prefix, QWidget* parent = nullptr);

    // To display when double-tapping
    void setDoubleNumber(bool flag);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    int prefixLength;
    bool doubleNumber;
};

#endif //OURPAINT_PROTECTEDPREFIXLINEEDIT_H
