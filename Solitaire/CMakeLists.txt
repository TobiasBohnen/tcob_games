add_executable(Solitaire)

target_sources(Solitaire PRIVATE
    Cardset.cpp
    Field.cpp
    main.cpp
    StartScene.cpp
    ./ui/UI.cpp
    ./ui/UIHelper.cpp
    ./games/Cards.cpp
    ./games/Games.cpp
    ./games/Piles.cpp
)

set_target_properties(Solitaire PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
)

target_include_directories(Solitaire
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/games
)

target_compile_options(Solitaire PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:TCOB_USE_DEFAULT_MSVC_OPTIONS>>: /W4>
    $<$<CXX_COMPILER_ID:Clang>: -Wall -Wextra
    -Wno-float-equal -Wno-double-promotion -Wno-implicit-int-float-conversion -Wno-sign-conversion -Wno-switch-enum -Wno-switch-default
    -Wno-unsafe-buffer-usage -Wno-ctad-maybe-unsupported -Wno-c++20-compat -Wno-c++98-compat -Wno-c++98-compat-pedantic
    -Wno-exit-time-destructors -Wno-global-constructors -Wno-header-hygiene>
    $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -pedantic
    -Wno-missing-field-initializers>
)

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(Solitaire PRIVATE tcob_static)
else()
    target_link_libraries(Solitaire PRIVATE tcob_shared)
endif()

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET Solitaire POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Solitaire> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()

add_custom_target(Solitaire_copyFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/games/lua/ ${CMAKE_BINARY_DIR}/../../x64/
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/games/sq/ ${CMAKE_BINARY_DIR}/../../x64/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/theme.ini ${CMAKE_BINARY_DIR}/../../x64/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/solitaire.zip ${CMAKE_BINARY_DIR}/../../x64/
)

add_dependencies(Solitaire_copyFiles Solitaire)
