if(TCOB_ENABLE_ADDON_PHYSICS_BOX2D
    AND TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT
    AND TCOB_ENABLE_ADDON_SCRIPTING_LUA
    AND TCOB_ENABLE_ADDON_GFX_LITEHTML)
    add_subdirectory(Mines)
    add_subdirectory(Snake)
    add_subdirectory(BrickOut)
    add_subdirectory(Solitide)
endif()