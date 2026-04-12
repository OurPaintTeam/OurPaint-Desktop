#ifndef OURPAINT_APPLICATION_IPLATFORMRUNTIME_H_
#define OURPAINT_APPLICATION_IPLATFORMRUNTIME_H_

class IViewportHost;

class IPlatformRuntime {
public:
    virtual ~IPlatformRuntime() = default;

    // Инициализация платформы. Должна быть вызвана до всего остального.
    virtual bool init() = 0;

    // Запускает event loop. Блокирует до закрытия окна. Cleanup происходит в деструкторе, не здесь.
    virtual int run() = 0;

    // Возвращает viewport. Runtime владеет.
    virtual IViewportHost* createViewportHost() = 0;

    virtual void destroyViewportHost(IViewportHost* host) = 0;
};

#endif // ! OURPAINT_APPLICATION_IPLATFORMRUNTIME_H_