
/** $VER: StatisticsManager.cpp (2024.07.17) **/

#include "pch.h"

#include "StatisticsManager.h"

#include "Resources.h"
#include "Configuration.h"
#include "GUIDS.h"
#include "MetaDbIndexClient.h"
#include "Tags.h"

#include <SDK/input.h>
#include <SDK/file_lock_manager.h>
#include <SDK/file_info_impl.h>

#include <pfc/filetimetools.h>

#include <ranges>

#pragma hdrstop

/// <summary>
/// Gets the singleton instance of the meta database manager.
/// </summary>
metadb_index_manager_v2::ptr StatisticsManager::GetMetaDbIndexManager() noexcept
{
    static metadb_index_manager_v2 * Singleton = metadb_index_manager_v2::get().detach();

    return Singleton;
}

/// <summary>
/// Called when the specified track should be marked as 'played'.
/// </summary>
void StatisticsManager::OnItemPlayed(const metadb_handle_ptr & hTrack) noexcept
{
    const auto Timestamp = Now();

    metadb_index_hash Hash = 0;

    if (!MetaDbIndexClient::Instance()->hashHandle(hTrack, Hash))
        return;

    {
        auto Statistics = GetStatistics(Hash);

        Statistics.Timestamps.push_back(Timestamp);

        {
            auto Transaction = GetMetaDbIndexManager()->begin_transaction();

            SetStatistics(Hash, Statistics, Transaction);

            Transaction->commit();
        }

        // Signals all components that the metadata for the specified track has been altered.
        GetMetaDbIndexManager()->dispatch_refresh(MetaDbGUID, Hash);
    }
}

/// <summary>
/// Gets the statistics with the specified hash.
/// </summary>
StatisticsManager::statistics_t StatisticsManager::GetStatistics(metadb_index_hash hash) noexcept
{
    mem_block_container_impl Data;

    GetMetaDbIndexManager()->get_user_data(MetaDbGUID, hash, Data);

    if (Data.get_size() > 0)
    {
        try
        {
            statistics_t Statistics;

            stream_reader_formatter_simple_ref Reader(Data.get_ptr(), Data.get_size());

            // Read the timestamps.
            uint32_t Size;

            Reader >> Size;

            for (uint32_t i = 0; i < Size; ++i)
            {
                uint64_t Timestamp;

                Reader >> Timestamp;

                Statistics.Timestamps.push_back(Timestamp);
            }

            // Read the other statistics.
            Reader >> Statistics.Rating;

            return Statistics;
        }
        catch (exception_io_data)
        {
        }
    }

    return statistics_t();
}

/// <summary>
/// Sets the statistics.
/// </summary>
void StatisticsManager::SetStatistics(metadb_index_hash hash, const statistics_t & statistics, const metadb_index_transaction::ptr & transaction) noexcept
{
    stream_writer_formatter_simple Writer;

    Writer << (uint32_t) statistics.Timestamps.size();

    for (const auto & Timestamp : statistics.Timestamps)
        Writer << Timestamp;

    Writer << statistics.Rating;

    transaction->set_user_data(MetaDbGUID, hash, Writer.m_buffer.get_ptr(), Writer.m_buffer.get_size());
}

/// <summary>
/// Gets the hashes of the specified tracks.
/// </summary>
hash_set_t StatisticsManager::GetHashes(metadb_handle_list_cref hTracks) noexcept
{
    hash_set_t Hashes;

    auto Client = MetaDbIndexClient::Instance();

    for (const auto & hTrack : hTracks)
    {
        metadb_index_hash Hash = 0;

        if (Client->hashHandle(hTrack, Hash))
            Hashes.emplace(Hash);
    }

    return Hashes;
}

/// <summary>
/// Converts the specified timestamp to a string.
/// </summary>
pfc::string StatisticsManager::TimestampToText(uint64_t timestamp)
{
    return pfc::format_filetimestamp(timestamp);
}

/// <summary>
/// Resets the metadata of the specified tracks.
/// </summary>
void StatisticsManager::Reset(metadb_handle_list_cref hTracks) noexcept
{
    StatisticsManager::Process(hTracks, [](statistics_t & s)
    {
        s.Reset();
    });
}

