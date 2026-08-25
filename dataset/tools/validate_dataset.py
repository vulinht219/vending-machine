import json
import sys
from collections import Counter
from pathlib import Path


ALLOWED_CATEGORIES = {
    "math",
    "physics",
    "animals",
    "science",
    "world_history",
    "dad_jokes",
}


TARGET_COUNTS = {
    "math": 4500,
    "physics": 3000,
    "animals": 3000,
    "science": 4000,
    "world_history": 3000,
    "dad_jokes": 2500,
}


ALLOWED_TYPES = {
    "multiple_choice",
    "numeric",
}


ALLOWED_DIFFICULTIES = {
    1,
    2,
    3,
}


def normalize_question(text: str) -> str:
    """
    Normalize question text so obvious duplicates such as:

    "Why do dogs pant?"
    " why do dogs pant? "

    are treated as the same question.
    """

    return " ".join(
        text.lower().strip().split()
    )


def validate_quiz(quiz: dict, line_number: int):
    errors = []

    # =================================================
    # REQUIRED FIELDS
    # =================================================

    required_fields = {
        "id",
        "category",
        "type",
        "difficulty",
        "question",
        "options",
        "answer",
    }

    missing = required_fields - quiz.keys()

    if missing:
        errors.append(
            f"Line {line_number}: missing fields {sorted(missing)}"
        )

        return errors


    # =================================================
    # ID
    # =================================================

    if not isinstance(quiz["id"], int):
        errors.append(
            f"Line {line_number}: id must be integer"
        )

    elif quiz["id"] <= 0:
        errors.append(
            f"Line {line_number}: id must be > 0"
        )


    # =================================================
    # CATEGORY
    # =================================================

    category = quiz["category"]

    if category not in ALLOWED_CATEGORIES:
        errors.append(
            f"Line {line_number}: invalid category '{category}'"
        )


    # =================================================
    # TYPE
    # =================================================

    quiz_type = quiz["type"]

    if quiz_type not in ALLOWED_TYPES:
        errors.append(
            f"Line {line_number}: invalid type '{quiz_type}'"
        )


    # =================================================
    # DIFFICULTY
    # =================================================

    difficulty = quiz["difficulty"]

    if difficulty not in ALLOWED_DIFFICULTIES:
        errors.append(
            f"Line {line_number}: difficulty must be 1, 2 or 3"
        )


    # =================================================
    # QUESTION
    # =================================================

    question = quiz["question"]

    if not isinstance(question, str):
        errors.append(
            f"Line {line_number}: question must be string"
        )

    elif not question.strip():
        errors.append(
            f"Line {line_number}: question cannot be empty"
        )


    # =================================================
    # ANSWER
    # =================================================

    answer = quiz["answer"]

    if not isinstance(answer, str):
        errors.append(
            f"Line {line_number}: answer must be string"
        )


    # =================================================
    # OPTIONS
    # =================================================

    options = quiz["options"]

    if not isinstance(options, list):
        errors.append(
            f"Line {line_number}: options must be an array"
        )

        return errors


    # =================================================
    # MULTIPLE CHOICE
    # =================================================

    if quiz_type == "multiple_choice":

        if len(options) != 4:
            errors.append(
                f"Line {line_number}: multiple_choice must have exactly 4 options"
            )

        for index, option in enumerate(options):

            if not isinstance(option, str):
                errors.append(
                    f"Line {line_number}: option {index} must be string"
                )

            elif not option.strip():
                errors.append(
                    f"Line {line_number}: option {index} cannot be empty"
                )

        if answer not in {
            "0",
            "1",
            "2",
            "3",
        }:
            errors.append(
                f"Line {line_number}: multiple_choice answer must be 0-3"
            )


    # =================================================
    # NUMERIC
    # =================================================

    elif quiz_type == "numeric":

        if category != "math":
            errors.append(
                f"Line {line_number}: numeric questions are only allowed for math"
            )

        if len(options) != 0:
            errors.append(
                f"Line {line_number}: numeric question options must be []"
            )

        if not answer.strip():
            errors.append(
                f"Line {line_number}: numeric answer cannot be empty"
            )

        # Verify it is actually numeric
        try:
            float(answer)

        except ValueError:
            errors.append(
                f"Line {line_number}: numeric answer '{answer}' is not numeric"
            )


    return errors


