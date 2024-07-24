
/** $VER: ContextMenu.cpp (2024.07.24) **/

#include "pch.h"

#include <SDK/contextmenu.h>
#include <SDK/ui.h>

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
        std::function<void (metadb_handle_list_cref hTracks)> Handler;
    };

    static const GUID ParentGUID        = {0x68452aaa,0x3b65,0x46c1,{0x86,0xa2,0xa6,0xc3,0x0d,0xb4,0x6d,0x57}}; // {68452aaa-3b65-46c1-86a2-a6c30db46d57}

    static const GUID MarkGUID          = {0x8fe6aad8,0x558e,0x455b,{0x85,0x07,0xac,0xd7,0x71,0xf1,0x47,0xe4}};
    static const GUID WriteGUID         = {0xfcad381f,0x457a,0x44e1,{0xa1,0xe3,0x43,0x04,0x40,0x21,0x9d,0x07}}; // {fcad381f-457a-44e1-a1e3-430440219d07}
    static const GUID ReadGUID          = {0xafcbe911,0x9e04,0x4ea6,{0xa7,0x96,0xb3,0xe7,0x30,0x5f,0x4b,0xa1}}; // {afcbe911-9e04-4ea6-a796-b3e7305f4ba1}
    static const GUID ImportGUID        = {0x2e0319a1,0xc5b1,0x444b,{0x8a,0x20,0xef,0xbc,0x45,0x54,0x70,0x3c}}; // {2e0319a1-c5b1-444b-8a20-efbc4554703c}
    static const GUID ResetGUID         = {0x15d5e249,0x1b69,0x4ad1,{0xab,0x6f,0xd1,0xcb,0xa5,0x39,0x1d,0x3f}}; // {15d5e249-1b69-4ad1-ab6f-d1cba5391d3f}

    static const GUID PreferencesGUID   = {0x757b6a07,0x3f64,0x4ccc,{0xab,0x09,0x3c,0xcf,0xd2,0x4a,0x3a,0x91}};

    static const menu_item_t MenuItems[] =
    {
        { "Mark as played", "Marks the selected tracks as played.", MarkGUID, statistics_manager_t::MarkAsPlayed },
        { "Reset", "Resets the metadata of the selected tracks.", ResetGUID, statistics_manager_t::Reset },

        { "Write to tags", "Writes the metadata of the selected tracks to file tags.", WriteGUID, statistics_manager_t::WriteToTags },
        { "Read from tags", "Reads the metadata of the selected tracks from file tags.", ReadGUID, statistics_manager_t::ReadFromTags },
        { "Import", "Imports the foo_playcount tags as metadata.", ImportGUID, statistics_manager_t::ImportFromPlayCount },

        { "Preferences...", "Opens the " STR_COMPONENT_NAME " preferences page.", PreferencesGUID, [](metadb_handle_list_cref) { static_api_ptr_t<ui_control>()->show_preferences(GUID_PREFERENCES); }, }
    };

    static const GUID RatingGUID    = { 0xd445a8db, 0xdfb6, 0x442d, { 0xa3, 0xc8, 0x72, 0x74, 0x4, 0x3c, 0x92, 0x2f } };

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

    /// <summary>
    /// Implements a menu item of the Rating submenu.
    /// </summary>
    class RatingItem : public contextmenu_item_node_leaf
    {
    public:
        RatingItem(size_t rating) : _Rating(rating) { }

        RatingItem(const RatingItem &) = delete;
        RatingItem(RatingItem &&) = delete;
        RatingItem & operator=(const RatingItem &) = delete;
        RatingItem & operator=(RatingItem &&) = delete;

        virtual ~RatingItem() { };

        bool get_display_data(pfc::string_base & text, unsigned & flags, metadb_handle_list_cref hTracks, const GUID & caller)
        {
            if (_Rating >= 6)
                FB2K_BugCheck();

            if (_Rating == 0)
                text = "None";
            else
                text = pfc::format_int((t_int64) _Rating);

            flags = 0;

            return true;
        }

        void execute(metadb_handle_list_cref hTracks, const GUID & caller)
        {
            if (_Rating >= 6)
                FB2K_BugCheck();

            statistics_manager_t::SetRating(hTracks, (uint32_t) _Rating);
        }

        bool get_description(pfc::string_base & text)
        {
            if (_Rating >= 6)
                FB2K_BugCheck();

            if (_Rating == 0)
                text = "Removes the rating from the selected tracks.";
            else
                text = pfc::format("Sets the rating of the selected tracks to ", _Rating, ".");

            return true;
        }

        GUID get_guid()
        {
            if (_Rating >= _countof(RatingGUIDs))
                FB2K_BugCheck();

            return RatingGUIDs[_Rating];
        }

        bool is_mappable_shortcut()
        {
            return true;
        }

    private:
        size_t _Rating;
    };

    /// <summary>
    /// Implements the Rating submenu.
    /// </summary>
    class RatingMenu : public contextmenu_item_node_popup
    {
    public:
        RatingMenu() { };

        RatingMenu(const RatingMenu &) = delete;
        RatingMenu(RatingMenu &&) = delete;
        RatingMenu & operator=(const RatingMenu &) = delete;
        RatingMenu & operator=(RatingMenu &&) = delete;

        virtual ~RatingMenu() { };

        bool get_display_data(pfc::string_base & displayText, unsigned & displayFlags, metadb_handle_list_cref hTracks, const GUID & caller)
        {
            displayText = "Rating";

            return true;
        }

        t_size get_children_count()
        {
            return 6;
        }

        contextmenu_item_node * get_child(t_size index)
        {
            return new RatingItem(index);
        }

        GUID get_guid()
        {
            return RatingGUID;
        }
/*
        void execute(metadb_handle_list_cref p_data, const GUID & p_caller)
        {
        }

        bool get_description(pfc::string_base & text)
        {
            text = "Sets the rating of the selected tracks.";

            return true;
        }
*/
        bool is_mappable_shortcut()
        {
            return false;
        }
    };

    /// <summary>
    /// Implements a menu item of our submenu.
    /// </summary>
    class MenuItem : public contextmenu_item_node_leaf
    {
    public:
        MenuItem(size_t index) : _Index(index) { }

        MenuItem(const MenuItem &) = delete;
        MenuItem(MenuItem &&) = delete;
        MenuItem & operator=(const MenuItem &) = delete;
        MenuItem & operator=(MenuItem &&) = delete;

        virtual ~MenuItem() { };

        bool get_display_data(pfc::string_base & text, unsigned & flags, metadb_handle_list_cref hTracks, const GUID & caller)
        {
            if (_Index >= _countof(MenuItems))
                FB2K_BugCheck();
/*
        if (!(pfc::guid_equal(caller, contextmenu_item::caller_active_playlist_selection) || pfc::guid_equal(caller, contextmenu_item::caller_now_playing)))
            return false;
*/
            text = MenuItems[_Index].Text;

            flags = 0;

            return true;
        }

        void execute(metadb_handle_list_cref hTracks, const GUID & caller)
        {
            if (_Index >= _countof(MenuItems))
                FB2K_BugCheck();

            MenuItems[_Index].Handler(hTracks);
        }

        bool get_description(pfc::string_base & text)
        {
            if (_Index >= _countof(MenuItems))
                FB2K_BugCheck();

            text = MenuItems[_Index].Description;

            return true;
        }

        GUID get_guid()
        {
            if (_Index >= _countof(MenuItems))
                FB2K_BugCheck();

            return MenuItems[_Index].Id;
        }

        bool is_mappable_shortcut()
        {
            return true;
        }

    private:
        size_t _Index;
    };

    /// <summary>
    /// Implements our submenu.
    /// </summary>
    class Submenu : public contextmenu_item_node_root_popup
    {
    public:
        Submenu(unsigned index) : _Index(index) { }

        Submenu(const Submenu &) = delete;
        Submenu(Submenu &&) = delete;
        Submenu & operator=(const Submenu &) = delete;
        Submenu & operator=(Submenu &&) = delete;

        virtual ~Submenu() { };

        bool get_display_data(pfc::string_base & text, unsigned & flags, metadb_handle_list_cref hTracks, const GUID & caller)
        {
            text = STR_COMPONENT_NAME;
            flags = 0;

            return true;
        }

        t_size get_children_count()
        {
            return 1 /* submenu */ + 3 /* separators */ + _countof(MenuItems);
        }

        contextmenu_item_node * get_child(t_size index)
        {
            switch (index)
            {
                case 0: return new RatingMenu();

                case 1: return new contextmenu_item_node_separator();

                case 2: return new MenuItem(0);
                case 3: return new MenuItem(1);

                case 4: return new contextmenu_item_node_separator();

                case 5: return new MenuItem(2);
                case 6: return new MenuItem(3);
                case 7: return new MenuItem(4);

                case 8: return new contextmenu_item_node_separator();

                case 9: return new MenuItem(5);

                default: return nullptr;
            }
        }

        GUID get_guid()
        {
            return pfc::guid_null;
        }

        bool is_mappable_shortcut()
        {
            return false;
        }

    private:
        unsigned _Index;
    };

    /// <summary>
    /// Implements the menu item in the context menu that opens our submenu.
    /// </summary>
    class RootItem : public contextmenu_item_v2
    {
    public:
        RootItem() { }

        RootItem(const RootItem &) = delete;
        RootItem(RootItem &&) = delete;
        RootItem & operator=(const RootItem &) = delete;
        RootItem & operator=(RootItem &&) = delete;

        virtual ~RootItem() { };

        virtual unsigned get_num_items()
        {
            return 1;
        }

        virtual contextmenu_item_node_root * instantiate_item(unsigned index, metadb_handle_list_cref hTracks, const GUID & caller)
        {
            return new Submenu(index);
        }

        virtual GUID get_item_guid(unsigned index)
        {
            return ParentGUID;
        }

        virtual void get_item_name(unsigned index, pfc::string_base & text)
        {
            text = STR_COMPONENT_NAME;
        }

        virtual void get_item_default_path(unsigned index, pfc::string_base & text)
        {
            text = "";
        }

        virtual bool get_item_description(unsigned index, pfc::string_base & text)
        {
            text = "";

            return true;
        }

        virtual t_enabled_state get_enabled_state(unsigned index)
        {
            return contextmenu_item::DEFAULT_ON;
        }

        virtual void item_execute_simple(unsigned p_index, const GUID & p_node, metadb_handle_list_cref p_data, const GUID & p_caller) { }

        virtual double get_sort_priority()
        {
            return 0.;
        }

        virtual GUID get_parent()
        {
            return contextmenu_groups::root;
        }
    };

    FB2K_SERVICE_FACTORY(RootItem);
}
