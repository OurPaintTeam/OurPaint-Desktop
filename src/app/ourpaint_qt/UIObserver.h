#ifndef OURPAINT_OURPAINTQT_UI_OBSERVER_H_
#define OURPAINT_OURPAINTQT_UI_OBSERVER_H_

#include <QObject>
#include <string>

#include "../editor/IUIObserver.h"

#include <QDataStream>
#include <QtGlobal>
inline QDataStream& operator<<(QDataStream& out, app::ViewId v) {
    return out << static_cast<quint64>(v);
}
inline QDataStream& operator>>(QDataStream& in, app::ViewId& v) {
    quint64 tmp = 0;
    in >> tmp;
    v = static_cast<app::ViewId>(tmp);
    return in;
}

namespace UI {
class ProjectManager;
}

// Translates synchronous App notifications to Qt signals on the GUI thread.
// viewClosing must be delivered directly while findViewport(view) is still valid.
class UIObserver final : public QObject, public app::IUIObserver {
    Q_OBJECT
public:
    void viewAboutToClose(app::ViewId view) override { emit viewClosing(view); }
    void activeViewChanged(app::ViewId view) override { emit activeChanged(view); }
    void toolChanged(app::ViewId view, ToolId tool) override { emit selectedToolChanged(view, tool); }
    void redrawRequested(app::ViewId view) override { emit redraw(view); }

    static void presentActiveTool(UI::ProjectManager& window, ToolId tool);

signals:
    void viewClosing(app::ViewId view);
    void activeChanged(app::ViewId view);
    void selectedToolChanged(app::ViewId view, ToolId tool);
    void redraw(app::ViewId view);
};

#endif // ! OURPAINT_OURPAINTQT_UI_OBSERVER_H_