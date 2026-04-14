add_library(ourpaint_core
        # /saveload/
        headers/saveload/FileOurP.h
        headers/saveload/requirementInFile.h
        headers/saveload/objectInFile.h
        headers/saveload/objectInJson.h
        headers/saveload/SaveLoadJson.h
        headers/saveload/requirementInJson.h
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
        headers/UndoRedo/UndoRedoTemplate.h


        # /
        headers/ISceneObserver.h
        headers/ExceptionGuard.h
        headers/Document.h
        headers/DocumentManager.h
        headers/Scene.h
        src/Scene.cc
        src/Document.cc
        src/DocumentManager.cc
        src/saveload/DocumentSaveLoad.cc
)

target_include_directories(ourpaint_core
        PUBLIC
        headers
        headers/saveload
        headers/UndoRedo
        headers/UndoRedo/commands
        headers/UndoRedo/commands/Add
        headers/UndoRedo/commands/Delete
)
add_subdirectory(DCM {CMAKE_CURRENT_BINARY_DIR}/DCM)
target_link_libraries(ourpaint_core
    PUBLIC
        nlohmann_json::nlohmann_json
        OurPaintDCM
)




















