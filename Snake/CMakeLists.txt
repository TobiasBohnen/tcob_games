add_executable(Snake)

target_sources(Snake PRIVATE
    main.cpp
    StartScene.cpp
    Field.cpp
    FieldItems.cpp
    MainMenu.cpp
    ../_common/UIHelper.cpp
)

set_target_properties(Snake PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(Snake PRIVATE tcob_static)
else()
    target_link_libraries(Snake PRIVATE tcob_shared)
endif()

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET Snake POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Snake> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()

configure_file(snake.zip ${CMAKE_BINARY_DIR}/../../x64/snake.zip COPYONLY)
