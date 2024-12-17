#pragma once
#ifndef PAINT_23
#define PAINT_23

#include "BMPfile.h"
#include "List.h"
#include "objects.h"
#include "BMPpainter.h"
#include "QTPainter.h"
#include "Assoc.h"
#include "Matrix.h"
#include "FileOurP.h"
#include "LSMFORLMTask.h"
#include "GradientOptimizer.h"
#include "LevenbergMarquardtSolver.h"
#include "LMForTest.h"
#include "simpleGraph.h"
#include "enums.h"
#include "requirements.h"
#include "UndoRedo.h"
#include <map>


struct ElementData {
    Element et;
    std::vector<double> params;
    ElementData();
};

struct ActionsInfo{
    bool isNew;
    std::vector<ID> m_objects;
    std::vector<std::vector<double>> m_paramsBefore;
    std::vector<std::vector<double>> m_paramsAfter;
};

// c_ class
// v_ variable
// s_ structure
// m_ containers(List, Array and other)
// f_ private class method

class Paint {
    // Undo Redo
    UndoRedo<ActionsInfo> c_undoRedo;

    // Point, Section, Circle ID-containers
    std::map<ID, List<point>::iterator> m_pointIDs;
    std::map<ID, List<section>::iterator> m_sectionIDs;
    std::map<ID, List<circle>::iterator> m_circleIDs;

    // Point, Section, Circle containers
    List<point> m_pointStorage;
    List<section> m_sectionStorage;
    List<circle> m_circleStorage;

    // Requirements ID-container
    std::map<ID, List<RequirementData>::iterator> m_reqIDs;

    // Requirements container
    List<RequirementData> m_reqStorage;

    // BMP painting structure
    Painter* c_bmpPainter;

    // Rectangle
    rectangle s_allFigures;

    //
    ID s_maxID;

    // The graph. The edges are represented by requirements and the vertices are represented by objects.
    Graph<RequirementData, ID> m_graph;

public:

    // Default constructor
    Paint(Painter* p) : s_maxID(0), m_pointIDs(),c_bmpPainter(p), m_sectionIDs(), m_circleIDs(), m_pointStorage(),
        m_sectionStorage(), m_circleStorage() {
        s_allFigures.x_1 = -10;
        s_allFigures.x_2 = 10;
        s_allFigures.y_1 = -10;
        s_allFigures.y_2 = 10;
    }
        
    // Addition elements by specifying their type and needed parameters
    ID addElement(const ElementData &ed);
    
    // Addition requirement
    ID addRequirement(const RequirementData &rd);

    // Get information about object
    ElementData getElementInfo(ID id);
    std::vector<std::pair<ID, ElementData>> getAllElementsInfo();
    RequirementData getRequirementInfo(ID id);
    std::vector<std::pair<ID, RequirementData>> getAllRequirementsInfo();

    // Save data in file
    void saveToFile(const char *filename);
    std::string to_string() const;

    // Move element
    void moveElement(const ElementData& currentPos, const ElementData& newPos);
    void parallelMove(ID id, double dx, double dy);
    //Find element by ID
    ID findElement(const ElementData& ed);

    // Export to BMP file
    void exportToBMP(const char *file);

    // Upload data from file
    void loadFromFile(const char *file);

    // load data from string
    void loadFromString(const std::string & str);

    // Set background(change file name)
    // void changeBMP(const BMPfile& file);
    // void changeBMP(const char* filename);
    void setPainter(Painter* p);

    void deleteElement(ID elem);
    void deleteRequirement(ID req);
    void clear();

    void undo();
    void redo();

    void paint();
};

#endif