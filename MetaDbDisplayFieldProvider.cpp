
/** $VER: MetaDbDisplayFieldProvider.cpp (2024.07.17) **/

#include "pch.h"

#include "MetaDbIndexClient.h"
#include "StatisticsManager.h"
#include "Resources.h"
#include "GUIDS.h"
#include "Tags.h"

#include <SDK/metadb_display_field_provider.h>

#pragma hdrstop

namespace
{
    #pragma warning(disable: 5246)
    static const char * FieldNames[] =
    {
        TagNow,
        TagNowTimestamp,

        TagFirstPlayed,
        TagFirstPlayedTimestamp,

        TagLastPlayed,
        TagLastPlayedTimestamp,

        TagAdded,
        TagAddedTimestamp,

        TagPlaycount,
        TagRating,

        TagTimestamps,
    };

    /// <summary>
    /// Provides our own title-formatting fields that are parsed globally with each call to metadb_handle::format_title methods.
    /// </summary>
    class MetaDbDisplayFieldProvider : public metadb_display_field_provider_v2
    {
    public:
        MetaDbDisplayFieldProvider() { };

        MetaDbDisplayFieldProvider(const MetaDbDisplayFieldProvider &) = delete;
        MetaDbDisplayFieldProvider(MetaDbDisplayFieldProvider &&) = delete;
        MetaDbDisplayFieldProvider & operator=(const MetaDbDisplayFieldProvider &) = delete;
        MetaDbDisplayFieldProvider & operator=(MetaDbDisplayFieldProvider &&) = delete;

        virtual ~MetaDbDisplayFieldProvider() { };

        /// <summary>
        /// Returns the number of fields provided by this implementation.
        /// </summary>
        uint32_t get_field_count() final
        {
            return (uint32_t) _countof(FieldNames);
        }

        /// <summary>
        /// Returns the name of specified field provided by this implementation. Names are not case sensitive. It's strongly recommended that you keep your field names plain English / ASCII only.
        /// </summary>
        void get_field_name(uint32_t fieldIndex, pfc::string_base & out) final
        {
            out = FieldNames[fieldIndex];
        }

        /// <summary>
        /// Evaluates the specified field.
        /// </summary>
        bool process_field(uint32_t fieldIndex, metadb_handle * hTrack, titleformat_text_out * text) final
        {
            return process_field_v2(fieldIndex, hTrack, hTrack->query_v2_(), text);
        }

        /// <summary>
        /// Evaluates the specified field with a caller-supplied metadb record to reduce the number of database queries.
        /// </summary>
        bool process_field_v2(uint32_t fieldIndex, metadb_handle * hTrack, const metadb_v2::rec_t & record, titleformat_text_out * text) final
        {
            if (fieldIndex < 2)
                return GetTimestamp(fieldIndex, statistics_manager_t::Now(), text, titleformat_inputtypes::unknown);

            if (record.info.is_empty())
                return false;

            const auto Hash = MetaDbIndexClient::Instance()->transform(record.info->info(), hTrack->get_location());

            const auto Statistics = statistics_manager_t::GetStatistics(Hash);

            switch (fieldIndex)
            {
                case 2:
                case 3:
                    return GetTimestamp(fieldIndex, Statistics.GetFirstPlayedTimestamp(), text);

                case 4:
                case 5:
                    return GetTimestamp(fieldIndex, Statistics.GetLastPlayedTimestamp(), text);

                case 6:
                case 7:
                    return GetTimestamp(fieldIndex, Statistics.GetAddedTimestamp(), text);

                case 8:
                    return GetNumber(Statistics.GetPlaycount(), text);

                case 9:
                    return GetNumber(Statistics.Rating, text);

                case 10:
                    text->write(titleformat_inputtypes::meta, Statistics.GetTimestamps());

                    return true;
            }

            return false;
        }

    private:
        bool GetTimestamp(uint32_t fieldIndex, uint64_t timestamp, titleformat_text_out * text, const GUID & inputType = titleformat_inputtypes::meta)
        {
            if (timestamp == 0)
                return false;

            if (fieldIndex % 2 == 0)
                text->write(inputType, statistics_manager_t::TimestampToText(timestamp));
            else
                text->write(inputType, pfc::format_uint(timestamp));

            return true;
        }

        bool GetNumber(uint32_t value, titleformat_text_out * text)
        {
            if (value == 0)
                return false;

            text->write_int(titleformat_inputtypes::meta, value);

            return true;
        }

        static constexpr uint64_t SecondsPerDay   = 24 * 60 * 60;
        static constexpr uint64_t SecondsPerWeek  = 7 * SecondsPerDay;
        static constexpr uint64_t SecondsPerMonth = 30 * SecondsPerDay;
        static constexpr uint64_t SecondsPerYear  = (uint64_t) (365.25 * SecondsPerDay);
    };

    FB2K_SERVICE_FACTORY(MetaDbDisplayFieldProvider);
}
