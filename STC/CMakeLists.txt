add_executable(STC)

target_sources(STC PRIVATE
    main.cpp
    STC.cpp

    Mission.cpp
    Object.cpp
    Spacecraft.cpp
    Station.cpp
)

set_target_properties(STC PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(EMSCRIPTEN)
    target_compile_options(STC PRIVATE -fexperimental-library -pthread)
    target_link_options(STC PRIVATE -sFULL_ES3 -sASSERTIONS -sPTHREAD_POOL_SIZE=8 -sASYNCIFY -pthread)
endif()

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(STC PRIVATE tcob_static)
else()
    target_link_libraries(STC PRIVATE tcob_shared)
endif()

target_include_directories(STC PRIVATE ../../../tcob/include)

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET STC POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:STC> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()
