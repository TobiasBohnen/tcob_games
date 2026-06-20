add_executable(Plinth)

target_sources(Plinth PRIVATE
    main.cpp
    Cache.cpp
    Plinth.cpp
    Raycaster.cpp
)

set_target_properties(Plinth PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED TRUE
)

if(EMSCRIPTEN)
    target_compile_options(Plinth PRIVATE -fexperimental-library -pthread)
    target_link_options(Plinth PRIVATE -sFULL_ES3 -sASSERTIONS -sPTHREAD_POOL_SIZE=8 -sASYNCIFY -pthread --embed-file plinth.zip)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plinth-assets.zip)
    if(EMSCRIPTEN)
        configure_file(plinth-assets.zip ${CMAKE_BINARY_DIR}/plinth-assets.zip COPYONLY)
    else()
        configure_file(plinth-assets.zip ${CMAKE_BINARY_DIR}/../../x64/plinth-assets.zip COPYONLY)
    endif()
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
