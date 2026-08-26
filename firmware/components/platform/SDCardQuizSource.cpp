#include "SDCardQuizSource.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>


// =====================================================
// CONSTRUCTOR
// =====================================================

SDCardQuizSource::SDCardQuizSource(
    const std::string& filePath
)
    : filePath(filePath)
{
    buildIndex();
}


// =====================================================
// SIZE
// =====================================================

std::size_t SDCardQuizSource::size() const
{
    return offsets.size();
}


// =====================================================
// BUILD INDEX
// =====================================================
//
// Scan the JSONL file once.
//
// We do NOT load every quiz into RAM.
//
// Instead we remember:
//
// quiz 0 → byte offset 0
// quiz 1 → byte offset 182
// quiz 2 → byte offset 397
// ...
//
// Later getQuiz(index) can jump directly
// to the required JSON line.
//
// =====================================================

void SDCardQuizSource::buildIndex()
{
    FILE* file =
        std::fopen(
            filePath.c_str(),
            "rb"
        );


    if (file == nullptr) {
        throw std::runtime_error(
            "Could not open quiz dataset: "
            + filePath
        );
    }


    offsets.clear();


    while (true) {

        long position =
            std::ftell(file);


        if (position < 0) {

            std::fclose(file);

            throw std::runtime_error(
                "Failed to read quiz file position."
            );
        }


        int firstChar =
            std::fgetc(file);


        // End of file.
        if (firstChar == EOF) {
            break;
        }


        // Ignore blank lines.
        if (
            firstChar == '\n' ||
            firstChar == '\r'
        ) {
            continue;
        }


        // This is the beginning of a quiz line.
        offsets.push_back(
            position
        );


        // Skip remainder of this line.
        int c;

        while (
            (c = std::fgetc(file))
            != EOF
        ) {

            if (c == '\n') {
                break;
            }
        }


        if (c == EOF) {
            break;
        }
    }


    std::fclose(file);


    if (offsets.empty()) {
        throw std::runtime_error(
            "Quiz dataset contains no questions."
        );
    }
}


// =====================================================
// GET QUIZ
// =====================================================

Quiz SDCardQuizSource::getQuiz(
    std::size_t index
)
{
    if (
        index >= offsets.size()
    ) {
        throw std::runtime_error(
            "Quiz index out of range."
        );
    }


    FILE* file =
        std::fopen(
            filePath.c_str(),
            "rb"
        );


    if (file == nullptr) {
        throw std::runtime_error(
            "Could not open quiz dataset."
        );
    }


    if (
        std::fseek(
            file,
            offsets[index],
            SEEK_SET
        )
        != 0
    ) {

        std::fclose(file);

        throw std::runtime_error(
            "Failed to seek quiz dataset."
        );
    }


    std::string line;

    int c;


    while (
        (c = std::fgetc(file))
        != EOF
    ) {

        if (c == '\n') {
            break;
        }


        if (c != '\r') {
            line.push_back(
                static_cast<char>(c)
            );
        }
    }


    std::fclose(file);


    if (line.empty()) {
        throw std::runtime_error(
            "Quiz line is empty."
        );
    }


    return parseLine(
        line
    );
}


// =====================================================
// JSON HELPERS
// =====================================================

