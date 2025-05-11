#include "GUI_Logger.h"

QFile logFile;

void guiLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString logMessage;

    switch (type) {
        case QtDebugMsg:    logMessage = "Debug";   break;
        case QtInfoMsg:     logMessage = "Info";    break;
        case QtWarningMsg:  logMessage = "Warning"; break;
        case QtCriticalMsg: logMessage = "Critical";break;
        case QtFatalMsg:    logMessage = "Fatal";   break;
    }

    QString times = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString fullMessage = QString("[%1] %2: %3\n").arg(times, logMessage, msg);

    if (!logFile.isOpen()) {
        logFile.setFileName("log.txt");
        if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
            throw std::runtime_error("Failed to open log file");
        }
    }

    QTextStream out(&logFile);
    out << fullMessage;
    out.flush();

    if (type == QtFatalMsg) {
        abort();
    }
}
