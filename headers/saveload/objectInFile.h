#ifndef OURPAINT_HEADERS_SAVELOAD_OBJECTINFILE_H_
#define OURPAINT_HEADERS_SAVELOAD_OBJECTINFILE_H_

#include "objects.h"
#include <string>

class objectInFile {
    ID id;
    primitive *s_obj;
public:
    explicit objectInFile(std::pair<ID, primitive *> &obj);

    objectInFile(const objectInFile &other);

    objectInFile(objectInFile &&other) noexcept;

    objectInFile &operator=(const objectInFile &other);

    objectInFile &operator=(objectInFile &&other) noexcept;
    std::pair<ID, primitive *> to_pair() const;

    std::string to_string() const;
};


#endif // ! OURPAINT_HEADERS_SAVELOAD_OBJECTINFILE_H_
