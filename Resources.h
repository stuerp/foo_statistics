
/** $VER: Resources.h (2024.07.29) P. Stuer **/

#pragma once

#define TOSTRING_IMPL(x) #x
#define TOSTRING(x) TOSTRING_IMPL(x)

#define NUM_FILE_MAJOR          0
#define NUM_FILE_MINOR          1
#define NUM_FILE_PATCH          2
#define NUM_FILE_PRERELEASE     0

#define NUM_PRODUCT_MAJOR       0
#define NUM_PRODUCT_MINOR       1
#define NUM_PRODUCT_PATCH       2
#define NUM_PRODUCT_PRERELEASE  0

/** Component specific **/

#define STR_COMPONENT_NAME          "Statistics"
#define STR_COMPONENT_VERSION       TOSTRING(NUM_FILE_MAJOR) "." TOSTRING(NUM_FILE_MINOR) "." TOSTRING(NUM_FILE_PATCH) "." TOSTRING(NUM_FILE_PRERELEASE)
#define STR_COMPONENT_BASENAME      "foo_statistics"
#define STR_COMPONENT_FILENAME      STR_COMPONENT_BASENAME ".dll"
#define STR_COMPONENT_COMPANY_NAME  ""
#define STR_COMPONENT_COPYRIGHT     "Copyright (c) 2024 P. Stuer. All rights reserved."
#define STR_COMPONENT_COMMENTS      ""
#define STR_COMPONENT_DESCRIPTION   "Collects and maintains playback statistics."

/** Generic **/

#define STR_COMPANY_NAME        TEXT(STR_COMPONENT_COMPANY_NAME)
#define STR_INTERNAL_NAME       TEXT(STR_COMPONENT_NAME)
#define STR_COMMENTS            TEXT(STR_COMPONENT_COMMENTS)
#define STR_COPYRIGHT           TEXT(STR_COMPONENT_COPYRIGHT)

#define STR_FILE_NAME           TEXT(STR_COMPONENT_FILENAME)
#define STR_FILE_VERSION        TOSTRING(NUM_FILE_MAJOR) TEXT(".") TOSTRING(NUM_FILE_MINOR) TEXT(".") TOSTRING(NUM_FILE_PATCH) TEXT(".") TOSTRING(NUM_FILE_PRERELEASE)
#define STR_FILE_DESCRIPTION    TEXT(STR_COMPONENT_DESCRIPTION)

#define STR_PRODUCT_NAME        STR_INTERNAL_NAME
#define STR_PRODUCT_VERSION     TOSTRING(NUM_PRODUCT_MAJOR) TEXT(".") TOSTRING(NUM_PRODUCT_MINOR) TEXT(".") TOSTRING(NUM_PRODUCT_PATCH) TEXT(".") TOSTRING(NUM_PRODUCT_PRERELEASE)

#define STR_ABOUT_NAME          STR_INTERNAL_NAME
#define STR_ABOUT_WEB           TEXT("https://github.com/stuerp/") STR_COMPONENT_BASENAME
#define STR_ABOUT_EMAIL         TEXT("mailto:peter.stuer@outlook.com")

/** Window **/

#define GUID_PREFERENCES        { 0x29f3ee03, 0xa782, 0x4a2a, { 0xa2, 0xf1, 0x7b, 0xec, 0xb2, 0xaf, 0x6a, 0xb2 } }

/** Preferences **/

#define IDD_PREFERENCES                      101

#define IDC_PIN_TO                          1000
#define IDC_THRESHOLD_FORMAT                1002

#define IDC_RETENTION_VALUE                 1004
#define IDC_RETENTION_UNIT                  1006

#define IDC_WRITE_TO_TAGS                   1050
#define IDC_REMOVE_TAGS                     1051

#define IDC_TAGS_TO_REMOVE                  1052
