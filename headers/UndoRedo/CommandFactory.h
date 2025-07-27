#ifndef OURPAINT_HEADERS_UNDOREDO_COMMAND_FACTORY_H_
#define OURPAINT_HEADERS_UNDOREDO_COMMAND_FACTORY_H_

#include <string>
#include <vector>
#include "Transaction.h"
#include "Scene.h"
#include "Objects.h"

#include "CommandAddObject.h"
#include "CommandAddPoint.h"
#include "CommandAddSection.h"
#include "CommandAddCircle.h"
#include "CommandAddArc.h"
#include "CommandDeleteObject.h"
#include "CommandDeletePoint.h"
#include "CommandDeleteSection.h"
#include "CommandDeleteCircle.h"
#include "CommandDeleteArc.h"
#include "CommandAddRequirement.h"
#include "CommandDeleteRequirement.h"
#include "CommandMove.h"

using namespace UndoRedo;

class ICommandFactory {
public:
    virtual ~ICommandFactory() = default;

    virtual std::string id() const = 0;
    virtual std::string hint() const = 0;

    virtual void createCommands(const std::vector<std::string>& rawArgs, UndoRedo::Transaction& txn) const = 0;
    virtual void createCommands(const std::vector<double>& rawArgs, UndoRedo::Transaction& txn) const = 0;
};



class PointFactory : public ICommandFactory {
public:
    PointFactory(Scene& sc) : _scene(sc) {}

    std::string id() const override {
        return "POINT";
    }

    std::string hint() const override {
        return "POINT X Y";
    }

    void createCommands(const std::vector<std::string>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() != 2) {
            throw std::runtime_error("POINT: need 2 numbers");
        }
        ObjectData obj;
        obj.et = ObjType::ET_POINT;
        obj.params = {
                std::stod(rawArgs[0]),
                std::stod(rawArgs[1]),
        };
        txn.addCommand(new CommandAddPoint(_scene, obj));
    }

    void createCommands(const std::vector<double>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() != 2) {
            throw std::runtime_error("POINT: need 2 numbers");
        }
        ObjectData obj;
        obj.et = ObjType::ET_POINT;
        obj.params = {
                rawArgs[0],
                rawArgs[1],
        };
        txn.addCommand(new CommandAddPoint(_scene, obj));
    }

private:
    Scene& _scene;
};

class LineFactory : public ICommandFactory {
public:
    LineFactory(Scene& sc) : _scene(sc) {}

    std::string id() const override {
        return "LINE";
    }

    std::string hint() const override {
        return "LINE X1 Y1 X2 Y2";
    }

    void createCommands(const std::vector<std::string>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() != 4) {
            throw std::runtime_error("LINE: need 4 numbers");
        }
        ObjectData obj;
        obj.et = ObjType::ET_SECTION;
        obj.params = {
                std::stod(rawArgs[0]),
                std::stod(rawArgs[1]),
                std::stod(rawArgs[2]),
                std::stod(rawArgs[3])
        };
        txn.addCommand(new CommandAddSection(_scene, obj));
    }

    void createCommands(const std::vector<double>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() != 4) {
            throw std::runtime_error("LINE: need 4 numbers");
        }
        ObjectData obj;
        obj.et = ObjType::ET_SECTION;
        obj.params = {
                rawArgs[0],
                rawArgs[1],
                rawArgs[2],
                rawArgs[3]
        };
        txn.addCommand(new CommandAddSection(_scene, obj));
    }

private:
    Scene& _scene;
};

class CircleFactory : public ICommandFactory {
public:
    CircleFactory(Scene& sc) : _scene(sc) {}

    std::string id() const override {
        return "CIRCLE";
    }

    std::string hint() const override {
        return "CIRCLE X Y R";
    }

    void createCommands(const std::vector<std::string>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() != 3) {
            throw std::runtime_error("CIRCLE: need 3 numbers");
        }
        ObjectData obj;
        obj.et = ObjType::ET_CIRCLE;
        obj.params = {
                std::stod(rawArgs[0]),
                std::stod(rawArgs[1]),
                std::stod(rawArgs[2])
        };
        txn.addCommand(new CommandAddCircle(_scene, obj));
    }

    void createCommands(const std::vector<double>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() != 3) {
            throw std::runtime_error("CIRCLE: need 3 numbers");
        }
        ObjectData obj;
        obj.et = ObjType::ET_CIRCLE;
        obj.params = {
                rawArgs[0],
                rawArgs[1],
                rawArgs[2]
        };
        txn.addCommand(new CommandAddCircle(_scene, obj));
    }

