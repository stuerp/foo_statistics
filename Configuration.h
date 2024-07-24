
/** $VER: Configuration.h (2024.07.24) P. Stuer **/

#pragma once

#include "pch.h"

#include <SDK/titleformat.h>
#include <SDK/system_time_keeper.h>
#include <SDK/file.h>

enum RetentionUnit : uint32_t
{
    Seconds = 0,
    Minutes = 1,
    Hours = 2,
    Days = 3,
    Weeks = 4,

    Count,
};

enum WriteToTags : uint32_t
{
    Never = 0,
    Always = ~0U
};

/// <summary>
/// Represents the configuration of the component.
/// </summary>
class configuration_t
{
public:
    configuration_t();

    configuration_t & operator=(const configuration_t & other);

    virtual ~configuration_t() { }

    void Reset() noexcept;

    void Read() noexcept;
    void Write() const noexcept;

    double GetThresholdTime() noexcept;
    t_filetimestamp GetRetentionPeriod() const noexcept;

private:
    void ParseTagsToRemove() noexcept;

public:
    pfc::string _PinTo;
    pfc::string _ThresholdFormat;

    uint32_t _RetentionValue;
    RetentionUnit _RetentionUnit;

    WriteToTags _WriteToTags = WriteToTags::Always;
    bool _RemoveTags;
    pfc::string _TagsToRemove;

    std::vector<pfc::string> _TagsToRemoveList;

private:
    titleformat_object::ptr _ThresholdScript;

    const int32_t _CurrentVersion = 1;
};

extern configuration_t _Configuration;
