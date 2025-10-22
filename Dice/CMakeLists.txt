add_executable(Dice)

target_sources(Dice PRIVATE
    main.cpp
    StartScene.cpp
    Painter.cpp
    ./games/Field.cpp
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

configure_file(./assets/dice.zip ${CMAKE_BINARY_DIR}/../../x64/dice.zip COPYONLY)