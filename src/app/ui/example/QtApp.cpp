#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QSurfaceFormat>
#include <QVBoxLayout>
#include <QWidget>

#include "ViewportWindow.h"

int main(int argc, char *argv[]) {

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    auto* mainWindow = new QMainWindow();
    mainWindow->setWindowTitle("CAD Test Host");
    mainWindow->resize(1000, 700);

    auto* central = new QWidget(mainWindow);
    auto* layout = new QVBoxLayout(central);

    auto* button = new QPushButton("Button", central);

    auto* renderWindow = new ViewportWindow();
    auto* renderWidget = QWidget::createWindowContainer(renderWindow, central);

    renderWidget->setFocusPolicy(Qt::StrongFocus);
    renderWidget->setMinimumSize(640, 480);

    layout->addWidget(button);
    layout->addWidget(renderWidget, 1);

    mainWindow->setCentralWidget(central);

    mainWindow->show();

    return app.exec();
}
