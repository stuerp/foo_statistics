
/** $VER: MetaDbIndexClient.h (2026.03.14) **/

#include "pch.h"

#pragma once

/// <summary>
/// Transforms track information (location + metadata) to a metadb_index_manager hash to which our data gets pinned.
/// </summary>
class metadb_index_client_t : public metadb_index_client
{
public:
    metadb_index_client_t();

    metadb_index_client_t(const metadb_index_client_t &) = delete;
    metadb_index_client_t(metadb_index_client_t &&) = delete;
    metadb_index_client_t & operator=(const metadb_index_client_t &) = delete;
    metadb_index_client_t & operator=(metadb_index_client_t &&) = delete;

    virtual ~metadb_index_client_t() noexcept { };

    #pragma region metadb_index_client

    metadb_index_hash transform(const file_info & fileInfo, const playable_location & location) final;

    #pragma endregion

    static metadb_index_client_t * Instance() noexcept;
    static void Initialize() noexcept;

    metadb_index_hash HashPathName(pfc::string pathName) noexcept;
    metadb_index_hash HashString(pfc::string s) noexcept;

private:
    hasher_md5::ptr _Hasher;
    titleformat_object_ptr _PinToScript;
};
