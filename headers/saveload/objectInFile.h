#ifndef OURPAINT_HEADERS_SAVELOAD_OBJECTINFILE_H_
#define OURPAINT_HEADERS_SAVELOAD_OBJECTINFILE_H_

#include <string>

#include "Objects.h"

class objectInFile {
    ID id;
    IGeometricObject *s_obj;
public:
    explicit objectInFile(std::pair<ID, IGeometricObject *> &obj);

    objectInFile(const objectInFile &other);

    objectInFile(objectInFile &&other) noexcept;

    objectInFile &operator=(const objectInFile &other);

    objectInFile &operator=(objectInFile &&other) noexcept;
    std::pair<ID, IGeometricObject *> to_pair() const;

    std::string to_string() const;
};


#endif // ! OURPAINT_HEADERS_SAVELOAD_OBJECTINFILE_H_
