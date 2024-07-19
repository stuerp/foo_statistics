
/** $VER: TrackPropertyProvider.cpp (2024.07.17) **/

#include "pch.h"

#include "Configuration.h"
#include "MetaDbIndexClient.h"
#include "StatisticsManager.h"

#include "Resources.h"

#include <SDK/track_property.h>

#pragma hdrstop

namespace
{
    /// <summary>
    /// Allows metadb v2 info records to be handed over transparently.
    /// </summary>
    class TrackPropertyProvider : public track_property_provider_v5
    {
    public:
        TrackPropertyProvider() { };

        TrackPropertyProvider(const TrackPropertyProvider &) = delete;
        TrackPropertyProvider(TrackPropertyProvider &&) = delete;
        TrackPropertyProvider & operator=(const TrackPropertyProvider &) = delete;
        TrackPropertyProvider & operator=(TrackPropertyProvider &&) = delete;

        virtual ~TrackPropertyProvider() { };

        /// <summary>
        /// Returns whether the specified tech info field is processed by our service and should not be displayed among unknown fields.
        /// <returns></returns>
        bool is_our_tech_info(const char *) final
        {
            return false;
        }

        /// <summary>
        /// Enumerates properties of the specified track list.
        /// </summary>
        void enumerate_properties_v5(metadb_handle_list_cref hTracks, track_property_provider_v5_info_source & source, track_property_callback_v2 & callback, abort_callback &) final
        {
            // Should the specified property group be displayed?
            if (!callback.is_group_wanted(STR_COMPONENT_NAME))
                return;

            if (hTracks.get_count() == 1)
            {
                auto Record = source.get_info(0);

                if (Record.info.is_empty())
                    return;

                const auto Hash = MetaDbIndexClient::Instance()->transform(Record.info->info(), hTracks[0]->get_location());

                const auto Statistics = statistics_manager_t::GetStatistics(Hash);

                if (Statistics.GetAddedTimestamp() > 0)
                    callback.set_property(STR_COMPONENT_NAME, 0.0, "Added", statistics_manager_t::TimestampToText(Statistics.GetAddedTimestamp()));

                if (Statistics.GetFirstPlayedTimestamp() > 0)
                    callback.set_property(STR_COMPONENT_NAME, 1.0, "First Played", statistics_manager_t::TimestampToText(Statistics.GetFirstPlayedTimestamp()));

                if (Statistics.GetLastPlayedTimestamp() > 0)
                    callback.set_property(STR_COMPONENT_NAME, 2.0, "Last Played", statistics_manager_t::TimestampToText(Statistics.GetLastPlayedTimestamp()));

                if (Statistics.GetPlaycount() > 0)
                    callback.set_property(STR_COMPONENT_NAME, 3.0, "Playcount", pfc::format_uint(Statistics.GetPlaycount()));

                if (Statistics.Rating > 0)
                    callback.set_property(STR_COMPONENT_NAME, 4.0, "Rating", pfc::format_uint(Statistics.Rating));
            }
            else
            {
                const uint32_t TotalPlaycount = GetTotalPlaycount(hTracks, source);

                if (TotalPlaycount > 0)
                    callback.set_property(STR_COMPONENT_NAME, 0.0, "Playcount", pfc::format_uint(TotalPlaycount));
            }
        }

    private:
        /// <summary>
        /// Gets the total playcount of all the specified tracks.
        /// </summary>
        uint32_t GetTotalPlaycount(metadb_handle_list_cref hTracks, track_property_provider_v5_info_source & source)
        {
            uint32_t TotalPlaycount = 0;

            auto Client = MetaDbIndexClient::Instance();

            hash_set_t Hashes;

            for (size_t i = 0; i < hTracks.get_count(); ++i)
            {
                auto Record = source.get_info(i);

                if (Record.info.is_empty())
                    continue;

                const auto Hash = Client->transform(Record.info->info(), hTracks[i]->get_location());

                if (Hashes.emplace(Hash).second)
                    TotalPlaycount += statistics_manager_t::GetStatistics(Hash).GetPlaycount();
            }

            return TotalPlaycount;
        }
    };

    FB2K_SERVICE_FACTORY(TrackPropertyProvider);
}
