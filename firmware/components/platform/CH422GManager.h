#pragma once

class CH422GManager {
public:
    static bool initialize();

    static bool setOutput(
        int exio,
        bool high
    );
};