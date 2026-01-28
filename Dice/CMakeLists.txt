add_executable(Dice)

target_sources(Dice PRIVATE
    main.cpp
    Die.cpp
    Engine.cpp
    Game.cpp
    Sprite.cpp
    Socket.cpp
    StartScene.cpp
    TexProxy.cpp
    UI.cpp
)

if(EMSCRIPTEN)
    # TODO: create 7z from asset files and include games instead of creating individual .die files
    target_compile_options(Dice PRIVATE -fexperimental-library -pthread)
    configure_file(dice.7z ${CMAKE_BINARY_DIR}/dice.7z COPYONLY)
    target_link_options(Dice PRIVATE -sFULL_ES3 -sASSERTIONS -sPTHREAD_POOL_SIZE=64 -sASYNCIFY -pthread --embed-file dice.7z)
else()
    set(TARGET_DIR "${CMAKE_BINARY_DIR}/../../x64/dice")
    set(GAMES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/games")
    file(MAKE_DIRECTORY "${TARGET_DIR}")
    file(GLOB GAMES_SUBDIRS RELATIVE "${GAMES_DIR}" "${GAMES_DIR}/*")

    add_custom_target(Dice_copyFiles ALL)

    foreach(subdir ${GAMES_SUBDIRS})
        if(IS_DIRECTORY "${GAMES_DIR}/${subdir}")
            add_custom_command(TARGET Dice_copyFiles POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E tar "cf" "${TARGET_DIR}/${subdir}.die" --format=7zip -- "${subdir}"
                WORKING_DIRECTORY "${GAMES_DIR}"
                COMMENT "Zipping ${subdir}..."
            )
        endif()
    endforeach()

    add_custom_command(TARGET Dice_copyFiles POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar "cf" "${TARGET_DIR}/../dice.7z" --format=7zip -- "."
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/zip"
        COMMENT "Creating dice.7z"
    )

    add_dependencies(Dice Dice_copyFiles)
endif()

set_target_properties(Dice PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED TRUE
)
target_compile_options(Dice PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>: /W4>
    $<$<CXX_COMPILER_ID:Clang>: -Wall -Wextra -Wconversion -Wpedantic -Wshadow-all
    -Wno-sign-conversion -Wunreachable-code
    >
    $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -pedantic
    -Wno-missing-field-initializers
    >
)

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(Dice PRIVATE tcob_static)
else()
    target_link_libraries(Dice PRIVATE tcob_shared)
endif()

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET Dice POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Dice> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()