namespace {

std::string extractString(
    const std::string& line,
    const std::string& key
)
{
    std::string keyToken =
        "\"" + key + "\"";


    std::size_t position =
        line.find(keyToken);


    if (
        position == std::string::npos
    ) {
        throw std::runtime_error(
            "Missing JSON string field: "
            + key
        );
    }


    position +=
        keyToken.length();


    // Skip whitespace.
    while (
        position < line.size() &&
        (
            line[position] == ' ' ||
            line[position] == '\t'
        )
    ) {
        ++position;
    }


    if (
        position >= line.size() ||
        line[position] != ':'
    ) {
        throw std::runtime_error(
            "Missing ':' after string field: "
            + key
        );
    }


    ++position;


    // Skip whitespace.
    while (
        position < line.size() &&
        (
            line[position] == ' ' ||
            line[position] == '\t'
        )
    ) {
        ++position;
    }


    if (
        position >= line.size() ||
        line[position] != '"'
    ) {
        throw std::runtime_error(
            "Expected string value for field: "
            + key
        );
    }


    ++position;


    std::string result;

    bool escaped = false;


    for (
        std::size_t i = position;
        i < line.size();
        ++i
    ) {
        char ch =
            line[i];


        if (escaped) {

            switch (ch) {
                case '"':
                    result += '"';
                    break;

                case '\\':
                    result += '\\';
                    break;

                case 'n':
                    result += '\n';
                    break;

                case 'r':
                    result += '\r';
                    break;

                case 't':
                    result += '\t';
                    break;

                default:
                    result += ch;
                    break;
            }


            escaped = false;

            continue;
        }


        if (ch == '\\') {

            escaped = true;

            continue;
        }


        if (ch == '"') {
            return result;
        }


        result += ch;
    }


    throw std::runtime_error(
        "Unterminated JSON string field: "
        + key
    );
}


int extractInt(
    const std::string& line,
    const std::string& key
)
{
    // Find:
    //
    // "id"
    //
    // instead of requiring exactly:
    //
    // "id":
    //
    std::string keyToken =
        "\"" + key + "\"";


    std::size_t position =
        line.find(keyToken);


    if (
        position == std::string::npos
    ) {
        throw std::runtime_error(
            "Missing JSON integer field: "
            + key
        );
    }


    position +=
        keyToken.length();


    // Skip whitespace.
    while (
        position < line.size() &&
        (
            line[position] == ' ' ||
            line[position] == '\t'
        )
    ) {
        ++position;
    }


    // Expect colon.
    if (
        position >= line.size() ||
        line[position] != ':'
    ) {
        throw std::runtime_error(
            "Missing ':' after integer field: "
            + key
        );
    }


    ++position;


    // Skip whitespace after colon.
    while (
        position < line.size() &&
        (
            line[position] == ' ' ||
            line[position] == '\t'
        )
    ) {
        ++position;
    }


    std::size_t end =
        position;


    if (
        end < line.size() &&
        line[end] == '-'
    ) {
        ++end;
    }


    while (
        end < line.size() &&
        line[end] >= '0' &&
        line[end] <= '9'
    ) {
        ++end;
    }


    if (
        end == position
    ) {
        throw std::runtime_error(
            "Invalid JSON integer field: "
            + key
        );
    }


    return std::atoi(
        line.substr(
            position,
            end - position
        ).c_str()
    );
}

std::vector<std::string>
extractStringArray(
    const std::string& line,
    const std::string& key
)
{
    std::string search =
        "\"" + key + "\":[";


    std::size_t start =
        line.find(search);


    if (
        start == std::string::npos
    ) {
        throw std::runtime_error(
            "Missing JSON array field: "
            + key
        );
    }


    start +=
        search.length();


    std::vector<std::string>
        values;


    std::size_t i =
        start;


    while (
        i < line.size()
    ) {

        // Array finished.
        if (line[i] == ']') {
            return values;
        }


        // Ignore commas / whitespace.
        if (
            line[i] == ',' ||
            line[i] == ' '
        ) {
            ++i;
            continue;
        }


        if (line[i] != '"') {
            throw std::runtime_error(
                "Invalid string array."
            );
        }


        ++i;


        std::string value;

        bool escaped = false;


        while (
            i < line.size()
        ) {

            char ch =
                line[i++];


            if (escaped) {

                switch (ch) {

                    case '"':
                        value += '"';
                        break;

                    case '\\':
                        value += '\\';
                        break;

                    case 'n':
                        value += '\n';
                        break;

                    case 'r':
                        value += '\r';
                        break;

                    case 't':
                        value += '\t';
                        break;

                    default:
                        value += ch;
                        break;
                }

                escaped = false;

                continue;
            }


            if (ch == '\\') {

                escaped = true;

                continue;
            }


            if (ch == '"') {
                break;
            }


            value += ch;
        }


        values.push_back(
            value
        );
    }


    throw std::runtime_error(
        "Unterminated JSON array."
    );
}

} // namespace


// =====================================================
// PARSE ONE QUIZ
// =====================================================

Quiz SDCardQuizSource::parseLine(
    const std::string& line
)
{
    Quiz quiz;


    // =================================================
    // ID
    // =================================================

    quiz.id =
        extractInt(
            line,
            "id"
        );


    // =================================================
    // CATEGORY
    // =================================================

    quiz.category =
        extractString(
            line,
            "category"
        );


    // =================================================
    // TYPE
    // =================================================

    std::string type =
        extractString(
            line,
            "type"
        );


    if (
        type == "numeric"
    ) {

        quiz.type =
            QuizType::NUMERIC;
    }

    else if (
        type == "multiple_choice"
    ) {

        quiz.type =
            QuizType::MULTIPLE_CHOICE;
    }

    else {

        throw std::runtime_error(
            "Unsupported quiz type: "
            + type
        );
    }


    // =================================================
    // DIFFICULTY
    // =================================================

    quiz.difficulty =
        extractInt(
            line,
            "difficulty"
        );


    // =================================================
    // QUESTION
    // =================================================

    quiz.question =
        extractString(
            line,
            "question"
        );


    // =================================================
    // OPTIONS
    // =================================================

    quiz.options =
        extractStringArray(
            line,
            "options"
        );


    // =================================================
    // ANSWER
    // =================================================

    quiz.answer =
        extractString(
            line,
            "answer"
        );


    // =================================================
    // BASIC VALIDATION
    // =================================================

    if (
        quiz.type
        == QuizType::MULTIPLE_CHOICE
    ) {

        if (
            quiz.options.size()
            != 4
        ) {
            throw std::runtime_error(
                "Multiple choice quiz must have exactly 4 options."
            );
        }


        if (
            quiz.answer != "0" &&
            quiz.answer != "1" &&
            quiz.answer != "2" &&
            quiz.answer != "3"
        ) {
            throw std::runtime_error(
                "Multiple choice answer must be 0, 1, 2 or 3."
            );
        }
    }

    else {

        if (
            !quiz.options.empty()
        ) {
            throw std::runtime_error(
                "Numeric quiz must not contain options."
            );
        }
    }


    return quiz;
}