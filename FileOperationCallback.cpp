
/** $VER: FileOperationCallback.cpp (2024.07.16) **/

#include "pch.h"

#include "Configuration.h"
#include "MetaDbIndexClient.h"
#include "StatisticsManager.h"

#include <SDK/file_operation_callback.h>

#include <ranges>

#pragma hdrstop

namespace
{
    /// <summary>
    /// Called  to notify about files being deleted or moved.
    /// </summary>
    class FileOperationCallback : public file_operation_callback
    {
    public:
        FileOperationCallback() { };

        FileOperationCallback(const FileOperationCallback &) = delete;
        FileOperationCallback(FileOperationCallback &&) = delete;
        FileOperationCallback & operator=(const FileOperationCallback &) = delete;
        FileOperationCallback & operator=(FileOperationCallback &&) = delete;

        virtual ~FileOperationCallback() { };

        #pragma region file_operation_callback

        void on_files_copied_sorted(t_pathlist from, t_pathlist to) final
        {
            OnFileOperation(from, to);
        }

        void on_files_deleted_sorted(t_pathlist) final
        {
        }

        void on_files_moved_sorted(t_pathlist from, t_pathlist to) final
        {
            OnFileOperation(from, to);
        }

        #pragma endregion

    private:
        void OnFileOperation(t_pathlist oldPath, t_pathlist newPath)
        {
            if (!_Configuration.LegacyMode)
                return;

            hash_list_t NewHashes;

            auto Client = MetaDbIndexClient::Instance();

            auto Transaction = StatisticsManager::GetMetaDbIndexManager()->begin_transaction();

            for (const size_t i : std::views::iota(size_t{}, oldPath.get_count()))
            {
                const auto OldHash = Client->HashPathName(oldPath[i]);
                const auto NewHash = Client->HashPathName(newPath[i]);

                const auto Statistics = StatisticsManager::GetStatistics(OldHash);

                StatisticsManager::SetStatistics(NewHash, Statistics, Transaction);

                NewHashes.add_item(NewHash);
            }

            Transaction->commit();

            StatisticsManager::Refresh(NewHashes);
        }
    };

    FB2K_SERVICE_FACTORY(FileOperationCallback);
}
