
/** $VER: MetaDbClient.cpp (2024.07.18) **/

#include "pch.h"

#include "MetaDbIndexClient.h"
#include "StatisticsManager.h"
#include "Resources.h"
#include "GUIDS.h"

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
MetaDbIndexClient::MetaDbIndexClient() : _Hasher(hasher_md5::get())
{
    titleformat_compiler::get()->compile_safe(_PinToScript, _Configuration.PinTo);
}

#pragma region metadb_index_client

/// <summary>
/// Transforms the track information (location + metadata) to a hash for the Meta database index manager.
/// </summary>
metadb_index_hash MetaDbIndexClient::transform(const file_info & fileInfo, const playable_location & location)
{
    pfc::string s;

    _PinToScript->run_simple(location, &fileInfo, s);

    return HashString(s);
}

#pragma endregion

#pragma region static

/// <summary>
/// Gets the singleton instance of the service.
/// </summary>
MetaDbIndexClient * MetaDbIndexClient::Instance() noexcept
{
    static MetaDbIndexClient * _Singleton = new service_impl_single_t<MetaDbIndexClient>();

    return _Singleton;
}

/// <summary>
/// Initializes the service.
/// </summary>
void MetaDbIndexClient::Initialize() noexcept
{
    auto Manager = StatisticsManager::GetMetaDbIndexManager();

//  Manager->erase_orphaned_data(GUIDs::MetaDbGUID); // Deletes no longer needed index user data files.

    try
    {
        Manager->add(Instance(), MetaDbGUID, _Configuration.RetentionPeriod);
        Manager->dispatch_global_refresh(); // Dispatches a global refresh, asks all components to refresh all tracks. To be calling after adding/removing indexes.
    }
    catch (const std::exception & e)
    {
        Manager->remove(MetaDbGUID);

        console::print(STR_COMPONENT_BASENAME " failed to initialize database: ", e.what());
    }
}

#pragma endregion

/// <summary>
/// Hashes the specified path name.
/// </summary>
metadb_index_hash MetaDbIndexClient::HashPathName(pfc::string pathName) noexcept
{
    return HashString(pathName + "|0");
}

/// <summary>
/// Hashes the specified string.
/// </summary>
metadb_index_hash MetaDbIndexClient::HashString(pfc::string s) noexcept
{
    return _Hasher->process_single_string(s).xorHalve();
}
