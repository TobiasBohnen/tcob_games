add_executable(Plinth)

target_sources(Plinth PRIVATE
    main.cpp
    Level.cpp
    LevelObject.cpp
    MapRenderer.cpp
    Player.cpp
    Plinth.cpp
    Raycaster.cpp
    TextureCache.cpp
    Voxel.cpp
    Walls.cpp
    Engine.cpp
)

set_target_properties(Plinth PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED TRUE
)

add_custom_target(PlinthScripts ALL
    COMMAND ${CMAKE_COMMAND} -E tar "cf" ${CMAKE_BINARY_DIR}/../../x64/plinth-scripts.zip --format=7zip -- "."
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts
    COMMENT "Creating plinth-scripts.zip..."
)
add_dependencies(Plinth PlinthScripts)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plinth-assets.zip)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/plinth-assets.zip DESTINATION ${CMAKE_BINARY_DIR}/../../x64/)
endif()

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(Plinth PRIVATE tcob_static)
else()
    target_link_libraries(Plinth PRIVATE tcob_shared)
endif()

target_include_directories(Plinth PRIVATE ../../../tcob/include)

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET Plinth POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Plinth> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()
