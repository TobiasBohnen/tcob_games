add_executable(Solitide)

target_sources(Solitide PRIVATE
    Database.cpp
    MainScene.cpp
    StartScene.cpp
    main.cpp

    ./gfx/Canvases.cpp
    ./gfx/CardRenderer.cpp
    ./gfx/CardSet.cpp
    ./gfx/CardTable.cpp

    ./ui/UI.cpp
    ./ui/Themes.cpp
    ./ui/Styles.cpp
    ./ui/Translator.cpp

    ./wizard/Wizard.cpp
    ./wizard/WizardScene.cpp

    ./games/Cards.cpp
    ./games/Games.cpp
    ./games/GameInfo.cpp
    ./games/Piles.cpp
)

set_target_properties(Solitide PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED TRUE
)

target_include_directories(Solitide
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_compile_options(Solitide PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>: /W4>
    $<$<CXX_COMPILER_ID:Clang>: -Wall -Wextra
    -Wno-float-equal -Wno-double-promotion -Wno-implicit-int-float-conversion -Wno-sign-conversion -Wno-switch-enum -Wno-switch-default
    -Wno-unsafe-buffer-usage -Wno-ctad-maybe-unsupported -Wno-c++20-compat -Wno-c++98-compat -Wno-c++98-compat-pedantic
    -Wno-exit-time-destructors -Wno-global-constructors -Wno-header-hygiene>
    $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -pedantic -Wno-missing-field-initializers>
)

if(NOT TCOB_BUILD_SHARED)
    target_link_libraries(Solitide PRIVATE tcob_static)
else()
    target_link_libraries(Solitide PRIVATE tcob_shared)
endif()

if(TCOB_BUILD_SHARED AND NOT TCOB_IS_CI)
    add_custom_command(TARGET Solitide POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Solitide> ${CMAKE_BINARY_DIR}/../../x64/
        COMMAND_EXPAND_LISTS
    )
endif()

add_custom_target(Solitide_copyFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/games/lua/ ${CMAKE_BINARY_DIR}/../../x64/scripts/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/styles.json ${CMAKE_BINARY_DIR}/../../x64/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/themes.json ${CMAKE_BINARY_DIR}/../../x64/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/assets.zip ${CMAKE_BINARY_DIR}/../../x64/
)

add_dependencies(Solitide Solitide_copyFiles)

# CPack configuration
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "Solitide")
set(CPACK_PACKAGE_VERSION "0.0.1")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Solitide Game Package")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Solitide")

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY, TRUE)
install(TARGETS Solitide DESTINATION .)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/games/lua/ DESTINATION scripts)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/styles.json DESTINATION .)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/themes.json DESTINATION .)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets.zip DESTINATION .)
install(FILES ${PROJECT_SOURCE_DIR}/license.txt DESTINATION .)
include(CPack)
