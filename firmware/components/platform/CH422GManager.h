#pragma once

#include <cstdint>


class CH422GManager
{
public:

    // Initialize CH422G and its shared output state.
    static bool initialize();


    // Set one EXIO output bit while preserving all
    // remaining CH422G outputs.
    static bool setOutput(
        int exio,
        bool high
    );


    // Write a complete output state.
    //
    // Mainly useful for board-specific initialization.
    static bool setOutputState(
        uint8_t state
    );


    // Current cached CH422G output state.
    static uint8_t getOutputState();


    static bool isInitialized();


private:

    static bool initialized;

    static uint8_t outputState;


    static bool writeOutputState();
};