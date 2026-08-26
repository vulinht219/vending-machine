#pragma once

class SDCardManager {
public:
    static bool mount();

    static void unmount();

    static bool isMounted();

private:
    static bool mounted;
};