
/** $VER: ContextMenu.cpp (2024.07.18) **/

#include "pch.h"

#include <SDK/contextmenu.h>

#include "Resources.h"
#include "StatisticsManager.h"

#pragma hdrstop

namespace
{
    struct menu_item_t
    {
        const char * Text;
        const char * Description;
        GUID Id;
    };

    static const GUID ParentGUID    = {0x68452aaa,0x3b65,0x46c1,{0x86,0xa2,0xa6,0xc3,0x0d,0xb4,0x6d,0x57}}; // {68452aaa-3b65-46c1-86a2-a6c30db46d57}

    static const GUID ResetGUID     = {0x15d5e249,0x1b69,0x4ad1,{0xab,0x6f,0xd1,0xcb,0xa5,0x39,0x1d,0x3f}}; // {15d5e249-1b69-4ad1-ab6f-d1cba5391d3f}
    static const GUID WriteGUID     = {0xfcad381f,0x457a,0x44e1,{0xa1,0xe3,0x43,0x04,0x40,0x21,0x9d,0x07}}; // {fcad381f-457a-44e1-a1e3-430440219d07}
    static const GUID ReadGUID      = {0xafcbe911,0x9e04,0x4ea6,{0xa7,0x96,0xb3,0xe7,0x30,0x5f,0x4b,0xa1}}; // {afcbe911-9e04-4ea6-a796-b3e7305f4ba1}
    static const GUID MigrateGUID   = {0x2e0319a1,0xc5b1,0x444b,{0x8a,0x20,0xef,0xbc,0x45,0x54,0x70,0x3c}}; // {2e0319a1-c5b1-444b-8a20-efbc4554703c}
    static const GUID MarkGUID      = {0x8fe6aad8,0x558e,0x455b,{0x85,0x07,0xac,0xd7,0x71,0xf1,0x47,0xe4}};

    static const menu_item_t MenuItems[] =
    {
        { "Reset", "Resets the metadata of the selected tracks.", ResetGUID },
        { "Write to tags", "Writes the metadata of the selected tracks to file tags.", WriteGUID },
        { "Read from tags", "Reads the metadata of the selected tracks from file tags.", ReadGUID },
        { "Import", "Imports the foo_playcount tags as metadata.", MigrateGUID },
        { "Mark as played", "Marks the selected tracks as played.", MarkGUID },
    };

    static contextmenu_group_popup_factory _ContextMenuGroupPopupFactory(ParentGUID, contextmenu_groups::root, STR_COMPONENT_NAME, 0);

    class ContextMenu : public contextmenu_item_simple
    {
    public:
        ContextMenu() noexcept { };

        ContextMenu(const ContextMenu &) = delete;
        ContextMenu(ContextMenu &&) = delete;
        ContextMenu & operator=(const ContextMenu &) = delete;
        ContextMenu & operator=(ContextMenu &&) = delete;

        virtual ~ContextMenu() { };

        GUID get_parent() final
        {
            return ParentGUID;
        }

        uint32_t get_num_items() final
        {
            return (uint32_t) _countof(MenuItems);
        }

        void get_item_name(uint32_t index, pfc::string_base & out) final
        {
            if (index >= _countof(MenuItems))
                FB2K_BugCheck();

            out = MenuItems[index].Text;
        }

        void context_command(uint32_t index, metadb_handle_list_cref hTracks, const GUID &) final
        {
            switch (index)
            {
                case 0:
                {
                    statistics_manager_t::Reset(hTracks);
                    break;
                }

                case 1:
                {
                    statistics_manager_t::Write(hTracks);
                    break;
                }

                case 2:
                {
                    statistics_manager_t::Read(hTracks);
                    break;
                }

                case 3:
                {
                    statistics_manager_t::Import(hTracks);
                    break;
                }

                case 4:
                {
                    statistics_manager_t::MarkAsPlayed(hTracks);
                    break;
                }
            }
        }

        bool context_get_display(uint32_t index, metadb_handle_list_cref, pfc::string_base & out, uint32_t &, const GUID &) final
        {
            if (index >= _countof(MenuItems))
                FB2K_BugCheck();

            get_item_name(index, out);

            return true;
        }

        GUID get_item_guid(uint32_t index) final
        {
            if (index >= _countof(MenuItems))
                FB2K_BugCheck();

            return MenuItems[index].Id;
        }

