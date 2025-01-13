#ifndef OURPAINT_HEADERS_SAVELOAD_LOADSETTINGSAPPLICATIONS_H_
#define OURPAINT_HEADERS_SAVELOAD_LOADSETTINGSAPPLICATIONS_H_

#include <vector>
#include <fstream>
#include <string>
#include <QWidget>
#include "CastomeWindowError.h"

class LoadSettingsApplications {
private:
    std::string File;
    QWidget *parent;

public:
    LoadSettingsApplications(const std::string &name, QWidget *parentWidget)
            : File(name + "Set"), parent(parentWidget) {}

    void LoadFigures(std::vector<std::vector<QString>> &figures);
    void LoadRequirements(std::vector<std::vector<QString>> &requirements);
    void LoadSettings(std::vector<bool> &settings,QString &name);
};

#endif // ! OURPAINT_HEADERS_SAVELOAD_LOADSETTINGSAPPLICATIONS_H_