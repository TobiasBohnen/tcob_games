add_executable(CurwenManor)

target_sources(CurwenManor PRIVATE
    main.cpp
    GameCanvas.cpp
    Player.cpp
    Tiles.cpp
    assets/GameAssets.cpp
    scenes/BaseScene.cpp
    scenes/StartScene.cpp
    scenes/GameScene.cpp
    scenes/CutScene.cpp
)

set_target_properties(CurwenManor PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(EMSCRIPTEN)
    target_compile_options(CurwenManor PRIVATE -fexperimental-library -pthread)
    target_link_options(CurwenManor PRIVATE -sFULL_ES3 -sASSERTIONS -sPTHREAD_POOL_SIZE=8 -sASYNCIFY --shell-file=${CMAKE_CURRENT_SOURCE_DIR}/basic.html -pthread)
endif()

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(CurwenManor PRIVATE tcob_static)
else()
    target_link_libraries(CurwenManor PRIVATE tcob_shared)
endif()

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET CurwenManor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:CurwenManor> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()
