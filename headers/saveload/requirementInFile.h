#ifndef OURPAINT_HEADERS_SAVELOAD_REQUIREMENTINFILE_H_
#define OURPAINT_HEADERS_SAVELOAD_REQUIREMENTINFILE_H_

#include "requirements.h"
#include <string>

class requirementInFile {
    ID id;
    RequirementData s_req;
    public:
    explicit requirementInFile(const std::pair<ID, RequirementData> &obj);

    requirementInFile(const requirementInFile &other);

    requirementInFile(requirementInFile &&other) noexcept;

    requirementInFile &operator=(const requirementInFile &other);

    requirementInFile &operator=(requirementInFile &&other) noexcept;
    [[nodiscard]] std::pair<ID, RequirementData> to_pair() const;
    [[nodiscard]] std::string to_string() const;
};


#endif // ! OURPAINT_HEADERS_SAVELOAD_REQUIREMENTINFILE_H_
