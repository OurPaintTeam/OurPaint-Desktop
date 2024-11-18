#ifndef OURPAINT_SAVESETTINGSAPPLICATIONS_H
#define OURPAINT_SAVESETTINGSAPPLICATIONS_H

#include <vector>
#include <fstream>
#include <string>
#include <QWidget>
#include "../GUI/Windows/CastomeWindowError.h"

class SaveSettingsApplications {
private:
    std::string File;
    QWidget *parent;

public:
    SaveSettingsApplications(const std::string &name, QWidget *parentWidget)
            : File(name+"Set"), parent(parentWidget) {}

    void SaveFigures(const std::vector<std::vector<QString>> &figures);

    void SaveRequirements(const std::vector<std::vector<QString>> &requirements) ;

    void SaveSettings(const std::vector<bool> &settings,const QString &name) ;
    void clear();
};

#endif //OURPAINT_SAVESETTINGSAPPLICATIONS_H