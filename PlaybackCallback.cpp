
/** $VER: PlaybackCallback.cpp (2024.07.22) **/

#include "pch.h"

#include <SDK/playback_control.h>
#include <SDK/play_callback.h>
#include <SDK/titleformat.h>

#include "Configuration.h"
#include "StatisticsManager.h"

#include <math.h>

#include <queue>

#pragma hdrstop

namespace
{
    /// <summary>
    /// Handles the playback events we're subscribed to.
    /// </summary>
    class PlaybackCallback : public play_callback_static
    {
    public:
        PlaybackCallback() noexcept { };

        PlaybackCallback(const PlaybackCallback &) = delete;
        PlaybackCallback(PlaybackCallback &&) = delete;
        PlaybackCallback & operator=(const PlaybackCallback &) = delete;
        PlaybackCallback & operator=(PlaybackCallback &&) = delete;

        virtual ~PlaybackCallback() { };

        /// <summary>
        /// Controls which methods your callback wants called.
        /// </summary>
        virtual unsigned get_flags() final
        {
            return flag_on_playback_all; // flag_on_playback_new_track | flag_on_playback_time;
        }

    private:
        #pragma region play_callback

        /// <summary>
        /// Called when playback is being initialized.
        /// </summary>
        virtual void on_playback_starting(play_control::t_track_command, bool) final
        {
        }

        /// <summary>
        /// Called when playback advances to a new track.
        /// </summary>
        virtual void on_playback_new_track(metadb_handle_ptr hTrack) final
        {
            _StatisticsUpdated = false;
            _ThresholdTime = _Configuration.GetThresholdTime();
        }

        /// <summary>
        /// Called when playback stops.
        /// </summary>
        virtual void on_playback_stop(play_control::t_stop_reason) final
        {
        }

        /// <summary>
        /// Called when the user seeks to a specific time.
        /// </summary>
        virtual void on_playback_seek(double) final
        {
            PrintProgress();
        }

        /// <summary>
        /// Called when playback pauses or resumes.
        /// </summary>
        virtual void on_playback_pause(bool) final
        {
        }

        /// <summary>
        /// Called when the currently played file gets edited.
        /// </summary>
        virtual void on_playback_edited(metadb_handle_ptr) final
        {
        }

        /// <summary>
        /// Called when dynamic info (VBR bitrate etc...) changes.
        /// </summary>
        virtual void on_playback_dynamic_info(const file_info &) final
        {
        }

        /// <summary>
        /// Called when the per-track dynamic info (stream track titles etc...) change. Happens less often than on_playback_dynamic_info().
        /// </summary>
        virtual void on_playback_dynamic_info_track(const file_info &) final
        {
        }

        /// <summary>
        /// Called, every second, for time display.
        /// </summary>
        virtual void on_playback_time(double time) final
        {
            if (_StatisticsUpdated || (time < _ThresholdTime))
                return;

            metadb_handle_ptr hTrack;

            playback_control::get()->get_now_playing(hTrack);

            if (hTrack.is_empty())
                return;

            statistics_manager_t::OnItemPlayed(hTrack);

            _StatisticsUpdated = true;
        }

        /// <summary>
        /// Called when the user changes the volume.
        /// </summary>
        virtual void on_volume_change(float) final
        {
        }

        #pragma endregion

        void PrintProgress() noexcept;

    private:
        bool _StatisticsUpdated = false; // True if the statistics for the playing item have been updated.
        double _ThresholdTime = 0.;
        std::queue<metadb_handle_ptr> _TracksToPersist;

//      titleformat_object::ptr _TitleFormatScript;
    };

    /// <summary>
    /// Prints a playback progress message on the console.
    /// </summary>
    void PlaybackCallback::PrintProgress() noexcept
    {
/*
        if (_TitleFormatScript.is_empty())
        {
            pfc::string TitleFormatPattern = "%codec% | %bitrate% kbps | %samplerate% Hz | %channels% | %playback_time%[ / %length%]$if(%ispaused%, | paused,)";

            static_api_ptr_t<titleformat_compiler>()->compile_safe_ex(_TitleFormatScript, TitleFormatPattern);
        }

        auto pbc = playback_control::get();

        pfc::string_formatter Formatter;

        if (!pbc->playback_format_title(nullptr, Formatter, _TitleFormatScript, nullptr, playback_control::display_level_all))
        {
            if (pbc->is_playing())
                Formatter = "Opening..."; // Starting playback but not done opening the first track yet.

            else
                Formatter = "Stopped.";
        }

        console::print(Formatter);
*/
    }

    FB2K_SERVICE_FACTORY(PlaybackCallback);
}
