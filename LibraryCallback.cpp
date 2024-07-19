
/** $VER: LibraryCallback.cpp (2024.07.19) **/

#include "pch.h"

#include <SDK/titleformat.h>
#include <SDK/library_callbacks.h>
#include <SDK/search_tools.h>

#include <thread>

#include "Resources.h"
#include "StatisticsManager.h"
#include "Tags.h"

#pragma hdrstop

namespace
{
    /// <summary>
    /// Callback service receiving notifications about Media Library content changes.
    /// </summary>
    class LibraryCallback : public library_callback_v2
    {
    public:
        LibraryCallback() noexcept { };

        LibraryCallback(const LibraryCallback &) = delete;
        LibraryCallback(LibraryCallback &&) = delete;
        LibraryCallback & operator=(const LibraryCallback &) = delete;
        LibraryCallback & operator=(LibraryCallback &&) = delete;

        virtual ~LibraryCallback() { };

        // Called when new items are added to the Media Library.
        void on_items_added(metadb_handle_list_cref hTracks) final
        {
            if (!library_manager_v5::get()->is_initialized())
                return;

            auto Index = search_index_manager::get()->create_index(hTracks, nullptr);

            OnItemsAdded(Index);
        }

        // Called when some items in the Media Library have been modified. The list is sorted by pointer value for convenient matching by binary search.
        void on_items_modified(metadb_handle_list_cref) final
        {
        }

        void on_items_modified_v2(metadb_handle_list_cref, metadb_io_callback_v2_data &) final
        {
        }

        // Called when some items have been removed from the Media Library.
        void on_items_removed(metadb_handle_list_cref) final
        {
        }

        void on_library_initialized() final
        {
            auto Thread = std::thread
            (
                [this]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

                    fb2k::inMainThread
                    (
                        [this]
                        {
                            auto Index = search_index_manager::get()->get_library_index();

                            OnItemsAdded(Index);
                        }
                    );
                }
            );

            Thread.detach();
        }

    private:
        /// <summary>
        /// Called when items have been added to the media library.
        /// </summary>
        void OnItemsAdded(const search_index::ptr & searchIndex) const noexcept
        {
            // Determine the tracks that don't have statistics yet.
            const char * Query = "%" TagAdded "% MISSING";

            static const auto SearchFilter = search_filter_manager_v2::get()->create_ex(Query, fb2k::service_new<completion_notify_dummy>(), search_filter_manager_v2::KFlagSuppressNotify);

            auto TrackArray = searchIndex->search(SearchFilter, nullptr, 0, fb2k::noAbort);

            if (TrackArray->get_count() == 0)
                return;

            // Add statistics to the metadatabase for each of the tracks.
            {
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
                    console::print(STR_COMPONENT_BASENAME " failed to add metadata of newly added tracks: ", e.what());
                }
            }
        }
    };

    FB2K_SERVICE_FACTORY(LibraryCallback);
}
