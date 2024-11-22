#include "LoadSettingsApplications.h"

void LoadSettingsApplications::LoadFigures(std::vector<std::vector<QString>> &figures) {
    std::ifstream inFile(File);
    if (!inFile) {
        new CastomeWindowError("Произошла ошибка открытия файла!", parent);
        return;
    }

    std::string line;
    bool inFiguresSection = false;

    while (std::getline(inFile, line)) {
        if (line == "Figure:") {
            inFiguresSection = true;
            std::getline(inFile, line); // Считываем {
            continue;
        }

        if (inFiguresSection) {
            // Конец фигур
            if (line == "}") {
                break;
            }

            std::string figureName = line;
            std::getline(inFile, line); // Считываем {

            if (line != "{") continue;

            std::vector<QString> figure;
            figure.push_back(QString::fromStdString(figureName));

            while (std::getline(inFile, line)) {
                if (line == "}") break;

                size_t pos = line.find(": ");
                if (pos != std::string::npos) {
                    line = line.substr(pos + 2);
                }

                figure.push_back(QString::fromStdString(line));
            }


            figures.push_back(figure);
        }
    }

    inFile.close();
}

void LoadSettingsApplications::LoadRequirements(std::vector<std::vector<QString>> &requirements) {
    std::ifstream inFile(File);
    if (!inFile) {
        new CastomeWindowError("Произошла ошибка открытия файла!", parent);
        return;
    }

    std::string line;
    bool inRequirementsSection = false;

    while (std::getline(inFile, line)) {
        if (line == "Requirements:") {
            inRequirementsSection = true;
            std::getline(inFile, line); // Считываем {
            continue;
        }

        if (inRequirementsSection) {
            // Конец  требований
            if (line == "}") {
                break;
            }

            std::string reqName = line;
            std::getline(inFile, line); // Считываем {

            if (line != "{") continue;

            std::vector<QString> req;
            req.push_back(QString::fromStdString(reqName));

            while (std::getline(inFile, line)) {
                if (line == "}") break;

                size_t pos = line.find(": ");
                if (pos != std::string::npos) {
                    line = line.substr(pos + 2);
                }


                req.push_back(QString::fromStdString(line));
            }

            requirements.push_back(req);
        }
    }

    inFile.close();
}

void LoadSettingsApplications::LoadSettings(std::vector<bool> &settings,QString &name) {
    std::ifstream inFile(File);
    if (!inFile) {
        new CastomeWindowError("Произошла ошибка открытия файла!", parent);
        return;
    }

    std::string line;
    bool inSettingsSection = false;

    while (std::getline(inFile, line)) {
        if (line == "Settings:") {
            inSettingsSection = true;
            std::getline(inFile, line); // Считываем {
            continue;
        }

        if (inSettingsSection) {
            // Конец  настроек
            if (line == "}") {
                break;
            }

            if (line.find("Grid:") != std::string::npos) {
                std::string value = line.substr(line.find(":") + 2);
                bool gridValue = (value == "1");
                settings.push_back(gridValue);
            }
            if (line.find("Name:") != std::string::npos) {
                // Находим позицию двоеточия
                size_t colonPos = line.find(":");
                std::string value = line.substr(colonPos + 2);
                name = QString::fromStdString(value);
            }
        }
    }

    inFile.close();
}