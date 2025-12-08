add_executable(Dice)

target_sources(Dice PRIVATE
    main.cpp
    StartScene.cpp
    Painter.cpp
    Game.cpp
    Engine.cpp
    UI.cpp
    Die.cpp
    Slot.cpp
)

set_target_properties(Dice PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED TRUE
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

set(GAMES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/games")
set(TARGET_DIR "${CMAKE_BINARY_DIR}/../../x64/dice")

file(GLOB GAMES_SUBDIRS RELATIVE "${GAMES_DIR}" "${GAMES_DIR}/*")

set(ZIP_COMMANDS "")

foreach(subdir ${GAMES_SUBDIRS})
    if(IS_DIRECTORY "${GAMES_DIR}/${subdir}")
        list(APPEND ZIP_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E make_directory "${TARGET_DIR}"
            COMMAND ${CMAKE_COMMAND} -E tar
            "cf"
            "${TARGET_DIR}/${subdir}.die"
            --format=7zip
            -- "${subdir}"
            WORKING_DIRECTORY "${GAMES_DIR}"
        )
    endif()
endforeach()

add_custom_target(Dice_copyFiles ALL
    ${ZIP_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/dice.zip
    ${CMAKE_BINARY_DIR}/../../x64/
)

add_dependencies(Dice Dice_copyFiles)