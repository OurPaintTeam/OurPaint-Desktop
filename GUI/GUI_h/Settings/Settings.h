// settings.h
#ifndef OURPAINT_SAVESETTINGS_H
#define OURPAINT_SAVESETTINGS_H

#include <QSettings>
#include <QDir>
#include <QFileInfo>

class Settings {
private:
    QString SETTINGS_PATH;
public:
    explicit Settings(const QString& path) : SETTINGS_PATH(path) {}
    void saveSettings(bool gridChecked, bool axisChecked, const QString& userName);
    void loadSettings(bool& gridChecked, bool& axisChecked, QString& userName);
};

#endif // OURPAINT_SAVESETTINGS_H
