#ifndef OURPAINT_REGWIN_H
#define OURPAINT_REGWIN_H

#ifdef Q_OS_WIN
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

inline void registerFileAssociation() {
    const QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());

    QSettings registry("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);

    registry.setValue(".ourp", "OurPaint.Project");

    registry.setValue("OurPaint.Project", "OurPaint Project File");

    registry.setValue("OurPaint.Project/DefaultIcon", QStringLiteral("\"%1\",0").arg(appPath));

    registry.setValue("OurPaint.Project/shell/open/command",
                      QStringLiteral("\"%1\" \"%2\"").arg(appPath, "%1"));

    registry.sync();

    qDebug() << "[RegWin] Registered association for:" << appPath;
}
#endif

#endif //OURPAINT_REGWIN_H