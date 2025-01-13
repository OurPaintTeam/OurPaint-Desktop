#ifndef OURPAINT_HEADERS_PAINTERS_BMPPAINTER_H_
#define OURPAINT_HEADERS_PAINTERS_BMPPAINTER_H_

#include "BMPfile.h"
#include "objects.h"
#include "Painter.h"
#include <cmath>

class BMPpainter : public Painter{
    BMPfile c_file;
    unsigned int v_weight;
    unsigned int v_height;
public:
    BMPpainter(unsigned int weight = 1000, unsigned int height = 1000): c_file(weight, height), v_height(height), v_weight(weight){}
    BMPpainter(const BMPfile &file);
    BMPpainter(const BMPpainter &other);
    BMPpainter &operator=(const BMPpainter &other);

    unsigned long long getWeight();
    unsigned long long getHeight();
    void changeSize(const rectangle& allObjects);

    void drawPoint(point pt, bool isWhite = false);
    void drawSection(section sec, bool isWhite = false);
    void drawCircle(circle c, bool isWhite = false);

    void saveBMP(const char* filename="print.bmp");
};

#endif // ! OURPAINT_HEADERS_PAINTERS_BMPPAINTER_H_
