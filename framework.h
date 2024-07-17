
/** $VER: framework.h (2024.07.16) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 4738 5045 ALL_CPPCORECHECK_WARNINGS)

#define FOOBAR2000_TARGET_VERSION 82
#define NOMINMAX

#include <SDK/foobar2000-lite.h>

#include <SDKDDKVer.h>

#include <Windows.h>

#include <atlbase.h>
#include <atltypes.h>
#include <atlstr.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlwin.h>
#include <atlcom.h>
#include <atlcrack.h>

#include <algorithm>
#include <cmath>
#include <cassert>
#include <string>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define THIS_INSTANCE ((HINSTANCE) &__ImageBase)

#include <SDK/hasher_md5.h>
#include <SDK/metadb_index.h>

#include <pfc/list.h>
#include <set>

using hash_list_t = pfc::list_t<metadb_index_hash>;
using hash_set_t = std::set<metadb_index_hash>;
