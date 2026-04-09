#ifndef OURPAINT_QT_PLATFORM_RUNTIME_H_
#define OURPAINT_QT_PLATFORM_RUNTIME_H_

#include "IPlatformRuntime.h"
#include <cstdint>
#include <vector>

class QApplication;
class QtViewportHost;

class QtPlatformRuntime : public IPlatformRuntime {
public:
    QtPlatformRuntime(int& argc, char** argv);
    ~QtPlatformRuntime() override;

    bool            init()                                      override;
    int             run()                                       override;
    IViewportHost*  createViewportHost()                        override;
    void            destroyViewportHost(IViewportHost* host)    override;

private:
    int&            argc_;
    char**          argv_;
    QApplication*   app_  = nullptr;

    std::vector<QtViewportHost*> hosts_;
    uint32_t nextHostId_ = 1;
};

#endif // ! OURPAINT_QT_PLATFORM_RUNTIME_H_


