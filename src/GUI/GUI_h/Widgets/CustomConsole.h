/*
*   Custom console with auto-tag feature implementation
*
*/

#ifndef OURPAINT_HEADERS_GUI_CASTOMELINEEDIT_H_
#define OURPAINT_HEADERS_GUI_CASTOMELINEEDIT_H_

#include <QLineEdit>
#include <QStringList>
#include <QKeyEvent>
#include <QPainter>
#include <QString>

class CustomConsole : public QLineEdit {
Q_OBJECT

private slots:

    void updateConsole(const QString& text);

private:
    QStringList _commands; // Users commands
    QString _currentCommands; // The basic command

public:
    explicit CustomConsole(QWidget* parent = nullptr);
    void setCommands(const QStringList& commands);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool focusNextPrevChild(bool) override;

};


#endif // ! OURPAINT_HEADERS_GUI_CASTOMELINEEDIT_H_
