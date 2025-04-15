#ifndef OURPAINT_HEADERS_PAINT_H_
#define OURPAINT_HEADERS_PAINT_H_

#include <map>

#include "BMPfile.h"
#include "BMPpainter.h"
#include "Enums.h"
#include "FileOurP.h"
#include "InheritanceGraph.h"
#include "LMForTest.h"
#include "GeometricObjects.h"
#include "Requirements.h"
#include "UndoRedo.h"
#include <list>

struct ElementData {
    Element et;
    std::vector<double> params;
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
// m_ containers(std::list, Array and other)
// f_ private class method

class Paint {
    // Undo Redo
    UndoRedo<ActionsInfo> c_undoRedo;

    // Point, Section, Circle ID-containers
    std::map<ID, std::list<Point>::iterator> m_pointIDs;
    std::map<ID, std::list<Section>::iterator> m_sectionIDs;
    std::map<ID, std::list<Circle>::iterator> m_circleIDs;

    // Point, Section, Circle containers
    std::list<Point> m_pointStorage;
    std::list<Section> m_sectionStorage;
    std::list<Circle> m_circleStorage;

    // Requirements ID-container
    std::map<ID, std::list<RequirementData>::iterator> m_reqIDs;

    // Requirements container
    std::list<RequirementData> m_reqStorage;

    // BMP painting structure
    Painter* c_bmpPainter;

    // Rectangle
    rectangle s_allFigures;

    //
    ID s_maxID;

    // The graph. The edges are represented by requirements and the vertices are represented by objects.
    UndirectedWeightedGraph<ID, RequirementData> m_graph;

public:

    // Default constructor
    Paint(Painter* p) : s_maxID(0), m_pointIDs(),c_bmpPainter(p), m_sectionIDs(), m_circleIDs(), m_pointStorage(),
        m_sectionStorage(), m_circleStorage() {
        s_allFigures.x_1 = -10;
        s_allFigures.x_2 = 10;
        s_allFigures.y_1 = -10;
        s_allFigures.y_2 = 10;
    }

    // нет деструктора
        
    // Addition elements by specifying their type and needed parameters
    ID addElement(const ElementData &ed);
    
    // Addition requirement
    ID addRequirement(const RequirementData &rd);
    void updateRequirement(ID id);

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
    void parallelMove(ID id, double Cx, double Cy, double dx, double dy);
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
    void LeftMenuMove(ID id, const std::vector<double> &parametrs);
};

#endif // ! OURPAINT_HEADERS_PAINT_H_