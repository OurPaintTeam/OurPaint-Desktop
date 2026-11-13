#include "Settings.h"

Settings::Settings(DocumentManager& document) : document_(document) {}

void Settings::set_osnap(bool value) {
    osnap = value;
}

void Settings::set_grid(bool value) {
    grid = value;
}

void Settings::set_step_grid(double value) {
    step_grid = value;
}

void Settings::set_snap_snap(double value) {
    step_snap = value;
}

bool Settings::get_osnap() const {
    return osnap;
}

bool Settings::get_grid() const {
    return grid;
}

double Settings::get_step_grid() const {
    return step_grid;
}

double Settings::get_snap_snap() const {
    return step_snap;
}