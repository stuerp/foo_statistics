
# foo_statistics

[foo_statistics](https://github.com/stuerp/foo_statistics/releases) is a [foobar2000](https://www.foobar2000.org/) component that collects and maintains statistics about tracks in the media library.

The statistics can optionally be written to file tags. The statistics from [foo_playcount](https://www.foobar2000.org/components/view/foo_playcount) can be imported from its tags and optionally removed.

It was created because the official [foo_playcount](https://www.foobar2000.org/components/view/foo_playcount) component changed its behavior when it was released as 64-bit.

## Features

* Configurable tracking pattern
* Configurable period after which a track is considered 'played'.
* Can import tags from the official foo_playcount component.
* Collected statistics:
  * Timestamp when the track gets added to the Media Library
  * Timestamp each time the track gets played.
  * Track rating
* Supports dark mode.

## Requirements

* Tested on Microsoft Windows 10 and later.
* [foobar2000](https://www.foobar2000.org/download) v2.0 or later (32 or 64-bit). ![foobar2000](https://www.foobar2000.org/button-small.png)

## Getting started

### Installation

* Double-click `foo_statistics.fbk2-component`.

or

* Import `foo_statistics.fbk2-component` into foobar2000 using the "*File / Preferences / Components / Install...*" menu item.

### Configuration

None

### Metadata

The following metadata is available:

%_added%
    Timestamp at which the track was added to the Media Library, human readable.

%_added_timestamp%
    Timestamp at which the track was added to the Media Library.

%_first_played%
    Timestamp at which the track was played the first time, human readable.

%_first_played_timestamp%
    Timestamp at which the track was played the first time.

%_last_played%
    Timestamp at which the track was played the last time, human readable.

%_last_played_timestamp%
    Timestamp at which the track was played the last time.

%_playcount%
    How many times the track has been played.

%_rating%
    The track's rating.

### Tags

The following tags are available:

%_timestamps%
    A semicolon-separate list of all the collected timestamps. The first entry is the timestamp when the track was added to the media library. The following entries are the timestamps of when the track was played.

%_rating%
    The track's rating.

## Developing

To build the code you need:

* [Microsoft Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com/downloads/) or later
* [foobar2000 SDK](https://www.foobar2000.org/SDK) 2023-05-10
* [Windows Template Library (WTL)](https://github.com/Win32-WTL/WTL) 10.0.10320

To create the deployment package you need:

* [PowerShell 7.2](https://github.com/PowerShell/PowerShell) or later

### Setup

Create the following directory structure:

    3rdParty
        WTL10_10320
    bin
        x86
    foo_statistics
    out
    sdk

* `3rdParty/WTL10_10320` contains WTL 10.0.10320.
* `bin` contains a portable version of foobar2000 64-bit for debugging purposes.
* `bin/x86` contains a portable version of foobar2000 32-bit for debugging purposes.
* `foo_statistics` contains the [Git](https://github.com/stuerp/foo_statistics) repository.
* `out` receives a deployable version of the component.
* `sdk` contains the foobar2000 SDK.

### Building

Open `foo_statistics.sln` with Visual Studio and build the solution.

### Packaging

To create the component first build the x86 configuration and next the x64 configuration.

## Change Log

### v0.1.2.1, 2024-08-23

* FIXED: Tracks that are not part of the media library received play timestamps.

### v0.1.2.0, 2024-08-10

* Initial release.

## Acknowledgements / Credits

* Peter Pawlowski for the [foobar2000](https://www.foobar2000.org/) audio player. ![foobar2000](https://www.foobar2000.org/button-small.png)
* [marc2k3](https://github.com/marc2k3) for the releasing the source code of [foo_playcount_2003](https://github.com/marc2k3/foo_playcount_2003)

## Reference Material

### foobar2000

* [foobar2000 Development](https://wiki.hydrogenaud.io/index.php?title=Foobar2000:Development:Overview)

### Windows User Interface

* [Desktop App User Interface](https://learn.microsoft.com/en-us/windows/win32/windows-application-ui-development)
* [Windows User Experience Interaction Guidelines](https://learn.microsoft.com/en-us/windows/win32/uxguide/guidelines)
* [Windows Controls](https://learn.microsoft.com/en-us/windows/win32/controls/window-controls)
* [Control Library](https://learn.microsoft.com/en-us/windows/win32/controls/individual-control-info)
* [Resource-Definition Statements](https://learn.microsoft.com/en-us/windows/win32/menurc/resource-definition-statements)
* [Visuals, Layout](https://learn.microsoft.com/en-us/windows/win32/uxguide/vis-layout)

## Links

* Home page: [https://github.com/stuerp/foo_statistics](https://github.com/stuerp/foo_statistics)
* Repository: [https://github.com/stuerp/foo_statistics.git](https://github.com/stuerp/foo_statistics.git)
* Issue tracker: [https://github.com/stuerp/foo_statistics/issues](https://github.com/stuerp/foo_statistics/issues)

## License

![License: MIT](https://img.shields.io/badge/license-MIT-yellow.svg)
