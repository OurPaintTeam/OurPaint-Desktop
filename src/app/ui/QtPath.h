#ifndef OURPAINT_APPLICATION_QT_PATH_H_
#define OURPAINT_APPLICATION_QT_PATH_H_

#include <QString>
#include <filesystem>

namespace app::ui {

inline std::filesystem::path nativePath(const QString& text) {
#ifdef _WIN32
    return std::filesystem::path(text.toStdWString());
#else
    return std::filesystem::path(text.toStdString());
#endif
}

inline QString pathText(const std::filesystem::path& path) {
#ifdef _WIN32
    return QString::fromStdWString(path.native());
#else
    return QString::fromUtf8(path.native().data(), static_cast<qsizetype>(path.native().size()));
#endif
}

}  // namespace app::ui

#endif  // OURPAINT_APPLICATION_QT_PATH_H_
