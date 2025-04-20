#ifndef OURPAINT_HEADERS_SAVELOAD_REQUIREMENTINFILE_H_
#define OURPAINT_HEADERS_SAVELOAD_REQUIREMENTINFILE_H_

#include <string>

#include "Requirements.h"

class requirementInFile {
    unsigned int id;
    RequirementData s_req;
    public:
    explicit requirementInFile(const std::pair<unsigned int, RequirementData> &obj);

    requirementInFile(const requirementInFile &other);
    requirementInFile(requirementInFile &&other) noexcept;
    requirementInFile &operator=(const requirementInFile &other);
    requirementInFile &operator=(requirementInFile &&other) noexcept;

    std::pair<unsigned int, RequirementData> to_pair() const;
    std::string to_string() const;
};


#endif // ! OURPAINT_HEADERS_SAVELOAD_REQUIREMENTINFILE_H_
