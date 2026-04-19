
/** $VER: TrackPropertyProvider.cpp (2024.07.17) **/

#include "pch.h"

#include "MetaDbIndexClient.h"
#include "StatisticsManager.h"
#include "Resources.h"

#include <sdk\track_property.h>

#pragma hdrstop

namespace
{
    /// <summary>
    /// Provides our values for the Properties dialog.
    /// </summary>
    class track_property_provider_t : public track_property_provider_v5
    {
    public:
        track_property_provider_t() { };

        track_property_provider_t(const track_property_provider_t &) = delete;
        track_property_provider_t(track_property_provider_t &&) = delete;
        track_property_provider_t & operator=(const track_property_provider_t &) = delete;
        track_property_provider_t & operator=(track_property_provider_t &&) = delete;

        virtual ~track_property_provider_t() noexcept { };

        /// <summary>
        /// Returns whether the specified tech info field is processed by our service and should not be displayed among unknown fields.
        /// </summary>
        bool is_our_tech_info(const char *) final
        {
            return false;
        }

        /// <summary>
        /// Sets the display properties of the specified tracks.
        /// </summary>
        void enumerate_properties_v5(metadb_handle_list_cref hTracks, track_property_provider_v5_info_source & source, track_property_callback_v2 & callback, abort_callback &) final
        {
            // Should the specified property group be displayed?
            if (!callback.is_group_wanted(STR_COMPONENT_NAME))
                return;

            if (hTracks.get_count() == 1)
            {
//              console::print(STR_COMPONENT_BASENAME " is getting the display properties for ", hTracks[0]->get_location(), "...");

                auto Record = source.get_info(0);

                if (Record.info.is_empty())
                    return;

                const auto Hash = metadb_index_client_t::Instance()->transform(Record.info->info(), hTracks[0]->get_location());

                const auto Statistics = statistics_manager_t::GetStatistics(Hash);

                uint64_t Timestamp = Statistics.GetAddedTimestamp();

                if (Timestamp > 0)
                    callback.set_property(STR_COMPONENT_NAME, 0.0, "Added", statistics_manager_t::TimestampToText(Timestamp));

                Timestamp = Statistics.GetFirstPlayedTimestamp();

                if (Timestamp > 0)
                    callback.set_property(STR_COMPONENT_NAME, 1.0, "First Played", statistics_manager_t::TimestampToText(Timestamp));

                Timestamp = Statistics.GetLastPlayedTimestamp();

                if (Timestamp > 0)
                    callback.set_property(STR_COMPONENT_NAME, 2.0, "Last Played", statistics_manager_t::TimestampToText(Timestamp));

                const uint32_t PlayCount = Statistics.GetPlayCount();

                if (PlayCount > 0)
                    callback.set_property(STR_COMPONENT_NAME, 3.0, "Play Count", pfc::format_uint(PlayCount));

                if (Statistics.Rating > 0)
                    callback.set_property(STR_COMPONENT_NAME, 4.0, "Rating", pfc::format_uint(Statistics.Rating));
            }
            else
            {
                const uint32_t TotalPlayCount = GetTotalPlaycount(hTracks, source);

                if (TotalPlayCount > 0)
                    callback.set_property(STR_COMPONENT_NAME, 0.0, "Play Count", pfc::format_uint(TotalPlayCount));
            }
        }

    private:
        /// <summary>
        /// Gets the total play count of all the specified tracks.
        /// </summary>
        uint32_t GetTotalPlaycount(metadb_handle_list_cref hTracks, track_property_provider_v5_info_source & source)
        {
            uint32_t TotalPlaycount = 0;

            auto Client = metadb_index_client_t::Instance();

            hash_set_t Hashes;

            for (size_t i = 0; i < hTracks.get_count(); ++i)
            {
                auto Record = source.get_info(i);

                if (Record.info.is_empty())
                    continue;

                const auto Hash = Client->transform(Record.info->info(), hTracks[i]->get_location());

                if (Hashes.emplace(Hash).second)
                    TotalPlaycount += statistics_manager_t::GetStatistics(Hash).GetPlayCount();
            }

            return TotalPlaycount;
        }
    };

    FB2K_SERVICE_FACTORY(track_property_provider_t);
}
