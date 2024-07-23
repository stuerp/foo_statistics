
/** $VER: StatisticsManager.h (2024.07.23) **/

#include "pch.h"

#include <SDK/file_info_impl.h>

#include "Configuration.h"
#include "Statistics.h"

#pragma once

class statistics_manager_t
{
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
    static void PutStatistics(metadb_index_hash hash, const statistics_t & statistics, const metadb_index_transaction::ptr & transaction) noexcept;

    static pfc::string TimestampToText(uint64_t timestamp);
    
    static void Reset(metadb_handle_list_cref hTracks) noexcept;
    static void WriteToTags(metadb_handle_list_cref hTracks) noexcept;
    static void ReadFromTags(metadb_handle_list_cref hTracks) noexcept;
    static void ImportFromPlayCount(metadb_handle_list_cref hTracks) noexcept;
    static void MarkAsPlayed(metadb_handle_list_cref hTracks) noexcept;

    static void SetRating(metadb_handle_list_cref hTracks, uint32_t rating) noexcept;

    static void Process(metadb_handle_list_cref hTracks, std::function<void (statistics_t & s)> callback);
    static void Refresh(const hash_list_t & tracksToRefresh);

    static void WriteTrack(const metadb_handle_ptr & hTrack) noexcept;

private:
    /// <summary>
    /// Gets a numeric value from a tag.
    /// </summary>
    static int64_t GetNumber(file_info_impl & fileInfo, const char * name) noexcept
    {
        const char * Value = fileInfo.meta_get(name, 0);

        if (Value == nullptr)
            return 0;

        int64_t Number = ::_atoi64(Value);

        return (errno != ERANGE) ? Number : 0;
    }
};
