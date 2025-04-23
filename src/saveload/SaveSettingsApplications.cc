#include "SaveSettingsApplications.h"

void SaveSettingsApplications::clear() {
    std::ofstream outFile(File, std::ios::trunc);
    if (!outFile) {
        CastomeWindowError *errorWindow = new CastomeWindowError("Произошла ошибка открытия файла!", parent);
        return;
    }
    outFile.close();
}

void SaveSettingsApplications::SaveFigures(const std::vector<std::vector<QString>> &figures) {
    std::ofstream outFile(File, std::ios::app);
    if (!outFile) {
        CastomeWindowError *errorWindow = new CastomeWindowError("Произошла ошибка открытия файла!", parent);
        return;
    }

    outFile << "Figure:" << std::endl << "{" << std::endl;

    for (const auto &figure: figures) {
        if (figure.empty()) continue;

        QString name = figure[0];
        outFile << name.toStdString() << std::endl << '{' << std::endl;

        for (size_t i = 1; i < figure.size(); ++i) {
            outFile << figure[i].toStdString() << std::endl;
        }

        outFile << '}' << std::endl;
    }

    outFile << '}' << std::endl;
    outFile.close();
}

void SaveSettingsApplications::SaveRequirements(const std::vector<std::vector<QString>> &requirements) {
    std::ofstream outFile(File, std::ios::app);
    if (!outFile) {
        CastomeWindowError *errorWindow = new CastomeWindowError("Произошла ошибка открытия файла!", parent);
        return;
    }

    outFile << "Requirements:" << std::endl << '{' << std::endl;

    for (const auto &req: requirements) {
        if (req.empty()) continue;

        QString name = req[0];
        outFile << name.toStdString() << std::endl << '{' << std::endl;

        for (size_t i = 1; i < req.size(); ++i) {
            outFile << req[i].toStdString() << std::endl;
        }

        outFile << '}' << std::endl;
    }

    outFile << '}' << std::endl;
    outFile.close();
}

void SaveSettingsApplications::SaveSettings(const std::vector<bool> &settings, const QString &name) {
    std::ofstream outFile(File, std::ios::app);
    if (!outFile) {
        CastomeWindowError *errorWindow = new CastomeWindowError("Произошла ошибка открытия файла!", parent);
        return;
    }

    outFile << "Settings:" << std::endl << '{' << std::endl;

    outFile << "Grid: " << (settings[0] ? "1" : "0") << std::endl;
    outFile << "Name: " << name.toStdString() << std::endl;

    outFile << '}' << std::endl;
    outFile.close();
}

