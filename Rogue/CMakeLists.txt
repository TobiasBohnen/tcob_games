add_executable(Rogue)

target_sources(Rogue PRIVATE
    main.cpp

    Common.cpp
    Layout.cpp
    Level.cpp
    MainMenu.cpp
    StartScene.cpp
)

set_target_properties(Rogue PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(Rogue PRIVATE tcob_static)
else()
    target_link_libraries(Rogue PRIVATE tcob_shared)
endif()

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET Rogue POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Rogue> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()

configure_file(rogue.zip ${CMAKE_BINARY_DIR}/../../x64/rogue.zip COPYONLY)