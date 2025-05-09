#include "GUI_Logger.h"

QFile logFile;

void guiLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    QString logMessage;

    switch (type) {
        case QtDebugMsg:
            logMessage = QString("Debug: %1").arg(localMsg.constData());
            break;
        case QtInfoMsg:
            logMessage = QString("Info: %1").arg(localMsg.constData());
            break;
        case QtWarningMsg:
            logMessage = QString("Warning: %1").arg(localMsg.constData());
            break;
        case QtCriticalMsg:
            logMessage = QString("Critical: %1").arg(localMsg.constData());
            break;
        case QtFatalMsg:
            logMessage = QString("Fatal: %1").arg(localMsg.constData());
            break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString fullMessage = QString("[%1] %2\n").arg(timestamp, logMessage);

    if (!logFile.isOpen()) {
        logFile.setFileName("log.txt");
        logFile.open(QIODevice::Append | QIODevice::Text);
    }

    if (logFile.isOpen()) {
        QTextStream out(&logFile);
        out << fullMessage;
        out.flush();
    }

    if (stderr && _fileno(stderr) >= 0) {
        QByteArray byteArray = fullMessage.toLocal8Bit();
        fwrite(byteArray.constData(), 1, byteArray.size(), stderr);
        fflush(stderr);}


        if (type == QtFatalMsg) {
        abort();
      }
}
