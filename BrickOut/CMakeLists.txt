add_executable(BrickOut)

target_sources(BrickOut PRIVATE
    main.cpp
    Field.cpp
    GameObject.cpp
    StartScene.cpp
    MainMenu.cpp
    ../_common/UIHelper.cpp
)

set_target_properties(BrickOut PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED TRUE
)

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(BrickOut PRIVATE tcob_static)
else()
    target_link_libraries(BrickOut PRIVATE tcob_shared)
endif()

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET BrickOut POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:BrickOut> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()

configure_file(brickout.zip ${CMAKE_BINARY_DIR}/../../x64/brickout.zip COPYONLY)