private:
    Scene& _scene;
};

class ArcFactory : public ICommandFactory {
public:
    ArcFactory(Scene& sc) : _scene(sc) {}

    std::string id() const override {
        return "ARC";
    }

    std::string hint() const override {
        return "ARC X1 Y1 X2 Y2 CX CY";
    }

    void createCommands(const std::vector<std::string>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() != 6) {
            throw std::runtime_error("ARC: need 6 numbers");
        }
        ObjectData obj;
        obj.et = ObjType::ET_ARC;
        obj.params = {
                std::stod(rawArgs[0]),
                std::stod(rawArgs[1]),
                std::stod(rawArgs[2]),
                std::stod(rawArgs[3]),
                std::stod(rawArgs[4]),
                std::stod(rawArgs[5])
        };
        txn.addCommand(new CommandAddCircle(_scene, obj));
    }

    void createCommands(const std::vector<double>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() != 6) {
            throw std::runtime_error("ARC: need 6 numbers");
        }
        ObjectData obj;
        obj.et = ObjType::ET_ARC;
        obj.params = {
                rawArgs[0],
                rawArgs[1],
                rawArgs[2],
                rawArgs[3],
                rawArgs[4],
                rawArgs[5]
        };
        txn.addCommand(new CommandAddCircle(_scene, obj));
    }

private:
    Scene& _scene;
};

class ReqFactory : public ICommandFactory {
public:
    ReqFactory(Scene& sc) : _scene(sc) {}

    std::string id() const override {
        return "REQ";
    }

    std::string hint() const override {
        return "REQ REQ_ID OBJ_1 OBJ_2 PARAM";
    }

    void createCommands(const std::vector<std::string>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() < 3) {
            throw std::runtime_error("REQ ERROR:    REQ REQ_ID OBJ_1 OBJ_2 PARAM  ");
        }

        Requirement req;
        req.obj1 = ID(std::stoi(rawArgs[1]));
        req.obj2 = ID(std::stoi(rawArgs[2]));

        if (rawArgs.size() >= 4) {
            req.param = std::stod(rawArgs[3]);
        }

        int req_id = std::stoi(rawArgs[0]);
        switch (req_id) {
            case 1:
                req.type = ReqType::ET_POINTSECTIONDIST;
                break;
            case 2:
                req.type = ReqType::ET_POINTONSECTION;
                break;
            case 3:
                req.type = ReqType::ET_POINTPOINTDIST;
                break;
            case 4:
                req.type = ReqType::ET_POINTONPOINT;
                break;
            case 5:
                req.type = ReqType::ET_SECTIONCIRCLEDIST;
                break;
            case 6:
                req.type = ReqType::ET_SECTIONONCIRCLE;
                break;
            case 7:
                req.type = ReqType::ET_SECTIONINCIRCLE;
                break;
            case 8:
                req.type = ReqType::ET_SECTIONSECTIONPARALLEL;
                break;
            case 9:
                req.type = ReqType::ET_SECTIONSECTIONPERPENDICULAR;
                break;
            case 10:
                req.type = ReqType::ET_SECTIONSECTIONANGLE;
                break;
            default:
                throw std::runtime_error("REQ ERROR:    REQ REQ_ID OBJ_1 OBJ_2 PARAM  ");
                break;
        }

        txn.addCommand(new CommandAddRequirement(_scene, req));
    }

    void createCommands(const std::vector<double>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() < 3) {
            throw std::runtime_error("REQ ERROR:    REQ REQ_ID OBJ_1 OBJ_2 PARAM  ");
        }

        Requirement req;
        req.obj1 = ID(rawArgs[1]);
        req.obj2 = ID(rawArgs[2]);

        if (rawArgs.size() >= 4) {
            req.param = rawArgs[3];
        }

        int req_id = rawArgs[0];
        switch (req_id) {
            case 1:
                req.type = ReqType::ET_POINTSECTIONDIST;
                break;
            case 2:
                req.type = ReqType::ET_POINTONSECTION;
                break;
            case 3:
                req.type = ReqType::ET_POINTPOINTDIST;
                break;
            case 4:
                req.type = ReqType::ET_POINTONPOINT;
                break;
            case 5:
                req.type = ReqType::ET_SECTIONCIRCLEDIST;
                break;
            case 6:
                req.type = ReqType::ET_SECTIONONCIRCLE;
                break;
            case 7:
                req.type = ReqType::ET_SECTIONINCIRCLE;
                break;
            case 8:
                req.type = ReqType::ET_SECTIONSECTIONPARALLEL;
                break;
            case 9:
                req.type = ReqType::ET_SECTIONSECTIONPERPENDICULAR;
                break;
            case 10:
                req.type = ReqType::ET_SECTIONSECTIONANGLE;
                break;
            default:
                throw std::runtime_error("REQ ERROR:    REQ REQ_ID OBJ_1 OBJ_2 PARAM  ");
                break;
        }

        txn.addCommand(new CommandAddRequirement(_scene, req));
    }

