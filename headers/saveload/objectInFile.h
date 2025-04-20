#ifndef OURPAINT_HEADERS_SAVELOAD_OBJECTINFILE_H_
#define OURPAINT_HEADERS_SAVELOAD_OBJECTINFILE_H_

#include <string>

#include "objects/GeometricObjects.h"

class objectInFile {
    unsigned int id;
    IGeometricObject *s_obj;
public:
    explicit objectInFile(std::pair<unsigned int, IGeometricObject *> &obj);

    objectInFile(const objectInFile &other);
    objectInFile(objectInFile &&other) noexcept;
    objectInFile &operator=(const objectInFile &other);
    objectInFile &operator=(objectInFile &&other) noexcept;

    std::pair<unsigned int, IGeometricObject *> to_pair() const;

    std::string to_string() const;
};

#endif // ! OURPAINT_HEADERS_SAVELOAD_OBJECTINFILE_H_