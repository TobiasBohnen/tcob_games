add_executable(GAME_NAME)

target_sources(GAME_NAME PRIVATE
    main.cpp
    StartScene.cpp
    Field.cpp
    MainMenu.cpp
    ../_common/UIHelper.cpp
)

set_target_properties(GAME_NAME PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(EMSCRIPTEN)
    target_compile_options(GAME_NAME PRIVATE -fexperimental-library -pthread)
    configure_file(GAME_NAME_LOWER.zip ${CMAKE_BINARY_DIR}/GAME_NAME_LOWER.zip COPYONLY)
    target_link_options(GAME_NAME PRIVATE -sFULL_ES3 -sASSERTIONS -sPTHREAD_POOL_SIZE=8 -sASYNCIFY -pthread --embed-file GAME_NAME_LOWER.zip)
else()
    configure_file(GAME_NAME_LOWER.zip ${CMAKE_BINARY_DIR}/../../x64/GAME_NAME_LOWER.zip COPYONLY)
endif()

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(GAME_NAME PRIVATE tcob_static)
else()
    target_link_libraries(GAME_NAME PRIVATE tcob_shared)
endif()

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET GAME_NAME POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:GAME_NAME> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()
