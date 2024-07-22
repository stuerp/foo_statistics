
/** $VER: Configuration.h (2024.07.22) P. Stuer **/

#pragma once

#include "pch.h"

#include <SDK/titleformat.h>
#include <SDK/system_time_keeper.h>

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

    double GetThresholdTime() noexcept;
    t_filetimestamp GetRetentionPeriod() noexcept;

public:
    pfc::string _PinTo = "%album artist%|%album%|%subtitle%|%publisher%|%album country%|%album released%|%album recorded%|%tracknumber%|%title%|%featuring%|%remix%|%artist%|%date%"; // "%path%|%subsong%"
    pfc::string _ThresholdFormat = "$if(%length_seconds%, $min($div(%length_seconds%, 2), 30),)";

    double _RetentionValue = 4.;
    RetentionUnit _RetentionUnit = RetentionUnit::Weeks;

    WriteToTags _WriteToTags = WriteToTags::Always;

    t_filetimestamp _RetentionPeriod = system_time_periods::week * 4;

    bool _StatisticsUpdated = false; // True if the statistics for the playing item have been updated.

private:
    titleformat_object::ptr _ThresholdScript;

    const int32_t _CurrentVersion = 1;
};

extern configuration_t _Configuration;
