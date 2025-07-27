#include "Modes.h"

// Initialization of static variables
std::bitset<static_cast<size_t>(WorkModes::Editor) + 1> ModeManager::workModes{
        (1 << static_cast<size_t>(WorkModes::Editor)) |
        (0 << static_cast<size_t>(WorkModes::Move)) |
        (0 << static_cast<size_t>(WorkModes::Section)) |
        (0 << static_cast<size_t>(WorkModes::Point)) |
        (0 << static_cast<size_t>(WorkModes::Circle)) |
        (0 << static_cast<size_t>(WorkModes::Section)) |
        (0 << static_cast<size_t>(WorkModes::Arc)) |
        (0 << static_cast<size_t>(WorkModes::Selected))
        // Editor only = true (lowest bit)
};

std::bitset<static_cast<size_t>(MouseMode::DoubleClickLeft) + 1> ModeManager::mouseModes{
        // We only set ReleasingLeft and ReleasingRight
        (0 << static_cast<size_t>(MouseMode::MouseMove)) |
        (0 << static_cast<size_t>(MouseMode::DoubleClickLeft)) |
        (0 << static_cast<size_t>(MouseMode::LeftClick)) |
        (0 << static_cast<size_t>(MouseMode::RightClick)) |
        (1 << static_cast<size_t>(MouseMode::ReleasingLeft)) |
        (1 << static_cast<size_t>(MouseMode::ReleasingRight))
};

std::bitset<static_cast<size_t>(KeyMode::ReleasingShift) + 1> ModeManager::keyModes{
        // We only set ReleasingTab and ReleasingShift
        (1 << static_cast<size_t>(KeyMode::ReleasingTab)) |
        (1 << static_cast<size_t>(KeyMode::ReleasingEnter)) |
        (1 << static_cast<size_t>(KeyMode::ReleasingShift))
};

bool ModeManager::cellEnabled = true;
bool ModeManager::axisEnabled = true;
bool ModeManager::cursorInArea = true;
bool ModeManager::saveFileMode = true;
bool ModeManager::isConnected = false;
bool ModeManager::isServer = false;

void ModeManager::setActiveMode(WorkModes mode) {
    workModes.reset();
    workModes.set(static_cast<size_t>(mode));
}

bool ModeManager::getActiveMode(WorkModes mode) {
    return workModes.test(static_cast<size_t>(mode));
}

void ModeManager::setNotActiveMode(MouseMode mode) {
    mouseModes.reset(static_cast<size_t>(mode));
}

void ModeManager::setActiveMode(MouseMode mode) {
    if (mode == MouseMode::MouseMove) {
        mouseModes.set(static_cast<size_t>(mode));
        return;
    }
    mouseModes.reset(static_cast<size_t>(MouseMode::LeftClick));
    mouseModes.reset(static_cast<size_t>(MouseMode::ReleasingLeft));
    mouseModes.reset(static_cast<size_t>(MouseMode::RightClick));
    mouseModes.reset(static_cast<size_t>(MouseMode::ReleasingRight));
    mouseModes.reset(static_cast<size_t>(MouseMode::DoubleClickLeft));
    mouseModes.reset(static_cast<size_t>(MouseMode::MouseMove));

    mouseModes.set(static_cast<size_t>(mode));
}

bool ModeManager::getActiveMode(MouseMode mode) {
    return mouseModes.test(static_cast<size_t>(mode));
}

void ModeManager::setActiveMode(KeyMode mode) {
    keyModes.reset();
    keyModes.set(static_cast<size_t>(mode));
}

bool ModeManager::getActiveMode(KeyMode mode) {
    return keyModes.test(static_cast<size_t>(mode));
}

void ModeManager::setCell(bool flag) {
    if (flag == getCell()) {
        return;
    }
    cellEnabled = flag;
    emit instance()->cellChanged(flag);
}

ModeManager* ModeManager::instance() {
    static ModeManager mgr;
    return &mgr;
}

bool ModeManager::getCell() {
    return cellEnabled;
}

void ModeManager::setAxis(bool flag) {
    axisEnabled = flag;
}

bool ModeManager::getAxis() {
    return axisEnabled;
}

void ModeManager::setCursor(bool flag) {
    cursorInArea = flag;
}

bool ModeManager::getCursor() {
    return cursorInArea;
}

void ModeManager::setSave(bool flag) {
    saveFileMode = flag;
}

bool ModeManager::getSave() {
    return saveFileMode;
}

void ModeManager::setConnection(bool flag) {
    isConnected = flag;
}

bool ModeManager::getConnection() {
    return isConnected;
}

void ModeManager::setFlagServer(bool flag) {
    isServer = flag;
}

bool ModeManager::getFlagServer() {
    return isServer;
}
