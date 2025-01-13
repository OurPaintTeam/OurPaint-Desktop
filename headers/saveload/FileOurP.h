#ifndef OURPAINT_HEADERS_SAVELOAD_EXPORTTOFILE_H_
#define OURPAINT_HEADERS_SAVELOAD_EXPORTTOFILE_H_

#include <fstream>
#include <vector>
#include "objectInFile.h"
#include "requirementInFile.h"
#include <sstream>
#include <algorithm>
#include <queue>

class FileOurP {
    std::vector<objectInFile> m_objects;
    std::vector<requirementInFile> m_requirements;
    void parseFile(std::istream& file);

public:
    FileOurP();

    explicit FileOurP(const std::vector<objectInFile> &obj, std::vector<requirementInFile> &req);

    FileOurP(const FileOurP &other);

    FileOurP(FileOurP &&other) noexcept;

    FileOurP &operator=(const FileOurP &other);

    FileOurP &operator=(FileOurP &&other) noexcept;

    void addObject(std::pair<ID, primitive*> &obj);
    void addRequirement(std::pair<ID, RequirementData> &req);
    void saveToOurP(const std::string &fileName) const;

    void loadFromOurP(const std::string &fileName);

    const std::vector<objectInFile>& getObjects() const;
    const std::vector<requirementInFile>& getRequirements() const;

    std::string to_string() const;

    void loadString(const std::string &basicString);
};


#endif // ! OURPAINT_HEADERS_SAVELOAD_EXPORTTOFILE_H_