/// <summary>
/// Writes the metadata of the specified tracks to file.
/// </summary>
void StatisticsManager::Write(metadb_handle_list_cref hTracks) noexcept
{
    auto Client = MetaDbIndexClient::Instance();

    for (const auto & hTrack : hTracks)
    {
        metadb_index_hash Hash = 0;

        if (!Client->hashHandle(hTrack, Hash))
            continue;

        const auto FilePath = hTrack->get_path();
        const auto SubSongIndex = hTrack->get_subsong_index();

        try
        {
            // Required to write to files being currently played. See file_lock_manager documentation for details.
            auto Lock = file_lock_manager::get()->acquire_write(FilePath, fb2k::noAbort);

            {
                input_info_writer::ptr Writer;

                const double Timeout = 1.0; // Wait and keep retrying for up to 1 second in case of a sharing violation error.

                input_entry::g_open_for_info_write_timeout(Writer, nullptr, FilePath, fb2k::noAbort, Timeout);

                {
                    file_info_impl FileInfo;

                    Writer->get_info(SubSongIndex, FileInfo, fb2k::noAbort);

                    const auto Statistics = StatisticsManager::GetStatistics(Hash);

                    FileInfo.meta_set(TagTimestamps, Statistics.GetTimestamps());

                    if (Statistics.Rating != 0)
                        FileInfo.meta_set(TagRating, pfc::format_uint(Statistics.Rating));
/*
                    FileInfo.meta_set(TagAddedTimestamp, pfc::format_uint(Statistics.GetAddedTimestamp()));
                    FileInfo.meta_set(TagFirstPlayedTimestamp, pfc::format_uint(Statistics.GetFirstPlayedTimestamp()));
                    FileInfo.meta_set(TagLastPlayedTimestamp, pfc::format_uint(Statistics.GetLastPlayedTimestamp()));
                    FileInfo.meta_set(TagPlaycount, pfc::format_uint(Statistics.GetPlaycount()));
*/
                    Writer->set_info(SubSongIndex, FileInfo, fb2k::noAbort);

                    Writer->commit(fb2k::noAbort);
                }
            }
        }
        catch (const std::exception & e)
        {
            console::print(STR_COMPONENT_BASENAME " failed to write tags to \"", FilePath, "\": ", e.what());
        }
    }
}

/// <summary>
/// Reads the metadata of the specified tracks from file.
/// </summary>
void StatisticsManager::Read(metadb_handle_list_cref hTracks) noexcept
{
    auto Client = MetaDbIndexClient::Instance();

    for (const auto & hTrack : hTracks)
    {
        metadb_index_hash Hash = 0;

        if (!Client->hashHandle(hTrack, Hash))
            continue;

        const auto FilePath = hTrack->get_path();
        const auto SubSongIndex = hTrack->get_subsong_index();

        try
        {
            auto Statistics = StatisticsManager::GetStatistics(Hash);

            // Required to read from files being currently played. See file_lock_manager documentation for details.
            auto Lock = file_lock_manager::get()->acquire_read(FilePath, fb2k::noAbort);

            {
                input_info_reader::ptr Reader;

                input_entry::g_open_for_info_read(Reader, nullptr, FilePath, fb2k::noAbort);

                // Import the timestamps.
                {
                    file_info_impl FileInfo;

                    Reader->get_info(SubSongIndex, FileInfo, fb2k::noAbort);

                    Statistics.Timestamps.clear();

                    const char * Timestamps = FileInfo.meta_get(TagTimestamps, 0);

                    if (Timestamps != nullptr)
                        Statistics.SetTimestamps(Timestamps);
                    else
                    {
                        /* Try to import from the tags of the official foo_playcount component. */

                        // Import the Added timestamp.
                        const char * Value = FileInfo.meta_get(TagAddedTimestamp, 0);

                        if (Value != nullptr)
                        {
                            uint64_t Timestamp = (uint64_t) ::_atoi64(Value);

                            Statistics.SetAddedTimestamp((errno != ERANGE) ? Timestamp : 0);

                            // Import the First Played timestamp.
                            Value = FileInfo.meta_get(TagFirstPlayedTimestamp, 0);

                            if (Value != nullptr)
                            {
                                Timestamp = (uint64_t) ::_atoi64(Value);

                                if (errno != ERANGE)
                                {
                                    Statistics.Timestamps.push_back(Timestamp);

                                    // Import the Last Played timestamp.
                                    Value = FileInfo.meta_get(TagLastPlayedTimestamp, 0);

                                    if (Value != nullptr)
                                    {
                                        Timestamp = (uint64_t) ::_atoi64(Value);

                                        if (errno != ERANGE)
                                            Statistics.Timestamps.push_back(Timestamp);
                                    }
                                }
                            }
                        }
                    }

                    // Import the rating.
                    {
                        const char * Value = FileInfo.meta_get(TagRating, 0);

                        if (Value != nullptr)
                        {
                            uint32_t Rating = (uint32_t) std::atoi(Value);

                            Statistics.Rating = (errno != ERANGE) ? Rating : 0;
                        }
                    }
                }
            }

            {
                auto Transaction = GetMetaDbIndexManager()->begin_transaction();

                SetStatistics(Hash, Statistics, Transaction);

                Transaction->commit();

                // Signals all components that the metadata for the specified track has been altered.
                GetMetaDbIndexManager()->dispatch_refresh(MetaDbGUID, Hash);
            }
        }
        catch (const std::exception & e)
        {
            console::print(STR_COMPONENT_BASENAME " failed to read tags from \"", FilePath, "\": ", e.what());
        }
    }
}

