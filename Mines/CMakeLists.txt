add_executable(Mines)

target_sources(Mines PRIVATE
    main.cpp
    StartScene.cpp
    Field.cpp
    MainMenu.cpp
    ../_common/UIHelper.cpp
)

set_target_properties(Mines PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(Mines PRIVATE tcob_static)
else()
    target_link_libraries(Mines PRIVATE tcob_shared)
endif()

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET Mines POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Mines> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()

configure_file(mines.zip ${CMAKE_BINARY_DIR}/../../x64/mines.zip COPYONLY)