#ifndef OURPAINT_HEADERS_GUI_WINDOWS_HELP_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_HELP_H_

#include <QMainWindow>
#include <QMouseEvent>

#include "ui_help.h"
class Help : public QMainWindow
{
Q_OBJECT

public:
    explicit Help(QWidget *parent = nullptr);
    ~Help();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void showCommands();
    void showRequirements();
    void showAbout();
    void GuideInf();
    void handleAnchorClicked(const QUrl &url);

private:
    Ui::Help *ui;
    QPoint dragPosition;
};

#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_HELP_H_