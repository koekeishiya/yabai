<!-- Please be careful editing the below HTML, as GitHub is quite finicky with anything that looks like an HTML tag in GitHub Flavored Markdown. -->
<p align="center">
  <img width="75%" src="assets/banner/banner.svg" alt="Banner">
</p>
<p align="center">
  <b>Tiling window management for the Mac.</b>
</p>
<p align="center">
  <a href="https://github.com/koekeishiya/yabai/blob/master/LICENSE.txt">
    <img src="https://img.shields.io/github/license/koekeishiya/yabai.svg?color=green" alt="License Badge">
  </a>
  <a href="https://travis-ci.org/koekeishiya/yabai">
    <img src="https://travis-ci.org/koekeishiya/yabai.svg?branch=master" alt="CI Status Badge">
  </a>
  <a href="https://github.com/koekeishiya/yabai/blob/master/README.md#changelog">
    <img src="https://img.shields.io/badge/view-changelog-green.svg" alt="Changelog Badge">
  </a>
  <a href="https://github.com/koekeishiya/yabai/releases">
    <img src="https://img.shields.io/github/commits-since/koekeishiya/yabai/latest.svg?color=green" alt="Version Badge">
  </a>
</p>

## About

<img align="right" width="40%" src="assets/screenshot.png" alt="Screenshot">

yabai is a window management utility that is designed to work as an extension to the built-in window manager of macOS.
yabai allows you to control your windows, spaces and displays freely using an intuitive command line interface and optionally set user-defined keyboard shortcuts using tight integration with [&nearr;&nbsp;skhd](gh-skhd) and other third-party software.

The primary function of yabai is tiling window management, which makes it automatically modify your window layout using a binary space partitioning algorithm to allow you to focus on the content of your windows without distractions, so you can get more work done without having to think about arranging windows.
Additional features of yabai include focus-follows-mouse, disabling animations for switching spaces, creating spaces past the limit of 16 spaces, and much more.

yabai started as a C99 rewrite of [&nearr;&nbsp;chunkwm][gh-chunkwm], originally supposed to be its first RC version.
However due to major architectural changes, supported systems, and changes to functionality, it is being released separately.

The experience gained through experimenting with, designing, and using both kwm and chunkwm lead to a clear vision of how window management on macOS should function: seamless integration with the window server (when possible), quick to respond, low battery usage, proper error reporting, and high customizability where possible.

An optional scripting addition unlocks further features of yabai, but requires (partially) disabling System Integrity Protection.

## Installation and Configuration

- The [&nearr;&nbsp;yabai&nbsp;wiki][yabai-wiki] has both brief and detailed installation instuctions for multiple installation methods, and also explains how to uninstall yabai completely.
- Sample configuration files can be found in the [&nearr;&nbsp;examples][yabai-examples] directory. Refer to the [&nearr;&nbsp;documentation][yabai-docs] or the wiki for further information.
- Keyboard shortcuts can be defined with [&nearr;&nbsp;skhd][gh-skhd] or any other suitable software you may prefer.

## Requirements and Caveats

Please read the below requirements carefully.
Make sure you fullfil all all of them before filing an issue.

|Requirement|Note|
|-:|:-|
|Operating&nbsp;System|macOS&nbsp;High&nbsp;Sierra&nbsp;10.13.6, macOS&nbsp;Mojave&nbsp;10.14.4+ are supported. macOS&nbsp;Catalina&nbsp;Beta is mostly supported.|
|Accessibility&nbsp;API|yabai must be given permission to utilize the Accessibility API and will request access upon launch. The application must be restarted after access has been granted.|
|Mission&nbsp;Control|In the Mission Control preferences pane in System Preferences, the setting "Displays have separate Spaces" must be enabled.|

Please also take note of the following caveats.

