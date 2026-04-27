#ifndef OURPAINT_HEADERS_DOCUMENT_H_
#define OURPAINT_HEADERS_DOCUMENT_H_

#include <string>
namespace core { class Scene;}
namespace UndoRedo { class UndoRedoManager; }
class CommandManager;

class Document {
public:
    explicit Document(const std::string& documentName = "Untitled");
    ~Document();

    std::string& name();
    const std::string& name() const;

    std::string& path();
    const std::string& path() const;

    core::Scene& scene();
    const core::Scene& scene() const;

    UndoRedo::UndoRedoManager& undoRedoManager();
    const UndoRedo::UndoRedoManager& undoRedoManager() const;

    CommandManager& commandManager();
    const CommandManager& commandManager() const;

    bool isDirty() const;

private:
    std::string _filePath{};
    std::string _documentName{};
    bool _isDirty = false;

    core::Scene* _scene;
    UndoRedo::UndoRedoManager* _undoRedo;
    CommandManager* _commandManager;
};


#endif // ! OURPAINT_HEADERS_DOCUMENT_H_