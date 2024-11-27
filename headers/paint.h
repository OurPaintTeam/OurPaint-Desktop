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

//c_ - класс, v_- переменная, s_структура, m_ - контейнеры(списки, массивы и другие) f_ - приватный метод класса
class Paint {
    //In the graph, the edges are represented by requirements and the vertices are represented by objects.
    UndoRedo<ActionsInfo> c_undoRedo;
    std::map<ID, List<point>::iterator> m_pointIDs;
    std::map<ID, List<section>::iterator> m_sectionIDs;
    std::map<ID, List<circle>::iterator> m_circleIDs;
    std::map<ID, List<RequirementData>::iterator> m_reqIDs;
    List<RequirementData> m_reqD;
    List<point> m_pointStorage;
    List<section> m_sectionStorage;
    List<circle> m_circleStorage;
    Painter* c_bmpPainter;
    rectangle s_allFigures;
    ID s_maxID;
    Graph<RequirementData, ID> m_graph;

public:
    Paint(Painter* p) : s_maxID(0), m_pointIDs(),c_bmpPainter(p), m_sectionIDs(), m_circleIDs(), m_pointStorage(),
        m_sectionStorage(), m_circleStorage() {
        s_allFigures.x_1 = -10;
        s_allFigures.x_2 = 10;
        s_allFigures.y_1 = -10;
        s_allFigures.y_2 = 10;
    } //default constructor
        
    // Добавление элементов с указанием их типа и необходимого набора параметров
    ID addElement(const ElementData &ed);
    
    // Добавление требованием
    ID addRequirement(const RequirementData &rd);

    // Получение информации об объекте    
    ElementData getElementInfo(ID id);
    std::vector<std::pair<ID, ElementData>> getAllElementsInfo();
    RequirementData getRequirementInfo(ID id);
    std::vector<std::pair<ID, RequirementData>> getAllRequirementsInfo();

    // Сохранение данных в файл
    void saveToFile(const char *filename);
    std::string to_string() const;
    
    // Экспорт в BMP файл
    void exportToBMP(const char *file);
    
    // Загрузка данных из файла
    void loadFromFile(const char *file);
    //Загрузка данных из string
    void loadFromString(const std::string & str);
    // Задает фон(изменяет filename файл)
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