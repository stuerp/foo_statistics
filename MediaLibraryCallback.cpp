
/** $VER: MediaLibraryCallback.cpp (2025.10.22) **/

#include "pch.h"

#include <sdk\library_callbacks.h>
#include <sdk\titleformat_object.h>
#include <sdk\search_tools.h>

#include <thread>

#include "Resources.h"
#include "Statistics.h"
#include "StatisticsManager.h"
#include "Tags.h"

#pragma hdrstop

namespace
{
    /// <summary>
    /// Callback service receiving notifications about Media Library content changes.
    /// </summary>
    class media_library_callback_t : public library_callback_v2
    {
    public:
        media_library_callback_t() noexcept { };

        media_library_callback_t(const media_library_callback_t &) = delete;
        media_library_callback_t(media_library_callback_t &&) = delete;
        media_library_callback_t & operator=(const media_library_callback_t &) = delete;
        media_library_callback_t & operator=(media_library_callback_t &&) = delete;

        virtual ~media_library_callback_t() noexcept { };

        #pragma region library_callback

        /// <summary>
        /// Called when new items are added to the Media Library.
        /// </summary>
        void on_items_added(metadb_handle_list_cref hTracks) final
        {
            if (!library_manager_v5::get()->is_initialized())
                return;

            metadb_handle_list hTracksToProcess;

            for (auto && hTrack : hTracks)
            {
                console::print(STR_COMPONENT_BASENAME " adding file to media library: \"", hTrack->get_path(), "\"");

                auto Container = hTrack->get_info_ref();

                if (!Container.is_valid())
                    continue;

                auto & FileInfo = Container->info();
/*
                // Dump all metadata tags.
                {
                    const t_size n = FileInfo.meta_get_count();

                    for(t_size i = 0; i < n; ++i)
                        console::print(FileInfo.meta_enum_name(i));
                }
*/
                if (!FileInfo.meta_exists_ex(TagTimestamps, std::strlen(TagTimestamps)))
                    hTracksToProcess += hTrack; // Tag is missing
            }

            if (hTracksToProcess.get_count() > 0)
            {
                console::print(STR_COMPONENT_BASENAME " is adding tags to ", hTracksToProcess.get_count(), " files.");

                const auto Now = statistics_manager_t::Now();

                try
                {
                    statistics_manager_t::Process(hTracksToProcess, [Now](statistics_t & s)
                    {
                        s.SetAddedTimestamp(Now);
                    });
                }
                catch (const std::exception & e)
                {
                    console::print(STR_COMPONENT_BASENAME " failed to add metadata of the newly added tracks: ", e.what());
                }
            }
/*
            auto Index = search_index_manager::get()->create_index(hTracks, nullptr);

            OnItemsAdded(Index);
*/
        }

        /// <summary>
        /// Called when some items in the Media Library have been modified. The list is sorted by pointer value for convenient matching by binary search.
        /// </summary>
        void on_items_modified(metadb_handle_list_cref hTracks) final
        {
            for (auto && hTrack : hTracks)
                console::print(STR_COMPONENT_BASENAME " is processing modified track \"", hTrack->get_path(), "\"");
        }

        /// <summary>
        /// Called when some items have been removed from the Media Library.
        /// </summary>
        void on_items_removed(metadb_handle_list_cref hTracks) final
        {
            for (auto && hTrack : hTracks)
                console::print(STR_COMPONENT_BASENAME " is processing removed track \"", hTrack->get_path(), "\".");
        }

        #pragma endregion

        #pragma region library_callback_v2

        void on_library_initialized() final
        {
            auto Thread = std::thread
            (
                []
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // Wait 5s

                    fb2k::inMainThread
                    (
                        []
                        {
                            if (core_api::are_services_available())
                            {
                                console::print(STR_COMPONENT_BASENAME " is processing tracks that were added to the media library when foobar2000 was not running...");

                                metadb_handle_list hTracks;

                                library_manager_v6::get()->get_all_items(hTracks);

                                metadb_handle_list hTracksToProcess;

                                for (auto && TrackHandle : hTracks)
                                {
                                    console::print(STR_COMPONENT_BASENAME " is adding metadata for \"", TrackHandle->get_path(), "\".");

                                    auto Container = TrackHandle->get_info_ref();

                                    if (!Container.is_valid())
                                        continue;

                                    auto & FileInfo = Container->info();
/*
                                    // Dump all metadata tags.

                                    {
                                        const t_size n = FileInfo.meta_get_count();

                                        for(t_size i = 0; i < n; ++i)
                                            console::print(FileInfo.meta_enum_name(i));
                                    }
*/
                                    if (!FileInfo.meta_exists_ex(TagTimestamps, std::strlen(TagTimestamps)))
                                        hTracksToProcess += TrackHandle; // Tag is missing
                                }

                                if (hTracksToProcess.get_count() > 0)
                                {
                                    const auto Now = statistics_manager_t::Now();

                                    try
                                    {
                                        statistics_manager_t::Process(hTracksToProcess, [Now](statistics_t & s)
                                        {
                                            s.SetAddedTimestamp(Now);
                                        });
                                    }
                                    catch (const std::exception & e)
                                    {
                                        console::print(STR_COMPONENT_BASENAME " failed to add metadata of the newly added tracks: ", e.what());
                                    }
                                }
/*
                                auto Index = search_index_manager::get()->get_library_index();

                                OnItemsAdded(Index);
*/
                            }
                        }
                    );
                }
            );

            Thread.detach();
        }

        void on_items_modified_v2(metadb_handle_list_cref, metadb_io_callback_v2_data &) final
        {
        }

        #pragma endregion

    private:
        /// <summary>
        /// Called when items have been added to the media library.
        /// </summary>
        void OnItemsAdded(const search_index::ptr & searchIndex) const noexcept
        {
            // Determine the tracks that don't have our taqs yet.
            static const char * Query = TagTimestamps " MISSING";

//          static const auto SearchFilter = search_filter_manager_v2::get()->create_ex(Query, fb2k::service_new<completion_notify_dummy>(), search_filter_manager_v2::KFlagSuppressNotify);
            static const auto SearchFilter = search_filter_manager_v3::get()->create(Query);

            if (!SearchFilter.is_valid())
                return;

            auto TrackArray = searchIndex->search(SearchFilter, nullptr, ~0u, fb2k::noAbort);
            
            if (TrackArray->get_count() == 0)
                return;

            // Add statistics to the meta database for each of the tracks.
            {
                console::printf(STR_COMPONENT_BASENAME " is adding metadata for %d tracks to the metadb library...", TrackArray->get_count());

                const auto & hTracks = TrackArray->as_list_of<metadb_handle>();

                const auto Now = statistics_manager_t::Now();

                try
                {
                    statistics_manager_t::Process(hTracks, [Now](statistics_t & s)
                    {
                        s.SetAddedTimestamp(Now);
                    });
                }
                catch (const std::exception & e)
                {
                    console::print(STR_COMPONENT_BASENAME " failed to add metadata of the newly added tracks: ", e.what());
                }
            }
        }
    };

    FB2K_SERVICE_FACTORY(media_library_callback_t);
}
