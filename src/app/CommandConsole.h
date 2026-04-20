#ifndef OURPAINT_COMMANDCONSOLE_H
#define OURPAINT_COMMANDCONSOLE_H



#include <QLineEdit>
#include <QStringList>
#include <QVector>

class QKeyEvent;
class QPaintEvent;


class CommandConsole final : public QLineEdit {
    Q_OBJECT

public:
    explicit CommandConsole(QWidget* parent = nullptr);
    void setCommands(const QStringList& commands);

    signals:
        void sentCommandTriggered(const QString& command);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool focusNextPrevChild(bool next);

private:
    void updateAutocomplete(const QString& text);

    QStringList commands_;
    QString currentCommands_;
    QVector<QString> commandsArray_;
    long long index_ = -1;
};


#endif //OURPAINT_COMMANDCONSOLE_H
