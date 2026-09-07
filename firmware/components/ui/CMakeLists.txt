file(
    GLOB_RECURSE UI_SOURCES

    "*.cpp"
    "*.c"
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
)