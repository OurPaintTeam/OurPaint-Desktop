#ifndef OURPAINT_COMMANDCONSOLE_H
#define OURPAINT_COMMANDCONSOLE_H

#include <QLineEdit>
#include <QStringList>
#include <QVector>

<<<<<<<< HEAD:src/app/ourpaint_qt/CommandConsole.h
class UIController;
========
#include "Scene.h"

class QtViewportHost;
class OverlayModel;

>>>>>>>> origin/feature/update_UI:src/app/ourpaint_qt/controllers/CommandConsole.h
class QKeyEvent;
class QPaintEvent;


class CommandConsole final : public QLineEdit {
    Q_OBJECT

public:
<<<<<<<< HEAD:src/app/ourpaint_qt/CommandConsole.h
    explicit CommandConsole(UIController& controller);
========
    explicit CommandConsole(OverlayModel& overlayModel_, QtViewportHost& host);
>>>>>>>> origin/feature/update_UI:src/app/ourpaint_qt/controllers/CommandConsole.h
    void setCommands(const QStringList& commands);

    signals:
        void sentCommandTriggered(const QString& command);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool focusNextPrevChild(bool next) override;

private:
    void updateAutocomplete(const QString& text);
    void onTextChanged(const QString& text);
    void parseReqInput(const QString& text) const;

    UIController& controller_;
    QStringList commands_;
    QString currentCommands_;
    QVector<QString> commandsArray_;
    long long index_ = -1;

<<<<<<<< HEAD:src/app/ourpaint_qt/CommandConsole.h
========
    QtViewportHost& host_;
>>>>>>>> origin/feature/update_UI:src/app/ourpaint_qt/controllers/CommandConsole.h
};


#endif //OURPAINT_COMMANDCONSOLE_H
