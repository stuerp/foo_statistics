
/** $VER: configuration.cpp (2024.07.22) P. Stuer **/

#include "pch.h"

#include "Configuration.h"
#include "Encoding.h"
#include "Resources.h"

#include <SDK/file.h>
#include <SDK/advconfig_impl.h>
#include <SDK/playback_control.h>

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
configuration_t::configuration_t()
{
    Reset();
}

/// <summary>
/// Resets this instance.
/// </summary>
void configuration_t::Reset() noexcept
{
    _PinTo = "%album artist%|%album%|%subtitle%|%publisher%|%album country%|%album released%|%album recorded%|%tracknumber%|%title%|%featuring%|%remix%|%artist%|%date%"; // "%path%|%subsong%"
    _ThresholdFormat = "$if(%length_seconds%, $min($div(%length_seconds%, 2), 30),)";

    _RetentionValue = 4.;
    _RetentionUnit = RetentionUnit::Weeks;

    _WriteToTags = WriteToTags::Always;
}

/// <summary>
/// Implements the = operator.
/// </summary>
configuration_t & configuration_t::operator=(const configuration_t & other)
{
     return *this;
}

/// <summary>
/// Gets the threshold time after which a track is considered 'played' (in seconds).
/// </summary>
double configuration_t::GetThresholdTime() noexcept
{
    if (_ThresholdScript.is_empty())
        static_api_ptr_t<titleformat_compiler>()->compile_safe_ex(_ThresholdScript, _Configuration._ThresholdFormat);

    auto pbc = playback_control::get();

    pfc::string Result;

    if (!pbc->playback_format_title(nullptr, Result, _ThresholdScript, nullptr, playback_control::display_level_all))
        return 0.;

    return std::atof(Result);
}

/// <summary>
/// Gets the retention period.
/// </summary>
t_filetimestamp configuration_t::GetRetentionPeriod() noexcept
{
    #pragma warning(disable: 4061 4062)
    switch (_RetentionUnit)
    {
        case RetentionUnit::Seconds: return (t_filetimestamp)(_Configuration._RetentionValue * system_time_periods::second);
        case RetentionUnit::Minutes: return (t_filetimestamp)(_Configuration._RetentionValue * system_time_periods::minute);
        case RetentionUnit::Hours:   return (t_filetimestamp)(_Configuration._RetentionValue * system_time_periods::hour);
        case RetentionUnit::Days:    return (t_filetimestamp)(_Configuration._RetentionValue * system_time_periods::day);

        default:
        case RetentionUnit::Weeks:   return (t_filetimestamp)(_Configuration._RetentionValue * system_time_periods::week);
    }
}

configuration_t _Configuration;
