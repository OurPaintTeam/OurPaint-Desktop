#ifndef OURPAINT_GUI_LOGGER_H
#define OURPAINT_GUI_LOGGER_H

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

// Declare logFile as an external variable
extern QFile logFile;

// Function declaration
void guiLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#endif //OURPAINT_GUI_LOGGER_H
