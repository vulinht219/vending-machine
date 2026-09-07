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
    src/ui/screens/SpecialEventIntroScreen.cpp
    src/ui/CandySelectScreen.cpp
    src/ui/DispensingScreen.cpp
    src/ui/SpecialEventScreen.cpp
    src/platform/FileSpecialEventProgressStore.cpp
    src/app/AppController.cpp
    src/ui/SpecialDayBackground.cpp

    src/ui/fonts/quiz_symbols_18.c
    src/ui/fonts/jersey25_85.c

    assets/result/correct_title.c
    assets/result/incorrect_title.c


    assets/home/home_1.c
    assets/home/home_2.c
    assets/home/home_3.c
    assets/home/home_button.c
    assets/home/home_title.c

    assets/quiz/quiz_back_button.c
    assets/quiz/quiz_popup.c

    assets/result/correct_1.c
    assets/result/correct_2.c
    assets/result/correct_3.c
    assets/result/correct_title.c

    assets/result/incorrect_1.c
    assets/result/incorrect_2.c
    assets/result/incorrect_3.c
    assets/result/incorrect_title.c

    assets/candy_select/candy_title.c
    assets/candy_select/candy_panel.c
    assets/candy_select/candy_button.c

    assets/dispensing/dispensing_1.c
    assets/dispensing/dispensing_2.c
    assets/dispensing/dispensing_3.c
    assets/dispensing/dispensing_title.c

    # =========================================================
    # SPECIAL DAYS BACKGROUNDS
    # =========================================================

    assets/special_days/ab/ab.c
    assets/special_days/ab/ab_title.c
    assets/special_days/ab/ab_button.c

    assets/special_days/anewyear/anewyear_1.c
    assets/special_days/anewyear/anewyear_2.c
    assets/special_days/anewyear/anewyear_3.c
    assets/special_days/anewyear/anewyear_title.c
    assets/special_days/anewyear/anewyear_button.c

    assets/special_days/halloween/halloween_1.c
    assets/special_days/halloween/halloween_2.c
    assets/special_days/halloween/halloween_3.c
    assets/special_days/halloween/halloween_4.c
    assets/special_days/halloween/halloween_title.c
    assets/special_days/halloween/halloween_button.c

    assets/special_days/mb/mb.c
    assets/special_days/mb/mb_title.c
    assets/special_days/mb/mb_button.c

    assets/special_days/monthi/monthi_1.c
    assets/special_days/monthi/monthi_2.c
    assets/special_days/monthi/monthi_3.c
    assets/special_days/monthi/monthi_4.c
    assets/special_days/monthi/monthi_title.c
    assets/special_days/monthi/monthi_button.c

    assets/special_days/newyear/newyear_1.c
    assets/special_days/newyear/newyear_2.c
    assets/special_days/newyear/newyear_3.c
    assets/special_days/newyear/newyear_title.c
    assets/special_days/newyear/newyear_button.c

    assets/special_days/val/val_1.c
    assets/special_days/val/val_2.c
    assets/special_days/val/val_3.c
    assets/special_days/val/val_4.c
    assets/special_days/val/val_title.c
    assets/special_days/val/val_button.c

    assets/special_days/xmas/xmas_1.c
    assets/special_days/xmas/xmas_2.c
    assets/special_days/xmas/xmas_3.c
    assets/special_days/xmas/xmas_4.c
    assets/special_days/xmas/xmas_title.c
    assets/special_days/xmas/xmas_button.c

    src/ui/theme/Theme.cpp

    ../core/src/game/GameManager.cpp
    ../core/src/quiz/QuizManager.cpp
    ../core/src/quiz/QuizLoader.cpp
    ../core/src/quiz/AnswerValidator.cpp
    ../core/src/quiz/QuizValidator.cpp
    ../core/src/quiz/FileQuizSource.cpp
    ../core/src/quiz/FileQuizProgressStore.cpp
    ../core/src/game/FileGameProgressStore.cpp
    ../core/src/app/StartupRouter.cpp

    ../core/src/event/SpecialEventCalendar.cpp
    ../core/src/event/SpecialEventManager.cpp
    
    ../firmware/src/dispenser/MockDispenser.cpp
)

target_include_directories(candy-simulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LVGL_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src

)

target_compile_definitions(candy-simulator PRIVATE
    SDL_MAIN_HANDLED
)

target_link_libraries(candy-simulator PRIVATE
    lvgl
    SDL2
)

# =====================================================
# SD CARD QUIZ SOURCE TEST
# =====================================================

add_executable(
    test-sd-quiz-source

    src/test_sd_quiz_source.cpp

    ../firmware/components/platform/SDCardQuizSource.cpp
)

target_include_directories(
    test-sd-quiz-source
    PRIVATE

    ../core/src
    ../firmware/components/platform
)

# =====================================================
# SPECIAL EVENT LOGIC TEST
# =====================================================

add_executable(
    test-special-event

    src/test_special_event.cpp

    ../core/src/event/SpecialEventCalendar.cpp
    ../core/src/event/SpecialEventManager.cpp

    ../firmware/src/dispenser/MockDispenser.cpp
)

target_include_directories(
    test-special-event
    PRIVATE

    ../core/src
    ../firmware/src
    src
)

add_executable(
    quiz-dataset-test

    ../core/tests/ProductionDatasetIntegrationTest.cpp

    ../core/src/quiz/FileQuizSource.cpp
    ../core/src/quiz/QuizLoader.cpp
    ../core/src/quiz/QuizValidator.cpp
    
)

target_include_directories(
    quiz-dataset-test

    PRIVATE

    ../core/src
    ../core/src/quiz
)
