add_executable(Dice)

target_sources(Dice PRIVATE
    main.cpp
    StartScene.cpp
    Painter.cpp
    Game.cpp
    Engine.cpp
    ./objects/Die.cpp
    ./objects/Slot.cpp
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

add_custom_target(Dice_copyFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/games/ ${CMAKE_BINARY_DIR}/../../x64/dice/games
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/assets/dice.zip ${CMAKE_BINARY_DIR}/../../x64/
)

add_dependencies(Dice Dice_copyFiles)