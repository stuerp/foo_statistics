
/** $VER: configuration.cpp (2024.07.22) P. Stuer **/

#include "pch.h"

#include "Configuration.h"
#include "Encoding.h"
#include "Resources.h"

#include <SDK/file.h>
#include <SDK/advconfig_impl.h>
#include <SDK/playback_control.h>
#include <SDK/cfg_var.h>

#pragma hdrstop

static const GUID VersionGUID         = { 0x09d14d16, 0x3a37, 0x4691, { 0xbd, 0x5e, 0x2a, 0xf2, 0xee, 0xe4, 0x05, 0x64 } };
static const GUID PinToGUID           = { 0xd33b70dd, 0xfc88, 0x4397, { 0xa5, 0xac, 0xd0, 0x85, 0x6b, 0x07, 0xd5, 0x4f } };
static const GUID ThresholdFormatGUID = { 0xff7d2dd4, 0x6d86, 0x4361, { 0x88, 0x62, 0xb0, 0x55, 0x84, 0x60, 0x9d, 0x05 } };
static const GUID RetentionValueGUID  = { 0xd287b736, 0x6ac2, 0x49fc, { 0xa4, 0x6f, 0xbf, 0xac, 0x7e, 0x2e, 0xac, 0xc3 } };
static const GUID RetentionUnitGUID   = { 0xd337ea2c, 0xa918, 0x4771, { 0xb2, 0xcd, 0xf1, 0x50, 0x59, 0xf6, 0x04, 0x37 } };
static const GUID WriteToTagsGUID     = { 0x28d2ef5c, 0x6a42, 0x4a92, { 0xb8, 0xe5, 0xe1, 0x20, 0x28, 0xb7, 0xae, 0x03 } };

static cfg_var_modern::cfg_int      VersionCfg          (VersionGUID, 0);
static cfg_var_modern::cfg_string   PinToCfg            (PinToGUID, "");
static cfg_var_modern::cfg_string   ThresholdFormatCfg  (ThresholdFormatGUID, "");
static cfg_var_modern::cfg_int      RetentionValueCfg   (RetentionValueGUID, 0);
static cfg_var_modern::cfg_int      RetentionUnitCfg    (RetentionUnitGUID, 0);
static cfg_var_modern::cfg_int      WriteToTagsCfg      (WriteToTagsGUID, 0);

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

    _RetentionValue = 4;
    _RetentionUnit = RetentionUnit::Weeks;

    _WriteToTags = WriteToTags::Always;
}

/// <summary>
/// Implements the = operator.
/// </summary>
configuration_t & configuration_t::operator=(const configuration_t & other)
{
    _PinTo              = other._PinTo;
    _ThresholdFormat    = other._ThresholdFormat;

    _RetentionValue     = other._RetentionValue;
    _RetentionUnit      = other._RetentionUnit;

    _WriteToTags        = other._WriteToTags;

     return *this;
}

/// <summary>
/// Reads this instance with the specified reader.
/// </summary>
void configuration_t::Read() noexcept
{
    Reset();

    try
    {
        if (VersionCfg < _CurrentVersion)
            return;

        _PinTo           = PinToCfg;
        _ThresholdFormat = ThresholdFormatCfg;

        _RetentionValue  = (uint32_t) RetentionValueCfg;
        _RetentionUnit   = (RetentionUnit) (int) RetentionUnitCfg;

        _WriteToTags     = (WriteToTags) (int) WriteToTagsCfg;
    }
    catch (std::exception & ex)
    {
        console::printf(STR_COMPONENT_BASENAME " failed to read configuration: %s", ex.what());

        Reset();
    }
}

/// <summary>
/// Writes this instance to the specified writer.
/// </summary>
void configuration_t::Write() const noexcept
{
    try
    {
        VersionCfg         = _CurrentVersion;

        PinToCfg           = _PinTo;
        ThresholdFormatCfg = _ThresholdFormat;

        RetentionValueCfg  = _RetentionValue;
        RetentionUnitCfg   = _RetentionUnit;

        WriteToTagsCfg     = _WriteToTags;
    }
    catch (std::exception & ex)
    {
        console::printf(STR_COMPONENT_BASENAME " failed to write configuration: %s", ex.what());
    }
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
