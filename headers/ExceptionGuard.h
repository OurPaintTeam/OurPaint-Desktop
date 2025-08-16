#ifndef OURPAINT_HEADERS_EXCEPTION_GUARD_H_
#define OURPAINT_HEADERS_EXCEPTION_GUARD_H_

#include <QDebug>
#include <exception>

#define SLOT_GUARD_BEGIN \
    try {

#define SLOT_GUARD_END \
    } catch (const std::exception& ex) { \
        qCritical() << "EXCEPTION: \"" << ex.what() << "\" in" << Q_FUNC_INFO; \
    } catch (...) { \
        qCritical() << "UNKNOWN EXCEPTION in " << Q_FUNC_INFO; \
    }

#define SLOT_GUARD_MAINWIND_BEGIN \
    try {

#define SLOT_GUARD_MAINWIND_END \
    } catch (const std::exception& ex) { \
        qCritical() << "EXCEPTION: \"" << ex.what() << "\" in" << Q_FUNC_INFO; \
        _mainWind.showError((QString("EXCEPTION: \"%1\" in %2").arg(ex.what())).arg(Q_FUNC_INFO)); \
    } catch (...) { \
        qCritical() << "UNKNOWN EXCEPTION in " << Q_FUNC_INFO; \
    }

#endif // ! OURPAINT_HEADERS_EXCEPTION_GUARD_H_