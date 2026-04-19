
/** $VER: StatisticsManager.cpp (2026.04.18) **/

#include "pch.h"

#include "StatisticsManager.h"

#include "Resources.h"
#include "Configuration.h"
#include "GUIDS.h"
#include "MetaDbIndexClient.h"
#include "Tags.h"

#include <sdk\input.h>
#include <sdk\file_lock_manager.h>
#include <sdk\file_info_impl.h>

#include <pfc\filetimetools.h>

#pragma hdrstop

/// <summary>
/// Gets the singleton instance of the meta database manager.
/// </summary>
metadb_index_manager_v2::ptr statistics_manager_t::GetMetaDbIndexManager() noexcept
{
    static metadb_index_manager_v2 * Singleton = metadb_index_manager_v2::get().detach();

    return Singleton;
}

/// <summary>
/// Called when the specified track should be marked as 'played'.
/// </summary>
void statistics_manager_t::OnItemPlayed(const metadb_handle_ptr & hTrack) noexcept
{
    metadb_handle_list_t Tracks;

    Tracks += hTrack;

    MarkAsPlayed(Tracks);
}

/// <summary>
/// Gets the statistics from the metadata db.
/// </summary>
statistics_t statistics_manager_t::GetStatistics(metadb_index_hash hash) noexcept
{
    mem_block_container_impl Data;

    GetMetaDbIndexManager()->get_user_data(MetaDbGUID, hash, Data);

    if (Data.get_size() == 0)
        return statistics_t(); // Return an empty record.

    try
    {
        stream_reader_formatter_simple_ref Reader(Data.get_ptr(), Data.get_size());

        statistics_t Statistics;

        // Read the timestamps.
        uint32_t Size;

        Reader >> Size;

        for (uint32_t i = 0; i < Size; ++i)
        {
            uint64_t Timestamp;

            Reader >> Timestamp;

            Statistics.Timestamps.push_back(Timestamp);
        }

        // Read the rating.
        Reader >> Statistics.Rating;

        return Statistics;
    }
    catch (const exception_io_data & e)
    {
        console::print(STR_COMPONENT_BASENAME, " failed to read statistics from metadata db: ", e.what());

        return statistics_t(); // Return an empty record.
    }
}

/// <summary>
/// Puts the statistics in the metadata db.
/// </summary>
void statistics_manager_t::PutStatistics(metadb_index_hash hash, const statistics_t & statistics, const metadb_index_transaction::ptr & transaction) noexcept
{
    stream_writer_formatter_simple Writer;

    // Write the timestamps.
    Writer << (uint32_t) statistics.Timestamps.size();

    for (const auto & Timestamp : statistics.Timestamps)
        Writer << Timestamp;

    // Write the rating.
    Writer << statistics.Rating;

    transaction->set_user_data(MetaDbGUID, hash, Writer.m_buffer.get_ptr(), Writer.m_buffer.get_size());
}

/// <summary>
/// Converts the specified timestamp to a string.
/// </summary>
pfc::string statistics_manager_t::TimestampToText(uint64_t timestamp)
{
    return pfc::format_filetimestamp(timestamp);
}

/// <summary>
/// Resets the metadata of the specified tracks.
/// </summary>
void statistics_manager_t::Reset(metadb_handle_list_cref hTracks) noexcept
{
    try
    {
        statistics_manager_t::Process(hTracks, [](statistics_t & s)
        {
            s.Reset();
        });
    }
    catch (const std::exception & e)
    {
        console::print(STR_COMPONENT_BASENAME " failed to reset tracks: ", e.what());
    }
}

/// <summary>
/// Writes the metadata of the specified track to file tags.
/// </summary>
void statistics_manager_t::WriteTrack(const metadb_handle_ptr & hTrack) noexcept
{
    metadb_handle_list_t Tracks;

    Tracks += hTrack;

    WriteToTags(Tracks);
}