private:
    Scene& _scene;
};

class DelFactory : public ICommandFactory {
public:
    DelFactory(Scene& sc) : _scene(sc) {}

    std::string id() const override {
        return "DELETE";
    }

    std::string hint() const override {
        return "DELETE REQ_OR_OBJ_ID";
    }

    void createCommands(const std::vector<std::string>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() < 1 || rawArgs.size() > 2) {
            throw std::runtime_error("DELETE: expected 1 or 2 arguments");
        }

        if (rawArgs.size() == 2) {
            ID id(std::stoi(rawArgs[1]));
            if (rawArgs[0] == "REQ") {
                txn.addCommand(new CommandDeleteRequirement(_scene, id));
            }
            else if (rawArgs[0] == "OBJ") {
                txn.addCommand(new CommandDeleteObject(_scene, id));
            }
            else {
                throw std::runtime_error("DELETE: Unknown " + rawArgs[0]);
            }
        }
        else if (rawArgs.size() == 1) {
            ID id(std::stoi(rawArgs[0]));
            bool bObj = _scene.hasObject(id);
            bool bReq = _scene.hasRequirement(id);
            if (bObj && bReq) {
                throw std::runtime_error("DELETE: Error: More than one entity with ID " + std::to_string(id.get()) + " was found.");
            }
            else if (bObj) {
                txn.addCommand(new CommandDeleteObject(_scene, id));
            }
            else if (bReq) {
                txn.addCommand(new CommandDeleteRequirement(_scene, id));
            }
            else {
                throw std::runtime_error("DELETE: no one with id " + std::to_string(id.get()) + " found");
            }
        }
        else {
            throw std::runtime_error("DELETE: need 1 number");
        }
    }

    void createCommands(const std::vector<double>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() < 3) {
            throw std::runtime_error("REQ ERROR:    REQ REQ_ID OBJ_1 OBJ_2 PARAM  ");
        }

        Requirement req;
        req.obj1 = ID(rawArgs[1]);
        req.obj2 = ID(rawArgs[2]);

        if (rawArgs.size() >= 4) {
            req.param = rawArgs[3];
        }

        int req_id = rawArgs[0];
        switch (req_id) {
            case 1:
                req.type = ReqType::ET_POINTSECTIONDIST;
                break;
            case 2:
                req.type = ReqType::ET_POINTONSECTION;
                break;
            case 3:
                req.type = ReqType::ET_POINTPOINTDIST;
                break;
            case 4:
                req.type = ReqType::ET_POINTONPOINT;
                break;
            case 5:
                req.type = ReqType::ET_SECTIONCIRCLEDIST;
                break;
            case 6:
                req.type = ReqType::ET_SECTIONONCIRCLE;
                break;
            case 7:
                req.type = ReqType::ET_SECTIONINCIRCLE;
                break;
            case 8:
                req.type = ReqType::ET_SECTIONSECTIONPARALLEL;
                break;
            case 9:
                req.type = ReqType::ET_SECTIONSECTIONPERPENDICULAR;
                break;
            case 10:
                req.type = ReqType::ET_SECTIONSECTIONANGLE;
                break;
            default:
                throw std::runtime_error("REQ ERROR:    REQ REQ_ID OBJ_1 OBJ_2 PARAM  ");
                break;
        }

        txn.addCommand(new CommandAddRequirement(_scene, req));
    }

private:
    Scene& _scene;
};

class GridOnFactory : public ICommandFactory {
public:
    GridOnFactory();

    std::string id() const override {
        return "GRID";
    }

    std::string hint() const override {
        return "GRID ON";
    }

    void createCommands(const std::vector<std::string>& rawArgs, UndoRedo::Transaction& txn) const override {
        if (rawArgs.size() != 1) {
            throw std::runtime_error("GRID: expected 1 argument, ON or OFF");
        }

        if (rawArgs[0] == "ON") {

        }
        else if (rawArgs[0] == "OFF") {

        }
    }

    void createCommands(const std::vector<double>& rawArgs, UndoRedo::Transaction& txn) const override {}

private:

};

#endif // ! OURPAINT_HEADERS_UNDOREDO_COMMAND_FACTORY_H_