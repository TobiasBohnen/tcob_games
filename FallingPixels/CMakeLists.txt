add_executable(FallingPixels)

target_sources(FallingPixels PRIVATE
    main.cpp
    FallingPixels.cpp
    Element.cpp
    UI.cpp
)

set_target_properties(FallingPixels PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

if(EMSCRIPTEN)
    target_compile_options(FallingPixels PRIVATE -fexperimental-library -pthread)
    target_link_options(FallingPixels PRIVATE -sFULL_ES3 -sASSERTIONS -sPTHREAD_POOL_SIZE=8 -sASYNCIFY -pthread)
endif()

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(FallingPixels PRIVATE tcob_static)
else()
    target_link_libraries(FallingPixels PRIVATE tcob_shared)
endif()

target_include_directories(FallingPixels PRIVATE ../../../tcob/include)

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET FallingPixels POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:FallingPixels> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()