|Caveat|Note|
|-:|:-|
|System&nbsp;Integrity&nbsp;Protection|System Integrity Protection needs to be (partially) disabled for yabai to inject a scripting addition into Dock.app for controlling windows with functions that require elevated privileges. This enables control of the window server, which is the sole owner of all window connections, and enables additional features of yabai.|
|Code&nbsp;Signing|When building from source (or installing from HEAD), it is recommended to codesign the binary so it retains its accessibility and automation privileges when updated or rebuilt.|
|Mission&nbsp;Control|In the Mission Control preferences pane in System Preferences, the setting "Automatically rearrange Spaces based on most recent use" should be enabled for commands that rely on the ordering of spaces to work reliably.|

## Changelog

<!-- NOTE: When releasing a new version, move the previous versions changelog
into the details tag. Make sure to add the link to the commit list at the
bottom of this file -->

All notable changes to this project will be documented here. This project
adheres to [Semantic Versioning][external-semver].

### [Unreleased]

### [2.0.1] - 2019-09-04
#### Changed
- *window_opacity_duration* was a *copy-pasta* job [#208](https://github.com/koekeishiya/yabai/issues/208)

<details>
<summary><b>Click to expand previous changes</b></summary>

### [2.0.0] - 2019-09-03
#### Added
- Commands to toggle mission-control, show-desktop, and application expose [#147](https://github.com/koekeishiya/yabai/issues/147)
- Command to close windows that provide a close button in its titlebar [#84](https://github.com/koekeishiya/yabai/issues/84)
- Expose window shadow as a property in window queries and add shadow as an option to *window --toggle* [#171](https://github.com/koekeishiya/yabai/issues/171)
- Config option to set the duration of the transition between active / normal window opacity [#208](https://github.com/koekeishiya/yabai/issues/208)

#### Changed
- Automatically restart Dock.app after installing the scripting-addition, and tweak messages shown when a payload gets loaded, is already loaded or does not support the version of macOS it's running on [#135](https://github.com/koekeishiya/yabai/issues/135)
- Work around macOS craziness so that we can properly tile a window after it leaves native fullscreen mode [#36](https://github.com/koekeishiya/yabai/issues/36)
- Return an error for queries with invalid, named selectors [#158](https://github.com/koekeishiya/yabai/issues/158)
- Resolve a potential multi-threaded issue due to "undefined behaviour" regarding x86 instruction ordering [#153](https://github.com/koekeishiya/yabai/issues/153)
- Fix space padding and gap underflow when modified with a relative value [#141](https://github.com/koekeishiya/yabai/issues/141)
- Window_* signals no longer pass the application pid [#154](https://github.com/koekeishiya/yabai/issues/154)
- Ignore all windows that report a main role of AXPopover [#162](https://github.com/koekeishiya/yabai/issues/162)
- Ignore all windows that report a sub role of AXUnknown [#164](https://github.com/koekeishiya/yabai/issues/164)
- Track when the Dock changes preferences [#147](https://github.com/koekeishiya/yabai/issues/147)
- Properly detect when mission-control is deactivated [#169](https://github.com/koekeishiya/yabai/issues/169)
- Pass arguments to signals through environment variables instead [#167](https://github.com/koekeishiya/yabai/issues/167)
- Revert change that made the status bar draw above other windows because of compatibility with "windowed fullscreen" applications [#170](https://github.com/koekeishiya/yabai/issues/170)
- Warping a window should respect insert direction in scenarios where the default warp is equal to a swap operation [#146](https://github.com/koekeishiya/yabai/issues/146)
- Exiting mission-control will invalidate the region assigned to Spaces/Views as because a Space may have been dragged to a different monitor [#118](https://github.com/koekeishiya/yabai/issues/118)
- Application_Launched signal incorrectly fired multiple times due to accessibility retries [#175](https://github.com/koekeishiya/yabai/issues/175)
- Global space settings should properly apply again [#176](https://github.com/koekeishiya/yabai/issues/176)
- Fixed an issue that could cause windows to be overlapped when tiled [#183](https://github.com/koekeishiya/yabai/issues/183)
- Verify that a message was given before trying to connect to the running yabai instance [#197](https://github.com/koekeishiya/yabai/issues/197)
- Properly re-zoom a window when toggling its border [#211](https://github.com/koekeishiya/yabai/issues/211)
- Workaround to make sure a window actually set the proper dimensions [#226](https://github.com/koekeishiya/yabai/issues/226) [#188](https://github.com/koekeishiya/yabai/issues/188)
- Moving window to a different space using rules could leave an empty tile [#232](https://github.com/koekeishiya/yabai/issues/232)
- Windows spawned while the owning application is hidden should not cause an empty tile to be created [#233](https://github.com/koekeishiya/yabai/issues/233)
- Properly handle destruction and re-creation of view when layout changes between bsp and float [#230](https://github.com/koekeishiya/yabai/issues/230)

### [1.1.2] - 2019-07-15
#### Changed
- Float native macOS fullscreen spaces [#130](https://github.com/koekeishiya/yabai/issues/130)
- Write error messsages returned to the client through *stderr* [#131](https://github.com/koekeishiya/yabai/issues/131)
- Allow window focus command to work without the existence of a focused window [#133](https://github.com/koekeishiya/yabai/issues/133)

### [1.1.1] - 2019-07-14
#### Changed
- The status bar should be disabled by default, if setting is missing from the config [#126](https://github.com/koekeishiya/yabai/issues/126)

### [1.1.0] - 2019-07-14
#### Added
- Make loading scripting-addition more robust - validating version and functionality [#108](https://github.com/koekeishiya/yabai/issues/108)
- Merge options for query constraints with command selectors to have a unified method for addressing displays, spaces, and windows, as well as allowing commands to specify both a *selected* and a *given entity* of its type [#112](https://github.com/koekeishiya/yabai/issues/112)

#### Changed
- Dragging a tiled window to another display using the mouse will cause the window to be warped to that display upon release [#103](https://github.com/koekeishiya/yabai/issues/103)
- Escape quotes in window titles returned through query commands [#114](https://github.com/koekeishiya/yabai/issues/114)
- Extend window, space and display properties exposed through *query* commands [#116](https://github.com/koekeishiya/yabai/issues/116)
- Native macOS fullscreen spaces can now be addressed using their mission-control index, and can also be moved [#117](https://github.com/koekeishiya/yabai/issues/117)
- Opacity changes only apply to windows that properly identify as "standard" or "dialog" windows [#120](https://github.com/koekeishiya/yabai/issues/120)
- The status bar should now properly draw above any potential window that overlaps its frame [#124](https://github.com/koekeishiya/yabai/issues/124)
- Support *XDG_CONFIG_HOME* when locating the config file [#125](https://github.com/koekeishiya/yabai/issues/125)
- Allow every single config setting to be applied at runtime with immediate effect. Run config asynchronously after initialization [#122](https://github.com/koekeishiya/yabai/issues/122)

### [1.0.6] - 2019-07-09
#### Changed
- No longer necessary to depend on the scripting-addition to focus a window [#102](https://github.com/koekeishiya/yabai/issues/102)
- Extend definition of *WINDOW_SEL* to include *smallest* and *largest* [#105](https://github.com/koekeishiya/yabai/issues/105)

### [1.0.5] - 2019-07-07
#### Changed
- Fix missing quotation of string value outputted through *window query* commands [#90](https://github.com/koekeishiya/yabai/issues/90)

### [1.0.4] - 2019-07-06
#### Changed
- Fixed an issue that prevented *yabai* from running under multiple users simultaneously [#95](https://github.com/koekeishiya/yabai/issues/95)
- Extend window properties exposed through *query* commands [#90](https://github.com/koekeishiya/yabai/issues/90)
- Extend definition of *WINDOW_SEL*, *SPACE_SEL* and *DISPLAY_SEL* to include *first*, *last*, and *recent* [#85](https://github.com/koekeishiya/yabai/issues/85)

### [1.0.3] - 2019-06-30
#### Changed
- Prevent *status_bar* and *window borders* from displaying in native fullscreen spaces [#71](https://github.com/koekeishiya/yabai/issues/71)
- Fixed an issue with the *status_bar* where *has_battery* and *charging* would not be default initialized when macOS report that there are zero power sources [#60](https://github.com/koekeishiya/yabai/issues/60)
- Properly destroy the underlaying *view* when a *space* changes layout [#81](https://github.com/koekeishiya/yabai/issues/81)

### [1.0.2] - 2019-06-25
#### Changed
- Hide power indicator from the status-bar if a battery could not be found [#60](https://github.com/koekeishiya/yabai/issues/60)
- Disable focus follows mouse while the *mouse_modifier* key is held down [#62](https://github.com/koekeishiya/yabai/issues/62)
- Silence meaningless warning reported by the scripting-bridge framework [#55](https://github.com/koekeishiya/yabai/issues/55)
- Extend definition of *WINDOW_SEL* to include *mouse*, targetting the window below the cursor [#66](https://github.com/koekeishiya/yabai/issues/66)
- Allow all *config* (except *status_bar*) settings to be edited at runtime [#69](https://github.com/koekeishiya/yabai/issues/69)
- Window should not be added to the window-tree twice when deminimized on an inactive display [#70](https://github.com/koekeishiya/yabai/issues/70)
- Expose *window_placement* as a config setting to specify if windows become the first or second child [#65](https://github.com/koekeishiya/yabai/issues/65)

### [1.0.1] - 2019-06-23
#### Added
- First official release

</details>

## License and Attribution

yabai is licensed under the [&nearr;&nbsp;MIT&nbsp;License][yabai-license], a short and simple permissive license with conditions only requiring preservation of copyright and license notices.
Licensed works, modifications, and larger works may be distributed under different terms and without source code.

Thanks to [@fools-mate](gh-fools-mate) for creating logo and banner for this project and making them available for free.

## Disclaimer

Use at your own discretion.
I take no responsibility if anything should happen to your machine while trying to install, test or otherwise use this software in any form.
You acknowledge that you understand the potential risk that may come from disabling [&nearr;&nbsp;System&nbsp;Integrity&nbsp;Protection][external-about-sip] on your system, and I make no recommendation as to whether you should or should not disable System Integrity Protection.

<!-- Changelog commit range links -->
[Unreleased]: https://github.com/koekeishiya/yabai/compare/v2.0.1...HEAD
[2.0.1]: https://github.com/koekeishiya/yabai/compare/v2.0.0...v2.0.1
[2.0.0]: https://github.com/koekeishiya/yabai/compare/v1.1.2...v2.0.0
[1.1.2]: https://github.com/koekeishiya/yabai/compare/v1.1.1...v1.1.2
[1.1.1]: https://github.com/koekeishiya/yabai/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/koekeishiya/yabai/compare/v1.0.6...v1.1.0
[1.0.6]: https://github.com/koekeishiya/yabai/compare/v1.0.5...v1.0.6
[1.0.5]: https://github.com/koekeishiya/yabai/compare/v1.0.4...v1.0.5
[1.0.4]: https://github.com/koekeishiya/yabai/compare/v1.0.3...v1.0.4
[1.0.3]: https://github.com/koekeishiya/yabai/compare/v1.0.2...v1.0.3
[1.0.2]: https://github.com/koekeishiya/yabai/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/koekeishiya/yabai/releases/tag/v1.0.1

<!-- Project internal links -->
[yabai-license]: LICENSE.txt
[yabai-examples]: https://github.com/koekeishiya/yabai/tree/master/examples
[yabai-wiki]: https://github.com/koekeishiya/yabai/wiki
[yabai-docs]: https://github.com/koekeishiya/yabai/blob/master/doc/yabai.asciidoc

<!-- Links to other GitHub projects/users -->
[gh-skhd]: https://github.com/koekeishiya/skhd
[gh-chunkwm]: https://github.com/koekeishiya/chunkwm
[gh-fools-mate]: https://github.com/fools-mate

<!-- External links -->
[external-about-sip]: https://support.apple.com/en-us/HT204899
[external-semver]: https://semver.org/spec/v2.0.0.html

