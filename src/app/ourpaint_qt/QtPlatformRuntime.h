#ifndef OURPAINT_QT_PLATFORM_RUNTIME_H_
#define OURPAINT_QT_PLATFORM_RUNTIME_H_

#include <cstdint>
#include <vector>

class QApplication;
class QtViewportHost;

class QtPlatformRuntime {
public:
    QtPlatformRuntime(int& argc, char** argv);
    ~QtPlatformRuntime();

    bool init();
    int run();
    QtViewportHost* createViewportHost();
    void destroyViewportHost(QtViewportHost* host);

private:
    int&            argc_;
    char**          argv_;
    QApplication*   app_  = nullptr;

    std::vector<QtViewportHost*> hosts_;
    uint32_t nextHostId_ = 1;
};

#endif // ! OURPAINT_QT_PLATFORM_RUNTIME_H_