/// <summary>
/// Migrates the metadata of the official foo_playcount components.
/// </summary>
void StatisticsManager::Migrate(metadb_handle_list_cref hTracks) noexcept
{
    auto Client = MetaDbIndexClient::Instance();

    for (const auto & hTrack : hTracks)
    {
        metadb_index_hash Hash = 0;

        if (!Client->hashHandle(hTrack, Hash))
            continue;

        const auto FilePath = hTrack->get_path();
        const auto SubSongIndex = hTrack->get_subsong_index();

        try
        {
            statistics_t Statistics = { };

            // Required to read from files being currently played. See file_lock_manager documentation for details.
            auto Lock = file_lock_manager::get()->acquire_read(FilePath, fb2k::noAbort);

            {
                input_info_reader::ptr Reader;

                input_entry::g_open_for_info_read(Reader, nullptr, FilePath, fb2k::noAbort);

                file_info_impl FileInfo;

                Reader->get_info(SubSongIndex, FileInfo, fb2k::noAbort);

                // Import the timestamps.
                {
                    // Import the Added timestamp.
                    uint64_t Timestamp = (uint64_t) StatisticsManager::GetNumber(FileInfo, "added_timestamp");

                    if (Timestamp != 0)
                    {
                        Statistics.SetAddedTimestamp(Timestamp);

                        // Import the First Played timestamp.
                        uint64_t FirstPlayed = (uint64_t) StatisticsManager::GetNumber(FileInfo, "first_played_timestamp");

                        if (FirstPlayed != 0)
                        {
                            Statistics.Timestamps.push_back(FirstPlayed);

                            uint64_t Playcount = (uint64_t) StatisticsManager::GetNumber(FileInfo, "play_count");

                            if (Playcount > 1)
                            {
                                // Import the Last Played timestamp.
                                uint64_t LastPlayed = (uint64_t) StatisticsManager::GetNumber(FileInfo, "last_played_timestamp");

                                if (LastPlayed != 0)
                                {
                                    if (Playcount > 2)
                                    {
                                        // Add the linear interpolated timestamps.
                                        uint64_t Delta = (LastPlayed - FirstPlayed) / (Playcount - 1);

                                        Timestamp = FirstPlayed;

                                        for (; Playcount > 2; --Playcount)
                                        {
                                            Timestamp += Delta;

                                            Statistics.Timestamps.push_back(Timestamp);
                                        }
                                    }

                                    Statistics.Timestamps.push_back(LastPlayed);
                                }
                            }
                        }
                    }
                }

                // Import the rating.
                Statistics.Rating = (uint32_t) StatisticsManager::GetNumber(FileInfo, "rating");
            }

            {
                auto Transaction = GetMetaDbIndexManager()->begin_transaction();

                SetStatistics(Hash, Statistics, Transaction);

                Transaction->commit();

                // Signals all components that the metadata for the specified track has been altered.
                GetMetaDbIndexManager()->dispatch_refresh(MetaDbGUID, Hash);
            }

            console::print(STR_COMPONENT_BASENAME, " has migrated tags from \"", FilePath, "\".");
        }
        catch (const std::exception & e)
        {
            console::print(STR_COMPONENT_BASENAME " failed to migrate tags from \"", FilePath, "\": ", e.what());
        }
    }
}

/// <summary>
/// Sets the rating of the specfied tracks.
/// </summary>
void StatisticsManager::SetRating(metadb_handle_list_cref hTracks, uint32_t rating) noexcept
{
    StatisticsManager::Process(hTracks, [rating](statistics_t & s)
    {
        s.Rating = rating;
    });
}

/// <summary>
/// Processes the specfied tracks.
/// </summary>
void StatisticsManager::Process(metadb_handle_list_cref hTracks, std::function<void (statistics_t & s)> callback) noexcept
{
    hash_list_t HashList;

    const auto Hashes = GetHashes(hTracks);

    auto Transaction = GetMetaDbIndexManager()->begin_transaction();

    for (const auto & Hash : Hashes)
    {
        statistics_t Statistics = GetStatistics(Hash);

        callback(Statistics);

        SetStatistics(Hash, Statistics, Transaction);

        HashList.add_item(Hash);
    }

    Transaction->commit();

    Refresh(HashList);
}

/// <summary>
/// Signals all components that the metadata for the specified tracks has been altered.
/// </summary>
void StatisticsManager::Refresh(const hash_list_t & hashList)
{
    if (hashList.get_count() == 0)
        return;

    GetMetaDbIndexManager()->dispatch_refresh(MetaDbGUID, hashList);
}
