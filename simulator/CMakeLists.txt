cmake_minimum_required(VERSION 3.20)

project(candy_simulator LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# Paths
# -----------------------------

set(LVGL_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/external/lv_port_pc_vscode/lvgl
)

# -----------------------------
# SDL2
# -----------------------------

add_library(SDL2 SHARED IMPORTED)

set_target_properties(SDL2 PROPERTIES
    IMPORTED_IMPLIB
        "C:/msys64/ucrt64/lib/libSDL2.dll.a"

    INTERFACE_INCLUDE_DIRECTORIES
        "C:/msys64/ucrt64/include/SDL2"
)

# -----------------------------
# LVGL
# -----------------------------

set(LV_CONF_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h
    CACHE STRING ""
    FORCE
)

add_subdirectory(
    ${LVGL_DIR}
    ${CMAKE_BINARY_DIR}/lvgl
)

# -----------------------------
# Candy Simulator
# -----------------------------

add_executable(candy-simulator
    src/main.cpp

    src/ui/HomeScreen.cpp
    src/ui/QuizScreen.cpp
    src/ui/CandySelectScreen.cpp
    src/ui/DispensingScreen.cpp

    src/ui/theme/Theme.cpp

    ../core/src/game/GameManager.cpp
    ../core/src/quiz/QuizManager.cpp
    ../core/src/quiz/QuizLoader.cpp
    ../core/src/quiz/AnswerValidator.cpp
    ../core/src/quiz/QuizValidator.cpp
    ../core/src/quiz/FileQuizSource.cpp
    ../core/src/quiz/FileQuizProgressStore.cpp
    ../core/src/game/FileGameProgressStore.cpp
    

    ../firmware/src/dispenser/MockDispenser.cpp
)

target_include_directories(candy-simulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LVGL_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src
)

target_compile_definitions(candy-simulator PRIVATE
    SDL_MAIN_HANDLED
)

target_link_libraries(candy-simulator PRIVATE
    lvgl
    SDL2
)