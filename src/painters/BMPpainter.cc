#include "BMPpainter.h"

BMPpainter::BMPpainter(const BMPpainter &other): c_file(other.c_file), v_weight(other.v_weight), v_height(other.v_height){}

BMPpainter& BMPpainter::operator=(const BMPpainter &other)
{
    if(this!=&other){
        c_file=other.c_file;
        v_weight = other.v_weight;
        v_height = other.v_height;
    }
    return *this;
}

BMPpainter::BMPpainter(const BMPfile &file): c_file(file), v_weight(file.getWidth()), v_height(file.getHeight()){}
/*
 * сделаем некоторые преобразования, чтобы центр СК всегда была в середине листа
 * x = v_height - y; y = v_weight + x
 */
void BMPpainter::initSection(/*std::unordered_map<ID, Section*>& sections*/) {
    //Алгоритм Брезенхема
  /*  int x0 = static_cast<int>(-s.beg->y + v_height / 2);
    int y0 = static_cast<int>(s.beg->x + v_weight / 2);
    int x1 = static_cast<int>(-s.end->y + v_height / 2);
    int y1 = static_cast<int>(s.end->x + v_weight / 2);
    int deltaX = abs(x1 - x0);
    int dirX = x0 < x1 ? 1 : -1;
    int deltaY = -abs(y1 - y0);
    int dirY = y0 < y1 ? 1 : -1;
    int err = deltaX + deltaY;
    int e2;
    for (;;) {
        c_file.setPixel(x0, y0, isWhite);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 <= deltaX) { err += deltaX; y0 += dirY; }  e_xy+e_y < 0 */
     //   if (e2 >= deltaY) { err += deltaY; x0 += dirX; } /* e_xy+e_x > 0 */
   // }
}

void BMPpainter::initPoint(/*std::unordered_map<ID, Point*>& points*/) {
 //   c_file.setPixel(static_cast<int>(v_height/2 - p.y), static_cast<int>( v_weight / 2 + p.x ), isWhite);
}

void BMPpainter::initCircle(/*std::unordered_map<ID, Circle*>& circles*/) {
   /* int x = 0;
    int y = static_cast<int>(c.r);
    int x0 = v_height/2 - c.center->y;
    int y0 = c.center->x + v_weight/2;
    int delta = 1 - 2 * y;
    int error = 0;
    while (y >= x) {
        c_file.setPixel(x0 + x, y0 + y, isWhite);
        c_file.setPixel(x0 + x, y0 - y, isWhite);
        c_file.setPixel(x0 - x, y0 + y, isWhite);
        c_file.setPixel(x0 - x, y0 - y, isWhite);
        c_file.setPixel(x0 + y, y0 + x, isWhite);
        c_file.setPixel(x0 + y, y0 - x, isWhite);
        c_file.setPixel(x0 - y, y0 + x, isWhite);
        c_file.setPixel(x0 - y, y0 - x, isWhite);
        error = (delta + y) * 2 - 1;
        if ((delta < 0) && (error <= 0)) {
            delta += 2 * ++x + 1;
            continue;
        }
        if ((delta > 0) && (error > 0)) {
            delta -= 2 * --y + 1;
            continue;
        }
        delta += 2 * (++x - --y);
    }*/
}

void BMPpainter::saveBMP(const char* filename) {
    c_file.saveBmp(filename);
}

void BMPpainter::getBoundBox(const BoundBox2D &allObjects) {
    int newWeight = allObjects.max_x - allObjects.min_x > 0 ? allObjects.max_x - allObjects.min_x: -allObjects.max_x + allObjects.min_x;
    int newHeight = allObjects.max_y - allObjects.min_y > 0 ? allObjects.max_y - allObjects.min_y: -allObjects.max_y + allObjects.min_y;
    c_file.resize(newWeight * 2, newHeight * 2);
    v_height = newHeight * 2;
    v_weight = newWeight * 2;
}

unsigned long long BMPpainter::getHeight() {
    return v_height;
}

unsigned long long BMPpainter::getWeight() {
    return v_weight;
}