        bool get_item_description(uint32_t index, pfc::string_base & out) final
        {
            if (index >= _countof(MenuItems))
                FB2K_BugCheck();

            out = MenuItems[index].Description;

            return true;
        }
    };

    FB2K_SERVICE_FACTORY(ContextMenu);

    static const GUID RatingGUID    = {0xba62bdc6,0x4000,0x45f6,{0x82,0xb1,0xd4,0x81,0x31,0x62,0x51,0x58}}; // {ba62bdc6-4000-45f6-82b1-d48131625158}

    static const GUID NoneGUID      = {0xf2a25ab3,0xa971,0x46ea,{0xa2,0x2f,0xd6,0x71,0x3c,0x4e,0x69,0xb4}}; // {f2a25ab3-a971-46ea-a22f-d6713c4e69b4}
    static const GUID OneGUID       = {0x4390684d,0x011e,0x43a1,{0xa7,0x37,0xf6,0x4d,0x01,0x0c,0x66,0x74}}; // {4390684d-011e-43a1-a737-f64d010c6674}
    static const GUID TwoGUID       = {0x8e1814e1,0x502c,0x43e0,{0x9e,0xf3,0xef,0x85,0x3c,0xf0,0xfc,0x77}}; // {8e1814e1-502c-43e0-9ef3-ef853cf0fc77}
    static const GUID ThreeGUID     = {0xf7df37a2,0xd9e7,0x416c,{0xb2,0xc8,0x6f,0x45,0x98,0xc5,0x48,0x13}}; // {f7df37a2-d9e7-416c-b2c8-6f4598c54813}
    static const GUID FourGUID      = {0x0b6cccc6,0xd6b8,0x4c8c,{0xb4,0xa5,0xd5,0x49,0x18,0x3d,0xef,0x69}}; // {0b6cccc6-d6b8-4c8c-b4a5-d549183def69}
    static const GUID FiveGUID      = {0xf9d9eed6,0x8d70,0x4265,{0x87,0xa3,0xaa,0x56,0x5e,0x50,0x47,0x04}}; // {f9d9eed6-8d70-4265-87a3-aa565e504704}

    static const GUID RatingGUIDs[] =
    {
        NoneGUID,

        OneGUID,
        TwoGUID,
        ThreeGUID,
        FourGUID,
        FiveGUID,
    };

    static contextmenu_group_popup_factory g_context_group_rating(RatingGUID, ParentGUID, "Rating", 0);

    class ContextMenuRating : public contextmenu_item_simple
    {
    public:
        ContextMenuRating() noexcept { };

        ContextMenuRating(const ContextMenuRating &) = delete;
        ContextMenuRating(ContextMenuRating &&) = delete;
        ContextMenuRating & operator=(const ContextMenuRating &) = delete;
        ContextMenuRating & operator=(ContextMenuRating &&) = delete;

        virtual ~ContextMenuRating() { };

        GUID get_parent() final
        {
            return RatingGUID;
        }

        uint32_t get_num_items() final
        {
            return _countof(RatingGUIDs);
        }

        void get_item_name(uint32_t index, pfc::string_base & out) final
        {
            if (index >= _countof(RatingGUIDs))
                FB2K_BugCheck();

            if (index == 0)
                out = "None";
            else
                out = pfc::format_int(index);
        }

        void context_command(uint32_t index, metadb_handle_list_cref hTracks, const GUID &) final
        {
            if (index >= _countof(RatingGUIDs))
                FB2K_BugCheck();

            statistics_manager_t::SetRating(hTracks, index);
        }

        bool context_get_display(uint32_t index, metadb_handle_list_cref, pfc::string_base & out, uint32_t &, const GUID &) final
        {
            if (index >= _countof(RatingGUIDs))
                FB2K_BugCheck();

            get_item_name(index, out);

            return true;
        }

        GUID get_item_guid(uint32_t index) final
        {
            if (index >= _countof(RatingGUIDs))
                FB2K_BugCheck();

            return RatingGUIDs[index];
        }

        bool get_item_description(uint32_t index, pfc::string_base & out) final
        {
            if (index >= _countof(RatingGUIDs))
                FB2K_BugCheck();

            if (index == 0)
                out = "Removes the rating from the selected tracks.";
            else
                out = pfc::format("Sets the rating of the selected tracks to ", index, ".");

            return true;
        }
    };

    FB2K_SERVICE_FACTORY(ContextMenuRating);
}
