add_subdirectory(sketch)

add_library(ourpaint_document STATIC
        Document.h
        Document.cc
        DocumentManager.h
        DocumentManager.cc
        saveload/DocumentSaveLoad.h
        saveload/DocumentSaveLoad.cc
        saveload/CborCodec.h
)
target_compile_features(ourpaint_document PUBLIC cxx_std_20)
target_include_directories(ourpaint_document PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ourpaint_document PUBLIC OurPaint::Sketch)
target_link_libraries(ourpaint_document PRIVATE nlohmann_json::nlohmann_json)

add_library(ourpaint_core
        # /saveload/
        saveload/FileOurP.h
        saveload/requirementInFile.h
        saveload/objectInFile.h
        saveload/objectInJson.h
        saveload/SaveLoadJson.h
        saveload/requirementInJson.h
        saveload/objectInFile.cc
        saveload/FileOurP.cc
        saveload/requirementInFile.cc
        saveload/requirementInJson.cc
        saveload/SaveLoadJson.cc
        saveload/objectInJson.cc

        # /objects/
        objects/GeometricObjects.cc
        objects/BoundBox.cc

        # /UndoRedo/
#        UndoRedo/commands/Add/CommandAddObject.cc
#        UndoRedo/commands/Delete/CommandDeleteObject.cc
#        UndoRedo/commands/CommandAddRequirement.cc
#        UndoRedo/commands/CommandDeleteRequirement.cc
#        UndoRedo/commands/CommandMove.cc
#        UndoRedo/commands/CommandClear.cc
#        UndoRedo/Transaction.cc
#        UndoRedo/UndoRedoTemplate.cc
#        UndoRedo/UndoRedo.cc
#        UndoRedo/ConsoleManager.cc
#        UndoRedo/UndoRedoTemplate.h


        # /
#        ISceneObserver.h
        ExceptionGuard.h
#        Scene.h
#        Scene.cc
)

target_include_directories(ourpaint_core PUBLIC
        /
        saveload
        sketch
#        UndoRedo
#        UndoRedo/commands
#        UndoRedo/commands/Add
#        UndoRedo/commands/Delete
)
target_link_libraries(ourpaint_core
    PUBLIC
        nlohmann_json::nlohmann_json
        OurPaintDCM
        ourpaint_document
)




















