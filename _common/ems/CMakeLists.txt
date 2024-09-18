if(EMSCRIPTEN)
    target_compile_options(GAME PRIVATE -fexperimental-library -pthread)
    target_link_options(GAME PRIVATE -sFULL_ES3 -sASSERTIONS -sPTHREAD_POOL_SIZE=8 -sASYNCIFY --shell-file=${CMAKE_CURRENT_SOURCE_DIR}/basic.html -pthread)
endif()