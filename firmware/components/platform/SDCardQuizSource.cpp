#include "SDCardQuizSource.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "esp_log.h"
#include "esp_timer.h"


namespace {

constexpr const char* TAG =
    "SDCardQuizSource";

}


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
// quiz 0 -> byte offset 0
// quiz 1 -> byte offset ...
// quiz 2 -> byte offset ...
//
// Later getQuiz(index) jumps directly to the
// corresponding JSONL line.
//
// =====================================================

void SDCardQuizSource::buildIndex()
{
    ESP_LOGI(
        TAG,
        "Index build started"
    );


    const int64_t startTime =
        esp_timer_get_time();


    FILE* file =
        std::fopen(
            filePath.c_str(),
            "rb"
        );


    if (
        file == nullptr
    )
    {
        throw std::runtime_error(
            "Could not open quiz dataset: "
            + filePath
        );
    }


    offsets.clear();


    // Production dataset has 20,000 quizzes.
    //
    // Reserve capacity up front to avoid repeated
    // vector reallocations while indexing.
    offsets.reserve(
        20000
    );


    // =================================================
    // BUFFERED SCAN
    // =================================================
    //
    // Read large blocks instead of calling fgetc()
    // millions of times.
    //

    constexpr std::size_t BUFFER_SIZE =
        16 * 1024;


    // Static buffer so it does not consume task stack.
    static unsigned char buffer[
        BUFFER_SIZE
    ];


    long absoluteOffset =
        0;


    bool lookingForLineStart =
        true;


    while (
        true
    )
    {
        const std::size_t bytesRead =
            std::fread(
                buffer,
                1,
                BUFFER_SIZE,
                file
            );


        if (
            bytesRead == 0
        )
        {
            if (
                std::ferror(
                    file
                )
            )
            {
                std::fclose(
                    file
                );


                throw std::runtime_error(
                    "Failed while reading quiz dataset."
                );
            }


            break;
        }


        for (
            std::size_t i = 0;
            i < bytesRead;
            ++i
        )
        {
            const unsigned char c =
                buffer[i];


            const long currentOffset =
                absoluteOffset
                +
                static_cast<long>(
                    i
                );


            if (
                lookingForLineStart
            )
            {
                // Ignore blank CR/LF lines.
                if (
                    c == '\n'
                    ||
                    c == '\r'
                )
                {
                    continue;
                }


                // First byte of a new JSONL record.
                offsets.push_back(
                    currentOffset
                );


                lookingForLineStart =
                    false;
            }


            if (
                c == '\n'
            )
            {
                lookingForLineStart =
                    true;
            }
        }


        absoluteOffset +=
            static_cast<long>(
                bytesRead
            );
    }


    std::fclose(
        file
    );


    if (
        offsets.empty()
    )
    {
        throw std::runtime_error(
            "Quiz dataset contains no questions."
        );
    }


    const int64_t endTime =
        esp_timer_get_time();


    const long long elapsedMs =
        static_cast<long long>(
            (
                endTime
                -
                startTime
            )
            /
            1000
        );


    ESP_LOGI(
        TAG,
        "Index build finished: %lld ms",
        elapsedMs
    );


    ESP_LOGI(
        TAG,
        "Indexed quizzes: %u",
        static_cast<unsigned>(
            offsets.size()
        )
    );
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
    )
    {
        throw std::runtime_error(
            "Quiz index out of range."
        );
    }


    FILE* file =
        std::fopen(
            filePath.c_str(),
            "rb"
        );


    if (
        file == nullptr
    )
    {
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
    )
    {
        std::fclose(
            file
        );


        throw std::runtime_error(
            "Failed to seek quiz dataset."
        );
    }


    std::string line;


    int c;


    while (
        (c = std::fgetc(file))
        != EOF
    )
    {
        if (
            c == '\n'
        )
        {
            break;
        }


        if (
            c != '\r'
        )
        {
            line.push_back(
                static_cast<char>(
                    c
                )
            );
        }
    }


    std::fclose(
        file
    );


    if (
        line.empty()
    )
    {
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
        "\""
        + key
        + "\"";


    std::size_t position =
        line.find(
            keyToken
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        throw std::runtime_error(
            "Missing JSON string field: "
            + key
        );
    }


    position +=
        keyToken.length();


    while (
        position < line.size()
        &&
        (
            line[position] == ' '
            ||
            line[position] == '\t'
        )
    )
    {
        ++position;
    }


    if (
        position >= line.size()
        ||
        line[position] != ':'
    )
    {
        throw std::runtime_error(
            "Missing ':' after string field: "
            + key
        );
    }


    ++position;


    while (
        position < line.size()
        &&
        (
            line[position] == ' '
            ||
            line[position] == '\t'
        )
    )
    {
        ++position;
    }


    if (
        position >= line.size()
        ||
        line[position] != '"'
    )
    {
        throw std::runtime_error(
            "Expected string value for field: "
            + key
        );
    }


    ++position;


    std::string result;


    bool escaped =
        false;


    for (
        std::size_t i = position;
        i < line.size();
        ++i
    )
    {
        char ch =
            line[i];


        if (
            escaped
        )
        {
            switch (
                ch
            )
            {
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


            escaped =
                false;


            continue;
        }


        if (
            ch == '\\'
        )
        {
            escaped =
                true;


            continue;
        }


        if (
            ch == '"'
        )
        {
            return result;
        }


        result +=
            ch;
    }


    throw std::runtime_error(
        "Unterminated JSON string field: "
        + key
    );
}


// =====================================================
// INTEGER
// =====================================================

int extractInt(
    const std::string& line,
    const std::string& key
)
{
    std::string keyToken =
        "\""
        + key
        + "\"";


    std::size_t position =
        line.find(
            keyToken
        );


    if (
        position
        ==
        std::string::npos
    )
    {
        throw std::runtime_error(
            "Missing JSON integer field: "
            + key
        );
    }


    position +=
        keyToken.length();


    while (
        position < line.size()
        &&
        (
            line[position] == ' '
            ||
            line[position] == '\t'
        )
    )
    {
        ++position;
    }


    if (
        position >= line.size()
        ||
        line[position] != ':'
    )
    {
        throw std::runtime_error(
            "Missing ':' after integer field: "
            + key
        );
    }


    ++position;


    while (
        position < line.size()
        &&
        (
            line[position] == ' '
            ||
            line[position] == '\t'
        )
    )
    {
        ++position;
    }


    std::size_t end =
        position;


    if (
        end < line.size()
        &&
        line[end] == '-'
    )
    {
        ++end;
    }


    while (
        end < line.size()
        &&
        line[end] >= '0'
        &&
        line[end] <= '9'
    )
    {
        ++end;
    }


    if (
        end == position
    )
    {
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


// =====================================================
// STRING ARRAY
// =====================================================

std::vector<std::string>
extractStringArray(
    const std::string& line,
    const std::string& key
)
{
    std::string search =
        "\""
        + key
        + "\":[";


    std::size_t start =
        line.find(
            search
        );


    if (
        start
        ==
        std::string::npos
    )
    {
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
    )
    {
        if (
            line[i] == ']'
        )
        {
            return values;
        }


        if (
            line[i] == ','
            ||
            line[i] == ' '
        )
        {
            ++i;


            continue;
        }


        if (
            line[i] != '"'
        )
        {
            throw std::runtime_error(
                "Invalid string array."
            );
        }


        ++i;


        std::string value;


        bool escaped =
            false;


        while (
            i < line.size()
        )
        {
            char ch =
                line[i++];


            if (
                escaped
            )
            {
                switch (
                    ch
                )
                {
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


                escaped =
                    false;


                continue;
            }


            if (
                ch == '\\'
            )
            {
                escaped =
                    true;


                continue;
            }


            if (
                ch == '"'
            )
            {
                break;
            }


            value +=
                ch;
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


    quiz.id =
        extractInt(
            line,
            "id"
        );


    quiz.category =
        extractString(
            line,
            "category"
        );


    std::string type =
        extractString(
            line,
            "type"
        );


    if (
        type == "numeric"
    )
    {
        quiz.type =
            QuizType::NUMERIC;
    }


    else if (
        type
        ==
        "multiple_choice"
    )
    {
        quiz.type =
            QuizType::MULTIPLE_CHOICE;
    }


    else
    {
        throw std::runtime_error(
            "Unsupported quiz type: "
            + type
        );
    }


    quiz.difficulty =
        extractInt(
            line,
            "difficulty"
        );


    quiz.question =
        extractString(
            line,
            "question"
        );


    quiz.options =
        extractStringArray(
            line,
            "options"
        );


    quiz.answer =
        extractString(
            line,
            "answer"
        );


    if (
        quiz.type
        ==
        QuizType::MULTIPLE_CHOICE
    )
    {
        if (
            quiz.options.size()
            != 4
        )
        {
            throw std::runtime_error(
                "Multiple choice quiz must have exactly 4 options."
            );
        }


        if (
            quiz.answer != "0"
            &&
            quiz.answer != "1"
            &&
            quiz.answer != "2"
            &&
            quiz.answer != "3"
        )
        {
            throw std::runtime_error(
                "Multiple choice answer must be 0, 1, 2 or 3."
            );
        }
    }


    else
    {
        if (
            !quiz.options.empty()
        )
        {
            throw std::runtime_error(
                "Numeric quiz must not contain options."
            );
        }
    }


    return quiz;
}