/// <summary>
/// Writes the metadata of the specified tracks to file tags.
/// </summary>
void statistics_manager_t::WriteToTags(metadb_handle_list_cref hTracks) noexcept
{
    auto Client = metadb_index_client_t::Instance();

    for (const auto & hTrack : hTracks)
    {
        metadb_index_hash Hash = 0;

        if (!Client->hashHandle(hTrack, Hash))
            continue; // The track is not in the meta database.

        const auto FilePath = hTrack->get_path();

        console::print(STR_COMPONENT_BASENAME, " is writing tags to \"", FilePath, "\".");

        try
        {
            // Required to write to files being currently played. See file_lock_manager documentation for details.
            auto Lock = file_lock_manager::get()->acquire_write(FilePath, fb2k::noAbort);

            {
                input_info_writer::ptr Writer;

                const double Timeout = 1.0; // Wait and keep retrying for up to 1 second in case of a sharing violation error.

                input_entry::g_open_for_info_write_timeout(Writer, nullptr, FilePath, fb2k::noAbort, Timeout);

                {
                    const auto SubSongIndex = hTrack->get_subsong_index();

                    file_info_impl FileInfo;

                    Writer->get_info(SubSongIndex, FileInfo, fb2k::noAbort);

                    {
                        const auto Statistics = statistics_manager_t::GetStatistics(Hash);

                        {
                            pfc::string Timestamps = Statistics.GetTimestamps();

                            if (!Timestamps.isEmpty())
                            {
                                FileInfo.meta_set(TagTimestamps, Timestamps);

                                // Add legacy tags for compatibility. Don't touch existing tags.
                                if (_Configuration._WriteLegacyTags)
                                {
                                    if (!FileInfo.meta_exists_ex(TagAddedTimestampLegacy, std::strlen(TagAddedTimestampLegacy)))
                                    {
                                        FileInfo.meta_set(TagAddedTimestampLegacy, pfc::format_uint(Statistics.Timestamps[0]));

                                        console::print(STR_COMPONENT_BASENAME, " added legacy tag " TagAddedTimestampLegacy ".");
                                    }

                                    if (!FileInfo.meta_exists_ex(TagFirstPlayedTimestampLegacy, std::strlen(TagFirstPlayedTimestampLegacy)) && (Statistics.Timestamps.size() > 1))
                                    {
                                        FileInfo.meta_set(TagFirstPlayedTimestampLegacy, pfc::format_uint(Statistics.Timestamps[1]));
                                        FileInfo.meta_set(TagPlayCountLegacy, pfc::format_uint(Statistics.Timestamps.size() - 1));

                                        console::print(STR_COMPONENT_BASENAME, " added legacy tag " TagFirstPlayedTimestampLegacy ".");
                                    }

                                    if (!FileInfo.meta_exists_ex(TagLastPlayedTimestampLegacy, std::strlen(TagLastPlayedTimestampLegacy)) && (Statistics.Timestamps.size() > 2))
                                    {
                                        FileInfo.meta_set(TagLastPlayedTimestampLegacy, pfc::format_uint(Statistics.Timestamps.back()));

                                        console::print(STR_COMPONENT_BASENAME, " added legacy tag " TagLastPlayedTimestampLegacy ".");
                                    }
                                }
                            }
                            else
                                console::print(STR_COMPONENT_BASENAME, " failed to find timestamps for \"", FilePath, "\".");
                        }

                        {
                            if (Statistics.Rating != 0)
                            {
                                FileInfo.meta_set(TagRating, pfc::format_uint(Statistics.Rating));

                                if (_Configuration._WriteLegacyTags)
                                    FileInfo.meta_set("RATING", pfc::format_uint(Statistics.Rating));
                            }
                            else
                                FileInfo.meta_remove_field(TagRating);
                        }

                        if (_Configuration._RemoveTags)
                        {
                            for (const auto & Tag : _Configuration._TagsToRemoveList)
                                FileInfo.meta_remove_field(Tag);
                        }
                    }

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
/// Reads the metadata of the specified tracks from file tags.
/// </summary>
void statistics_manager_t::ReadFromTags(metadb_handle_list_cref hTracks) noexcept
{
    auto Client = metadb_index_client_t::Instance();

    for (const auto & hTrack : hTracks)
    {
        metadb_index_hash Hash = 0;

        if (!Client->hashHandle(hTrack, Hash))
            continue; // The track is not in the metadata db.

        auto Statistics = statistics_manager_t::GetStatistics(Hash);

        const auto FilePath = hTrack->get_path();

        console::print(STR_COMPONENT_BASENAME, " is reading tags from \"", FilePath, "\".");

        try
        {
            // Required to read from files being currently played. See file_lock_manager documentation for details.
            auto Lock = file_lock_manager::get()->acquire_read(FilePath, fb2k::noAbort);

            {
                input_info_reader::ptr Reader;

                input_entry::g_open_for_info_read(Reader, nullptr, FilePath, fb2k::noAbort);

                {
                    const auto SubSongIndex = hTrack->get_subsong_index();

                    file_info_impl FileInfo;

                    Reader->get_info(SubSongIndex, FileInfo, fb2k::noAbort);

                    // Import the timestamps.
                    {
                        const char * Timestamps = FileInfo.meta_get(TagTimestamps, 0);

                        if (Timestamps != nullptr)
                        {
                            Statistics.Timestamps.clear();

                            Statistics.SetTimestamps(Timestamps);
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
                        else
                            Statistics.Rating = 0;
                    }
                }
            }
        }
        catch (const std::exception & e)
        {
            console::print(STR_COMPONENT_BASENAME " failed to read tags from \"", FilePath, "\": ", e.what());

            return;
        }

        try
        {
            auto Transaction = GetMetaDbIndexManager()->begin_transaction();

            PutStatistics(Hash, Statistics, Transaction);

            Transaction->commit();

            console::print(STR_COMPONENT_BASENAME, " updated metadata for \"", FilePath, "\".");
        }
        catch (const std::exception & e)
        {
            console::print(STR_COMPONENT_BASENAME " failed to update metadata for \"", FilePath, "\": ", e.what());
        }

        // Signals all components that the metadata for the specified track has been altered.
        GetMetaDbIndexManager()->dispatch_refresh(MetaDbGUID, Hash);
    }
}

/// <summary>
/// Imports the tags of the official foo_playcount component.
/// </summary>
void statistics_manager_t::ImportFromLegacyTags(metadb_handle_list_cref hTracks) noexcept
{
    auto Client = metadb_index_client_t::Instance();

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
                    uint64_t Timestamp = (uint64_t) statistics_manager_t::GetNumber(FileInfo, TagAddedTimestampLegacy);

                    if (Timestamp != 0)
                    {
                        Statistics.SetAddedTimestamp(Timestamp);

                        // Import the First Played timestamp.
                        uint64_t FirstPlayed = (uint64_t) statistics_manager_t::GetNumber(FileInfo, TagFirstPlayedTimestampLegacy);

                        if (FirstPlayed != 0)
                        {
                            Statistics.SetFirstPlayedTimestamp(FirstPlayed);

                            uint64_t PlayCount = (uint64_t) statistics_manager_t::GetNumber(FileInfo, TagPlayCountLegacy);

                            if (PlayCount > 1)
                            {
                                // Import the Last Played timestamp.
                                uint64_t LastPlayed = (uint64_t) statistics_manager_t::GetNumber(FileInfo, TagLastPlayedTimestampLegacy);

                                if (LastPlayed != 0)
                                {
                                    if (PlayCount > 2)
                                    {
                                        // Add the linear interpolated timestamps.
                                        uint64_t Delta = (LastPlayed - FirstPlayed) / (PlayCount - 1);

                                        Timestamp = FirstPlayed;

                                        for (; PlayCount > 2; --PlayCount)
                                        {
                                            Timestamp += Delta;

                                            Statistics.AddPlayTimestamp(Timestamp);
                                        }
                                    }

                                    Statistics.AddPlayTimestamp(LastPlayed);
                                }
                            }
                        }
                    }
                }

                // Import the rating.
                Statistics.Rating = (uint32_t) statistics_manager_t::GetNumber(FileInfo, "rating");
            }

            {
                auto Transaction = GetMetaDbIndexManager()->begin_transaction();

                PutStatistics(Hash, Statistics, Transaction);

                Transaction->commit();

                // Signals all components that the metadata for the specified track has been altered.
                GetMetaDbIndexManager()->dispatch_refresh(MetaDbGUID, Hash);
            }

            console::print(STR_COMPONENT_BASENAME, " has imported tags from \"", FilePath, "\".");
        }
        catch (const std::exception & e)
        {
            console::print(STR_COMPONENT_BASENAME " failed to import tags from \"", FilePath, "\": ", e.what());
        }
    }
}

/// <summary>
/// Marks the specified tracks as played.
/// </summary>
void statistics_manager_t::MarkAsPlayed(metadb_handle_list_cref hTracks) noexcept
{
    try
    {
        const auto Timestamp = Now();

        statistics_manager_t::Process(hTracks, [Timestamp](statistics_t & s)
        {
            s.AddPlayTimestamp(Timestamp);
        });
    }
    catch (const std::exception & e)
    {
        console::print(STR_COMPONENT_BASENAME " failed to mark ", hTracks.size(), " tracks as played: ", e.what());
    }
}

/// <summary>
/// Sets the rating of the specified tracks.
/// </summary>
void statistics_manager_t::SetRating(metadb_handle_list_cref hTracks, uint32_t rating) noexcept
{
    try
    {
        statistics_manager_t::Process(hTracks, [rating](statistics_t & s)
        {
            s.Rating = rating;
        });
    }
    catch (const std::exception & e)
    {
        console::print(STR_COMPONENT_BASENAME " failed to set rating of tracks: ", e.what());
    }
}

/// <summary>
/// Processes the specified tracks.
/// </summary>
void statistics_manager_t::Process(metadb_handle_list_cref hTracks, std::function<void (statistics_t & s)> callback)
{
    auto Client = metadb_index_client_t::Instance();

    hash_list_t TracksToRefresh;
    hash_set_t Hashes;

    auto Transaction = GetMetaDbIndexManager()->begin_transaction();

    for (const auto & hTrack : hTracks)
    {
        metadb_index_hash Hash = 0;

        if (!Client->hashHandle(hTrack, Hash))
            continue;

        if (!Hashes.emplace(Hash).second) // Only process unique tracks.
            continue;

        statistics_t Statistics = GetStatistics(Hash);

        callback(Statistics);

        PutStatistics(Hash, Statistics, Transaction);

        TracksToRefresh.add_item(Hash);
    }

    Transaction->commit();

    if (_Configuration._WriteToTags)
        WriteToTags(hTracks);

    Refresh(TracksToRefresh);
}

/// <summary>
/// Signals all components that the metadata for the specified tracks has been altered.
/// </summary>
void statistics_manager_t::Refresh(const hash_list_t & tracksToRefresh)
{
    if (tracksToRefresh.get_count() == 0)
        return;

    GetMetaDbIndexManager()->dispatch_refresh(MetaDbGUID, tracksToRefresh);
}

/// <summary>
/// Gets a numeric value from a tag.
/// </summary>
int64_t statistics_manager_t::GetNumber(file_info_impl & fileInfo, const char * name) noexcept
{
    const char * Value = fileInfo.meta_get(name, 0);

    if (Value == nullptr)
        return 0;

    int64_t Number = ::_atoi64(Value);

    return (errno != ERANGE) ? Number : 0;
}