def load_dataset(path: Path):
    quizzes = []
    errors = []

    with path.open(
        "r",
        encoding="utf-8",
    ) as file:

        for line_number, line in enumerate(
            file,
            start=1,
        ):

            line = line.strip()

            if not line:
                continue

            try:
                quiz = json.loads(line)

            except json.JSONDecodeError as error:

                errors.append(
                    f"Line {line_number}: invalid JSON: {error}"
                )

                continue


            if not isinstance(quiz, dict):

                errors.append(
                    f"Line {line_number}: JSON value must be an object"
                )

                continue


            quizzes.append(
                (
                    line_number,
                    quiz,
                )
            )

    return quizzes, errors


def validate_dataset(path: Path):
    quizzes, errors = load_dataset(path)

    ids = set()

    normalized_questions = set()

    categories = Counter()

    difficulties = Counter()

    types = Counter()

    math_types = Counter()


    # =================================================
    # VALIDATE EACH QUESTION
    # =================================================

    for line_number, quiz in quizzes:

        quiz_errors = validate_quiz(
            quiz,
            line_number,
        )

        errors.extend(
            quiz_errors
        )


        # =============================================
        # DUPLICATE ID
        # =============================================

        quiz_id = quiz.get("id")

        if isinstance(
            quiz_id,
            int,
        ):

            if quiz_id in ids:

                errors.append(
                    f"Line {line_number}: duplicate id {quiz_id}"
                )

            ids.add(
                quiz_id
            )


        # =============================================
        # DUPLICATE QUESTION
        # =============================================

        question = quiz.get(
            "question"
        )

        if isinstance(
            question,
            str,
        ):

            normalized = normalize_question(
                question
            )

            if normalized in normalized_questions:

                errors.append(
                    f"Line {line_number}: duplicate question: {question}"
                )

            normalized_questions.add(
                normalized
            )


        # =============================================
        # STATISTICS
        # =============================================

        category = quiz.get(
            "category"
        )

        if category in ALLOWED_CATEGORIES:

            categories[
                category
            ] += 1


        difficulty = quiz.get(
            "difficulty"
        )

        if difficulty in ALLOWED_DIFFICULTIES:

            difficulties[
                difficulty
            ] += 1


        quiz_type = quiz.get(
            "type"
        )

        if quiz_type in ALLOWED_TYPES:

            types[
                quiz_type
            ] += 1


        if category == "math":

            math_types[
                quiz_type
            ] += 1


    # =================================================
    # REPORT
    # =================================================

    total = len(quizzes)

    print()
    print("=" * 60)
    print("DATASET REPORT")
    print("=" * 60)

    print(
        f"Total questions: {total}"
    )

    print()
    print("Categories:")

    for category in TARGET_COUNTS:

        actual = categories[
            category
        ]

        target = TARGET_COUNTS[
            category
        ]

        print(
            f"  {category:15} {actual:6} / target {target}"
        )


    print()
    print("Question types:")

    for quiz_type, count in types.items():

        print(
            f"  {quiz_type:20} {count}"
        )


    print()
    print("Math types:")

    for quiz_type, count in math_types.items():

        print(
            f"  {quiz_type:20} {count}"
        )


    print()
    print("Difficulty:")

    for difficulty in [
        1,
        2,
        3,
    ]:

        print(
            f"  Level {difficulty}: {difficulties[difficulty]}"
        )


    # =================================================
    # RESULT
    # =================================================

    print()
    print("=" * 60)

    if errors:

        print(
            f"FAILED: {len(errors)} problem(s) found"
        )

        print("=" * 60)

        for error in errors:

            print(
                f"[ERROR] {error}"
            )

        return False


    print(
        "PASSED: dataset structure is valid"
    )

    print("=" * 60)

    return True


def main():
    if len(sys.argv) != 2:

        print(
            "Usage:"
        )

        print(
            "python validate_dataset.py <dataset.jsonl>"
        )

        sys.exit(1)


    path = Path(
        sys.argv[1]
    )


    if not path.exists():

        print(
            f"File not found: {path}"
        )

        sys.exit(1)


    valid = validate_dataset(
        path
    )


    sys.exit(
        0 if valid else 1
    )


if __name__ == "__main__":
    main()