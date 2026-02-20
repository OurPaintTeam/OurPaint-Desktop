#include "Modes.h"


// Initialization of static variables
std::bitset<static_cast<size_t>(WorkModes::Editor) + 1> ModeManager::workModes{
    (1 << static_cast<size_t>(WorkModes::Editor)) |
    (0 << static_cast<size_t>(WorkModes::Move)) |
    (0 << static_cast<size_t>(WorkModes::ShowSize)) |
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
bool ModeManager::isServer = false;
bool ModeManager::isConnected = false;
bool ModeManager::inProject = false;


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


[[maybe_unused]] void ModeManager::setNotActiveMode(KeyMode mode) {
    keyModes.reset(static_cast<size_t>(mode));
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


void ModeManager::setCell(const bool flag) {
    cellEnabled = flag;
}


bool ModeManager::getCell() {
    return cellEnabled;
}


void ModeManager::setAxis(const bool flag) {
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


void ModeManager::setSave(const bool flag) {
    saveFileMode = flag;
}


bool ModeManager::getSave() {
    return saveFileMode;
}


void ModeManager::setProject(const bool flag) {
    inProject = flag;
}


bool ModeManager::getProject() {
    return inProject;
}


void ModeManager::setConnection(const bool flag) {
    isConnected = flag;
}


bool ModeManager::getConnection() {
    return isConnected;
}


void ModeManager::setFlagServer(const bool flag) {
    isServer = flag;
}


bool ModeManager::getFlagServer() {
    return isServer;
}


ModeState ModeManager::copyModes() {
    ModeState state;

    state.workModes = workModes;
    state.mouseModes = mouseModes;
    state.keyModes = keyModes;

    state.cellEnabled = cellEnabled;
    state.axisEnabled = axisEnabled;
    state.inProject = inProject;

    return state;
}


void ModeManager::initModes(const ModeState& modes) {
    workModes = modes.workModes;
    mouseModes = modes.mouseModes;
    keyModes = modes.keyModes;

    cellEnabled = modes.cellEnabled;
    axisEnabled = modes.axisEnabled;
    inProject = modes.inProject;
}
