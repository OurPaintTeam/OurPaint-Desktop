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

BMPpainter::BMPpainter(const BMPfile &file): c_file(file), v_height(file.getHeight()), v_weight(file.getWidth()){}
/*
 * сделаем некоторые преобразования, чтобы центр СК всегда была в середине листа
 * x = v_height - y; y = v_weight + x
 */
void BMPpainter::initSectionCase(std::unordered_map<ID, Section*>& sections) {
    caseSections=&sections;
}

void BMPpainter::initPointCase(std::unordered_map<ID, Point*>& points) {
    casePoints=&points;
}

void BMPpainter::initCircleCase(std::unordered_map<ID, Circle*>& circles) {
    caseCircles=&circles;
}

void BMPpainter::drawPoints() {
    for (const auto &p : *casePoints) {
        c_file.setPixel(static_cast<int>(v_height / 2 - p.second->y), static_cast<int>( v_weight / 2 + p.second->y ),
                        true);
    }
}

void BMPpainter::drawSections() {
    for (const auto &s : *caseSections) {
        //Алгоритм Брезенхема
        int x0 = static_cast<int>(-s.second->beg->y + v_height / 2);
        int y0 = static_cast<int>(s.second->beg->x + v_weight / 2);
        int x1 = static_cast<int>(-s.second->end->y + v_height / 2);
        int y1 = static_cast<int>(s.second->end->x + v_weight / 2);
        int deltaX = abs(x1 - x0);
        int dirX = x0 < x1 ? 1 : -1;
        int deltaY = -abs(y1 - y0);
        int dirY = y0 < y1 ? 1 : -1;
        int err = deltaX + deltaY;
        int e2;
        for (;;) {
            c_file.setPixel(x0, y0, true);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 <= deltaX) {
                err += deltaX;
                y0 += dirY;
            } /* e_xy+e_y < 0 */
            if (e2 >= deltaY) {
                err += deltaY;
                x0 += dirX;
            } /* e_xy+e_x > 0 */
        }
    }
}

void BMPpainter::drawCircles() {
    for (const auto &c : *caseCircles) {
        int x = 0;
        int y = static_cast<int>(c.second->r);
        int x0 = v_height / 2 - c.second->center->y;
        int y0 = c.second->center->x + v_weight / 2;
        int delta = 1 - 2 * y;
        int error = 0;
        while (y >= x) {
            c_file.setPixel(x0 + x, y0 + y, true);
            c_file.setPixel(x0 + x, y0 - y, true);
            c_file.setPixel(x0 - x, y0 + y, true);
            c_file.setPixel(x0 - x, y0 - y, true);
            c_file.setPixel(x0 + y, y0 + x, true);
            c_file.setPixel(x0 + y, y0 - x, true);
            c_file.setPixel(x0 - y, y0 + x, true);
            c_file.setPixel(x0 - y, y0 - x, true);
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
        }
    }

}

void BMPpainter::drawArcs() {}

void BMPpainter::draw() {
    if (casePoints != nullptr) {
        drawPoints();
    }

    if (caseSections != nullptr) {
        drawSections();
    }

    if (caseCircles != nullptr) {
        drawCircles();
    }

    if (caseArcs != nullptr) {
        drawArcs();
    }
}

void BMPpainter::clear() {}

void BMPpainter::initArcCase(std::unordered_map<ID, Arc*>& arcs) {}

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
