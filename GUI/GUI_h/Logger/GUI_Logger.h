#ifndef OURPAINT_GUI_LOGGER_H
#define OURPAINT_GUI_LOGGER_H

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <stdexcept>

extern QFile logFile;

void guiLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#endif //OURPAINT_GUI_LOGGER_H
