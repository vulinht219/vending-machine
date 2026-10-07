file(
    GLOB_RECURSE UI_SOURCES
    "*.cpp"
    "*.c"
)

file(
    GLOB_RECURSE SPECIAL_DAY_IMAGES
    RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
    "assets/special_days/*.png"
)

idf_component_register(
    SRCS
        ${UI_SOURCES}

    INCLUDE_DIRS
        "."
        "screens"
        "theme"
        "fonts"

    REQUIRES
        core
        lvgl__lvgl

    EMBED_FILES
        ${SPECIAL_DAY_IMAGES}
)