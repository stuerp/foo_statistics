
/** $VER: StatisticsManager.h (2024.07.17) **/

#include "pch.h"

#include <SDK/file_info_impl.h>

#include <vector>
#include <set>

#include "Configuration.h"

#pragma once

class StatisticsManager
{
public:
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
                Timestamps.resize(1);

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
            return (Timestamps.size() > 2) ? Timestamps.back() : 0;
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
            char * Text = ::_strdup(s);

            char * Context = nullptr;

            char * Token = ::strtok_s(Text, ";", &Context);

            while (Token != nullptr)
            {
                auto Timestamp = (uint64_t) ::_atoi64(Token);

                if (errno != ERANGE)
                    Timestamps.push_back(Timestamp);

                Token = ::strtok_s(nullptr, ";", &Context);
            }

            ::free(Text);
        }
    };

public:
    /// <summary>
    /// Gets the system time as a Windows file time.
    /// </summary>
    static uint64_t Now() noexcept
    {
        return pfc::fileTimeNow();
    }

    static metadb_index_manager_v2::ptr GetMetaDbIndexManager() noexcept;

    static void OnItemPlayed(const metadb_handle_ptr & hTrack) noexcept;

    static statistics_t GetStatistics(metadb_index_hash hash) noexcept;
    static void SetStatistics(metadb_index_hash hash, const statistics_t & statistics, const metadb_index_transaction::ptr & transaction) noexcept;

    static hash_set_t GetHashes(metadb_handle_list_cref hTracks) noexcept;

    static pfc::string TimestampToText(uint64_t timestamp);
    
    static void Reset(metadb_handle_list_cref hTracks) noexcept;
    static void Write(metadb_handle_list_cref hTracks) noexcept;
    static void Read(metadb_handle_list_cref hTracks) noexcept;
    static void Migrate(metadb_handle_list_cref hTracks) noexcept;

    static void SetRating(metadb_handle_list_cref hTracks, uint32_t rating) noexcept;

    static void Refresh(const hash_list_t & hashList);

private:
    static void Process(metadb_handle_list_cref hTracks, std::function<void (statistics_t & s)> callback) noexcept;

    static int64_t GetNumber(file_info_impl & fileInfo, const char * name) noexcept
    {
        const char * Value = fileInfo.meta_get(name, 0);

        if (Value == nullptr)
            return 0;

        int64_t Number = ::_atoi64(Value);

        return (errno != ERANGE) ? Number : 0;
    }
};
