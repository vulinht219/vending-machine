#pragma once

class CH422GManager
{
public:
    static bool initialize();

    static bool setOutput(
        int exio,
        bool high
    );

private:
    static bool initialized;
    static unsigned char outputState;
};