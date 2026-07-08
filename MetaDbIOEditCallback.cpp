
/** $VER: MetaDbIOEditCallback.cpp (2024.07.29) **/

#include "pch.h"

#include "MetaDbIndexClient.h"
#include "StatisticsManager.h"

#include <SDK\metadb_callbacks.h>
#include <SDK\titleformat_object.h>

#pragma hdrstop

namespace
{
    /// <summary>
    /// Callback service receiving notifications about user-triggered tag edits.
    /// </summary>
    class metadb_io_edit_callback_t : public metadb_io_edit_callback
    {
    public:
        metadb_io_edit_callback_t() noexcept { };

        metadb_io_edit_callback_t(const metadb_io_edit_callback_t &) = delete;
        metadb_io_edit_callback_t(metadb_io_edit_callback_t &&) = delete;
        metadb_io_edit_callback_t & operator=(const metadb_io_edit_callback_t &) = delete;
        metadb_io_edit_callback_t & operator=(metadb_io_edit_callback_t &&) = delete;

        virtual ~metadb_io_edit_callback_t() noexcept { };

        /// <summary>
        ///  Called after the user has edited tags on a set of files.
        /// </summary>
        void on_edited(metadb_handle_list_cref hTracks, t_infosref oldInfos, t_infosref newInfos)
        {
            hash_list_t TracksToRefresh;

            auto Client = metadb_index_client_t::Instance();

            {
                auto Transaction = statistics_manager_t::GetMetaDbIndexManager()->begin_transaction();

                hash_set_t Hashes;

                for (size_t i = 0; i < hTracks.get_count(); ++i)
                {
                    const auto & Location = hTracks[i]->get_location();

                    const auto OldHash = Client->transform(*oldInfos[i], Location);
                    const auto NewHash = Client->transform(*newInfos[i], Location);

                    if ((OldHash == NewHash) || !Hashes.emplace(NewHash).second)
                        continue;

                    const auto Statistics = statistics_manager_t::GetStatistics(OldHash);

                    statistics_manager_t::PutStatistics(NewHash, Statistics, Transaction);

                    TracksToRefresh.add_item(NewHash);
                }

                Transaction->commit();
            }

            statistics_manager_t::ReadFromTags(hTracks);

            statistics_manager_t::Refresh(TracksToRefresh);
        }
    };

    FB2K_SERVICE_FACTORY(metadb_io_edit_callback_t);
}
