
/** $VER: configuration_t.cpp (2024.07.16) P. Stuer **/

#include "pch.h"

#include "Configuration.h"
#include "Encoding.h"
#include "Resources.h"

#include <SDK/file.h>
#include <SDK/advconfig_impl.h>

#include <pfc/string_conv.h>
#include <pfc/string-conv-lite.h>

using namespace pfc;
using namespace stringcvt;

#include <pathcch.h>
#pragma comment(lib, "pathcch")

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
}

/// <summary>
/// Implements the = operator.
/// </summary>
configuration_t & configuration_t::operator=(const configuration_t & other)
{
 
    return *this;
}

/// <summary>
/// Reads this instance with the specified reader.
/// </summary>
void configuration_t::Read(stream_reader * reader, size_t size, abort_callback & abortHandler) noexcept
{
    Reset();

    try
    {
        int32_t Version;

        reader->read(&Version, sizeof(Version), abortHandler);

    }
    catch (exception & ex)
    {
        console::printf(STR_COMPONENT_BASENAME " failed to read configuration: %s", ex.what());

        Reset();
    }
}

/// <summary>
/// Writes this instance to the specified writer.
/// </summary>
void configuration_t::Write(stream_writer * writer, abort_callback & abortHandler) const noexcept
{
    try
    {
        writer->write_object_t(_CurrentVersion, abortHandler);

    }
    catch (exception & ex)
    {
        console::printf(STR_COMPONENT_BASENAME " failed to write configuration: %s", ex.what());
    }
}

configuration_t _Configuration;
