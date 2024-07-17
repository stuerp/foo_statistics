
/** $VER: MetaDbIndexClient.h (2024.07.17) **/

#include "pch.h"

#include <SDK/titleformat.h>

#pragma once

/// <summary>
/// Transforms track information (location + metadata) to a hash for metadb_index_manager.
/// </summary>
class MetaDbIndexClient : public metadb_index_client
{
public:
    MetaDbIndexClient();

    MetaDbIndexClient(const MetaDbIndexClient &) = delete;
    MetaDbIndexClient(MetaDbIndexClient &&) = delete;
    MetaDbIndexClient & operator=(const MetaDbIndexClient &) = delete;
    MetaDbIndexClient & operator=(MetaDbIndexClient &&) = delete;

    virtual ~MetaDbIndexClient() { };

    #pragma region metadb_index_client

    metadb_index_hash transform(const file_info & fileInfo, const playable_location & location) final;

    #pragma endregion

    static MetaDbIndexClient * Instance() noexcept;
    static void Initialize() noexcept;

    metadb_index_hash HashPathName(pfc::string pathName) noexcept;
    metadb_index_hash HashString(pfc::string s) noexcept;

private:
    hasher_md5::ptr _Hasher;
    titleformat_object_ptr _PinToScript;
};
