
/** $VER: MetaDbIOEditCallback.cpp (2024.07.18) **/

#include "pch.h"

#include <SDK/metadb_callbacks.h>

#include "Configuration.h"
#include "MetaDbIndexClient.h"
#include "StatisticsManager.h"

#pragma hdrstop

namespace
{
    /// <summary>
    /// Callback service receiving notifications about user-triggered tag edits.
    /// </summary>
    class MetadbIOEditCallback : public metadb_io_edit_callback
    {
    public:
        MetadbIOEditCallback() noexcept { };

        MetadbIOEditCallback(const MetadbIOEditCallback &) = delete;
        MetadbIOEditCallback(MetadbIOEditCallback &&) = delete;
        MetadbIOEditCallback & operator=(const MetadbIOEditCallback &) = delete;
        MetadbIOEditCallback & operator=(MetadbIOEditCallback &&) = delete;

        virtual ~MetadbIOEditCallback() { };

        void on_edited(metadb_handle_list_cref hTracks, t_infosref oldInfos, t_infosref newInfos)
        {
            hash_list_t TracksToRefresh;

            auto Client = MetaDbIndexClient::Instance();

            {
                auto Transaction = StatisticsManager::GetMetaDbIndexManager()->begin_transaction();

                hash_set_t HashSet;

                for (size_t i = 0; i < hTracks.get_count(); ++i)
                {
                    const auto & Location = hTracks[i]->get_location();

                    const auto OldHash = Client->transform(*oldInfos[i], Location);
                    const auto NewHash = Client->transform(*newInfos[i], Location);

                    if ((OldHash == NewHash) || !HashSet.emplace(NewHash).second)
                        continue;

                    const auto Statistics = StatisticsManager::GetStatistics(OldHash);

                    StatisticsManager::SetStatistics(NewHash, Statistics, Transaction);

                    TracksToRefresh.add_item(NewHash);
                }

                Transaction->commit();
            }

            StatisticsManager::Refresh(TracksToRefresh);
        }
    };

    FB2K_SERVICE_FACTORY(MetadbIOEditCallback);
}
