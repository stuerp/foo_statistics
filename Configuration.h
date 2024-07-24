
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

enum RemoveOldTags : uint32_t
{
    None = 0,

    foo_playcount = 1,
    foo_playcount_2003 = 2,

    All = ~0U
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
    t_filetimestamp GetRetentionPeriod() noexcept;

public:
    pfc::string _PinTo = "%album artist%|%album%|%subtitle%|%publisher%|%album country%|%album released%|%album recorded%|%tracknumber%|%title%|%featuring%|%remix%|%artist%|%date%"; // "%path%|%subsong%"
    pfc::string _ThresholdFormat = "$if(%length_seconds%, $min($div(%length_seconds%, 2), 30),)";

    uint32_t _RetentionValue = 4;
    RetentionUnit _RetentionUnit = RetentionUnit::Weeks;

    WriteToTags _WriteToTags = WriteToTags::Always;
    RemoveOldTags _RemoveOldTags = RemoveOldTags::All;

    t_filetimestamp _RetentionPeriod = system_time_periods::week * 4;

private:
    titleformat_object::ptr _ThresholdScript;

    const int32_t _CurrentVersion = 1;
};

extern configuration_t _Configuration;
