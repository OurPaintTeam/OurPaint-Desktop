#ifndef OURPAINT_HEADERS_SAVELOAD_SAVESETTINGSAPPLICATIONS_H_
#define OURPAINT_HEADERS_SAVELOAD_SAVESETTINGSAPPLICATIONS_H_

#include <vector>
#include <fstream>
#include <string>
#include <QWidget>
#include "CastomeWindowError.h"

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

#endif // ! OURPAINT_HEADERS_SAVELOAD_SAVESETTINGSAPPLICATIONS_H_