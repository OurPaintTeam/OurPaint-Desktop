#include "Application.h"
#include <Eigen/Core>

int main(int argc, char *argv[]) {
    Eigen::setNbThreads(std::thread::hardware_concurrency());
    Eigen::initParallel();
    Application app(argc, argv);
    return app.exec();
}