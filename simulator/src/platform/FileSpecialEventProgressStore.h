#pragma once

#include "event/ISpecialEventProgressStore.h"

#include <string>


class FileSpecialEventProgressStore
    : public ISpecialEventProgressStore
{
public:

    explicit FileSpecialEventProgressStore(
        const std::string& filePath
    );


    bool exists() const override;


    SpecialEventProgress load() const override;


    void save(
        const SpecialEventProgress& progress
    ) const override;


private:

    std::string filePath;
};