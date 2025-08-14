add_executable(Rogue)

target_sources(Rogue PRIVATE
    main.cpp

    Common.cpp
    MasterControl.cpp
    StartScene.cpp

    dungeon/Layout.cpp
    dungeon/Dungeon.cpp
    dungeon/Object.cpp
    dungeon/Tile.cpp

    actors/Actor.cpp
    actors/Player.cpp

    ui/MainMenu.cpp
    ui/Renderer.cpp
)

set_target_properties(Rogue PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED TRUE
)

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(Rogue PRIVATE tcob_static)
else()
    target_link_libraries(Rogue PRIVATE tcob_shared)
endif()

set_target_properties(Rogue PROPERTIES OUTPUT_NAME "mage")

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET Rogue POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Rogue> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()

add_custom_target(Rogue_copyFiles ALL

    # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/_scripts/ ${CMAKE_BINARY_DIR}/../../x64/scripts/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/rogue.zip ${CMAKE_BINARY_DIR}/../../x64/
)

add_dependencies(Rogue Rogue_copyFiles)
