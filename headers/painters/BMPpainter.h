#ifndef OURPAINT_HEADERS_PAINTERS_BMPPAINTER_H_
#define OURPAINT_HEADERS_PAINTERS_BMPPAINTER_H_

#include <cmath>

#include "BMPfile.h"
#include "objects/GeometricObjects.h"
#include "Painter.h"
#include <list>

class BMPpainter {
    BMPfile c_file;
    unsigned int v_weight;
    unsigned int v_height;

public:
    BMPpainter(unsigned int weight = 1000, unsigned int height = 1000): c_file(weight, height), v_weight(weight), v_height(height){}
    BMPpainter(const BMPfile &file);
    BMPpainter(const BMPpainter &other);
    BMPpainter &operator=(const BMPpainter &other);

    unsigned long long getWeight();
    unsigned long long getHeight();
    void getBoundBox(const BoundBox2D& allObjects);

    void initPoint(/*std::unordered_map<ID, Point*>& points*/);
    void initSection(/*std::unordered_map<ID, Section*>& sections*/);
    void initCircle(/*std::unordered_map<ID, Circle*>& circles*/);



    void saveBMP(const char* filename="print.bmp");
};

#endif // ! OURPAINT_HEADERS_PAINTERS_BMPPAINTER_H_
