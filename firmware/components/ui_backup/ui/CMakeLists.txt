idf_component_register(
    SRCS
        "HomeScreen.cpp"

        "assets/home/home_1.c"
        "assets/home/home_2.c"
        "assets/home/home_3.c"
        "assets/home/home_title.c"
        "assets/home/home_button.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        lvgl__lvgl
)