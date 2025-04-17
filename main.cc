#include "Application.h"
#include "GUI_Logger.h"

int main(int argc, char *argv[]) {

    freopen(R"(..\\loggs\\guiLog.txt)", "a", stderr);
    logFile.setFileName("guiLog.txt");
    logFile.open(QIODevice::Append | QIODevice::Text);
    qInstallMessageHandler(guiLogger);  // Устанавливаем обработчик

    Application app(argc, argv);
    return app.exec();
}