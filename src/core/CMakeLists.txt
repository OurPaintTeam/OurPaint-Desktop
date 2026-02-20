add_library(ourpaint_core
        ../headers/saveload/FileOurP.h
        ../headers/saveload/requirementInFile.h
        ../headers/saveload/objectInFile.h
        ../headers/saveload/objectInJson.h
        ../headers/saveload/SaveLoadJson.h
        ../headers/saveload/requirementInJson.h
        ../headers/BMPfile.h
        ../headers/SceneObserver.h
        ../headers/ExceptionGuard.h
        ../headers/objects/Enums.h
        ../headers/objects/Objects.h
        ../headers/objects/GeometricObjects.h
        ../headers/objects/Component.h
        ../headers/objects/ID.h
        ../headers/Scene.h
        ../headers/UndoRedo/UndoRedoTemplate.h
        ../headers/objects/BoundBox.h






        Scene.cc
        objects/GeometricObjects.cc
        objects/BoundBox.cc
        saveload/objectInFile.cc
        saveload/FileOurP.cc
        saveload/requirementInFile.cc
        saveload/requirementInJson.cc
        saveload/SaveLoadJson.cc
        saveload/objectInJson.cc
        UndoRedo/commands/Add/CommandAddObject.cc
        UndoRedo/commands/Delete/CommandDeleteObject.cc
        UndoRedo/commands/CommandAddRequirement.cc
        UndoRedo/commands/CommandDeleteRequirement.cc
        UndoRedo/commands/CommandMove.cc
        UndoRedo/commands/CommandClear.cc
        UndoRedo/Transaction.cc
        UndoRedo/UndoRedoTemplate.cc
        UndoRedo/UndoRedo.cc
        UndoRedo/ConsoleManager.cc
)

target_include_directories(ourpaint_core
        PUBLIC
        ../headers
        ../headers/objects
        ../headers/painters
        ../headers/saveload
        ../headers/UndoRedo
        ../headers/UndoRedo/commands
        ../headers/UndoRedo/commands/Add
        ../headers/UndoRedo/commands/Delete
)
target_link_libraries(ourpaint_core
    PUBLIC
        Math
        nlohmann_json::nlohmann_json
)




















