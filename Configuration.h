
/** $VER: Configuration.h (2024.07.17) P. Stuer **/

#pragma once

#include "pch.h"

#include <SDK/system_time_keeper.h>

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

    void Read(stream_reader * reader, size_t size, abort_callback & abortHandler = fb2k::noAbort) noexcept;
    void Write(stream_writer * writer, abort_callback & abortHandler = fb2k::noAbort) const noexcept;

public:
    bool LegacyMode = false;
    t_filetimestamp RetentionPeriod = system_time_periods::week * 4;

    pfc::string PinTo = "%album artist%|%album%|%subtitle%|%publisher%|%album country%|%album released%|%album recorded%|%tracknumber%|%title%|%featuring%|%remix%|%artist%|%date%"; // "%path%|%subsong%"

    double TargetTime = 30.; // seconds
    double TargetPercentage = 0.9; // of total track length

    bool _StatisticsUpdated = false; // True if the statistics for the playing item has been updated.

private:
    const int32_t _CurrentVersion = 1;
};

extern configuration_t _Configuration;
