
/** $VER: Statistics.h (2024.07.24) **/

#include "pch.h"

#include <vector>

#pragma once

using timestamps_t = std::vector<uint64_t>;

struct statistics_t
{
    statistics_t() : Rating()
    {
    }

    timestamps_t Timestamps;    // Timestamps of each time the track has been played.
    uint32_t Rating;            // Rating of the item

    void Reset() noexcept
    {
        if (Timestamps.size() > 1)
            Timestamps.resize(1); // Keep the Added timestamp.

        Rating = 0;
    }

    void SetAddedTimestamp(uint64_t timestamp) noexcept
    {
        if (Timestamps.size() > 0)
            Timestamps[0] = timestamp;
        else
            Timestamps.push_back(timestamp);
    }

    uint64_t GetAddedTimestamp() const noexcept
    {
        return (Timestamps.size() > 0) ? Timestamps.front() : 0;
    }

    uint64_t GetFirstPlayedTimestamp() const noexcept
    {
        return (Timestamps.size() > 1) ? Timestamps[1] : 0;
    }

    uint64_t GetLastPlayedTimestamp() const noexcept
    {
        return (Timestamps.size() > 2) ? Timestamps.back() : GetFirstPlayedTimestamp();
    }

    uint32_t GetPlaycount() const noexcept
    {
        return (Timestamps.size() > 1) ? (uint32_t) Timestamps.size() - 1 : 0;
    }

    /// <summary>
    /// Gets the timestamps as a string.
    /// </summary>
    pfc::string GetTimestamps() const noexcept
    {
        pfc::string s;

        for (const auto & Timestamp : Timestamps)
        {
            if (!s.empty())
                s += ";";

            s += pfc::format_uint(Timestamp);
        }

        return s;
    }

    /// <summary>
    /// Sets the timestamps from a string.
    /// </summary>
    void SetTimestamps(const char * s) noexcept
    {
        const char * Delimiters = "; ";

        char * Text = ::_strdup(s);

        if (Text == nullptr)
            return;

        char * Context = nullptr;

        char * Token = ::strtok_s(Text, Delimiters, &Context);

        while (Token != nullptr)
        {
            auto Timestamp = (uint64_t) ::_atoi64(Token);

            if (errno != ERANGE)
                Timestamps.push_back(Timestamp);

            Token = ::strtok_s(nullptr, Delimiters, &Context);
        }

        ::free(Text);
    }
};
