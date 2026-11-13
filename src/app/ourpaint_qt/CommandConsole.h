#ifndef OURPAINT_COMMANDCONSOLE_H
#define OURPAINT_COMMANDCONSOLE_H

#include <QLineEdit>
#include <QStringList>
#include <QVector>

class UIController;
class QKeyEvent;
class QPaintEvent;


class CommandConsole final : public QLineEdit {
    Q_OBJECT

public:
    explicit CommandConsole(UIController& controller);
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

};


#endif //OURPAINT_COMMANDCONSOLE_H
