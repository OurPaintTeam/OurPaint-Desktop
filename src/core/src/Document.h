#ifndef OURPAINT_HEADERS_DOCUMENT_H_
#define OURPAINT_HEADERS_DOCUMENT_H_

#include <string>
class Scene;
namespace UndoRedo { class UndoRedoManager; }
class CommandManager;

class Document {
public:
    explicit Document(const std::string& documentName = "Untitled");
    ~Document();

    std::string& name();
    const std::string& name() const;
    void setName(const std::string& documentName);

    std::string& path();
    const std::string& path() const;
    void setPath(const std::string& filePath);

    Scene& scene();
    const Scene& scene() const;

    UndoRedo::UndoRedoManager& undoRedoManager();
    const UndoRedo::UndoRedoManager& undoRedoManager() const;

    CommandManager& commandManager();
    const CommandManager& commandManager() const;

    bool isDirty() const;
    void markDirty();
    void markClean();

private:
    std::string _filePath{};
    std::string _documentName{};
    bool _isDirty = false;

    Scene* _scene;
    UndoRedo::UndoRedoManager* _undoRedo;
    CommandManager* _commandManager;
};


#endif // ! OURPAINT_HEADERS_DOCUMENT_H_
