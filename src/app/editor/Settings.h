#ifndef OURPAINT_SETTINGS_H
#define OURPAINT_SETTINGS_H
#include "DocumentManager.h"

class Settings {
public:
    Settings(DocumentManager& document);

    void   set_osnap(bool value);
    void   set_grid(bool value);
    void   set_step_grid(double value);
    void   set_snap_snap(double value);

    bool   get_osnap()    const;
    bool   get_grid()     const;
    double get_step_grid() const;
    double get_snap_snap() const;

private:
    DocumentManager& document_;
    bool   osnap     = true;
    bool   grid      = true;
    double step_grid = 0.0;
    double step_snap = 0.0;
};

#endif  // OURPAINT_SETTINGS_H