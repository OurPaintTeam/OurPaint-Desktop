#ifndef OURPAINT_HEADERS_SAVELOAD_REQUIREMENTINFILE_H_
#define OURPAINT_HEADERS_SAVELOAD_REQUIREMENTINFILE_H_

#include <string>
#include "Objects.h"

class requirementInFile {
    unsigned int id;
    Requirement s_req;
    public:
    explicit requirementInFile(const std::pair<unsigned int, Requirement> &obj);

    requirementInFile(const requirementInFile &other);
    requirementInFile(requirementInFile &&other) noexcept;
    requirementInFile &operator=(const requirementInFile &other);
    requirementInFile &operator=(requirementInFile &&other) noexcept;

    std::pair<unsigned int, Requirement> to_pair() const;
    std::string to_string() const;
};


#endif // ! OURPAINT_HEADERS_SAVELOAD_REQUIREMENTINFILE_H_
