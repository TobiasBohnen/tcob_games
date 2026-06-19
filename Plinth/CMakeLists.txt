add_executable(Plinth)

target_sources(Plinth PRIVATE
    main.cpp
    Plinth.cpp
)

set_target_properties(Plinth PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED TRUE
)

if(EMSCRIPTEN)
    target_compile_options(Plinth PRIVATE -fexperimental-library -pthread)
    configure_file(plinth.zip ${CMAKE_BINARY_DIR}/plinth.zip COPYONLY)
    target_link_options(Plinth PRIVATE -sFULL_ES3 -sASSERTIONS -sPTHREAD_POOL_SIZE=8 -sASYNCIFY -pthread --embed-file plinth.zip)
else()
    configure_file(plinth.zip ${CMAKE_BINARY_DIR}/../../x64/plinth.zip COPYONLY)
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
