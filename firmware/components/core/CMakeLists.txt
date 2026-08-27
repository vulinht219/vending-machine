idf_component_register(
    SRCS
        "../../../core/src/game/GameManager.cpp"

        "../../../core/src/quiz/QuizManager.cpp"
        "../../../core/src/quiz/AnswerValidator.cpp"
        "../../../core/src/quiz/QuizValidator.cpp"
        
        "../../../core/src/event/SpecialEventCalendar.cpp"
        "../../../core/src/event/SpecialEventManager.cpp"
        "../../../core/src/app/StartupRouter.cpp"

    INCLUDE_DIRS
        "../../../core/src"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)