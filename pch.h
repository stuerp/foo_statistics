
/** $VER: pch.h (2026.03.14) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>

#define NOMINMAX

#include <winsock2.h>
#include <windows.h>

#include <atlbase.h>
#include <atltypes.h>
#include <atlstr.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlwin.h>
#include <atlcom.h>
#include <atlcrack.h>

#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <memory>
#include <ranges>
#include <set>
#include <string>

#include <sdk\foobar2000-lite.h>

#include <sdk\hasher_md5.h>
#include <sdk\metadb_index.h>

#include <pfc\list.h>

using hash_list_t = pfc::list_t<metadb_index_hash>;
using hash_set_t = std::set<metadb_index_hash>;

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define THIS_INSTANCE ((HINSTANCE) &__ImageBase)
