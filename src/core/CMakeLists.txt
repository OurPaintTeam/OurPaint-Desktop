add_library(ourpaint_core
        # /saveload/
        src/saveload/FileOurP.h
        src/saveload/requirementInFile.h
        src/saveload/objectInFile.h
        src/saveload/objectInJson.h
        src/saveload/SaveLoadJson.h
        src/saveload/requirementInJson.h
        src/saveload/objectInFile.cc
        src/saveload/FileOurP.cc
        src/saveload/requirementInFile.cc
        src/saveload/requirementInJson.cc
        src/saveload/SaveLoadJson.cc
        src/saveload/objectInJson.cc

        # /objects/
        src/objects/GeometricObjects.cc
        src/objects/BoundBox.cc

        # /UndoRedo/
        src/UndoRedo/commands/Add/CommandAddObject.cc
        src/UndoRedo/commands/Delete/CommandDeleteObject.cc
        src/UndoRedo/commands/CommandAddRequirement.cc
        src/UndoRedo/commands/CommandDeleteRequirement.cc
        src/UndoRedo/commands/CommandMove.cc
        src/UndoRedo/commands/CommandClear.cc
        src/UndoRedo/Transaction.cc
        src/UndoRedo/UndoRedoTemplate.cc
        src/UndoRedo/UndoRedo.cc
        src/UndoRedo/ConsoleManager.cc
        src/UndoRedo/UndoRedoTemplate.h


        # /
        src/ISceneObserver.h
        src/ExceptionGuard.h
        src/Document.h
        src/DocumentManager.h
        src/Scene.h
        src/Scene.cc
        src/Document.cc
        src/DocumentManager.cc
        src/saveload/DocumentSaveLoad.cc
)

target_include_directories(ourpaint_core
        PUBLIC
        src
        src/saveload
        src/UndoRedo
        src/UndoRedo/commands
        src/UndoRedo/commands/Add
        src/UndoRedo/commands/Delete
)
add_subdirectory(DCM ${CMAKE_CURRENT_BINARY_DIR}/DCM)
target_link_libraries(ourpaint_core
    PUBLIC
        nlohmann_json::nlohmann_json
        OurPaintDCM
)




















