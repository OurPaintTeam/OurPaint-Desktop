#include "Settings.h"

void Settings::saveSettings(bool gridChecked, bool axisChecked, const QString& userName) {
    QDir dir(QFileInfo(SETTINGS_PATH).absolutePath());
    if (!dir.exists()) dir.mkpath(".");

    QSettings settings(SETTINGS_PATH, QSettings::IniFormat);
    settings.setValue("componentGrid", gridChecked);
    settings.setValue("componentAxis", axisChecked);
    settings.setValue("nameUsers", userName);
    settings.sync();
}

void Settings::loadSettings(bool& gridChecked, bool& axisChecked, QString& userName) {
    QSettings settings(SETTINGS_PATH, QSettings::IniFormat);
    gridChecked = settings.value("componentGrid", true).toBool();
    axisChecked = settings.value("componentAxis", true).toBool();
    userName = settings.value("nameUsers", "").toString();
}