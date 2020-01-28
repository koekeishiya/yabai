# Changelog

All notable changes to this project will be documented in this file.

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [2.2.0] - 2020-01-19
### Added
- Ability to do real picture-in-picture of any arbitrary window [#286](https://github.com/koekeishiya/yabai/issues/286)

### Changed
- Disable live-resizing of bsp-layout when using mouse-drag so that we are consistent with other mouse-interactions [#341](https://github.com/koekeishiya/yabai/issues/341)
- Improved spin-lock while waiting for native-fullscreen enter/exit animation to end [#339](https://github.com/koekeishiya/yabai/issues/339)
- New attribute uuid returned through display queries [#346](https://github.com/koekeishiya/yabai/issues/346)
- Ignore window moved events for windows in native-fullscreen mode. This is necessary because of weird macOS behaviour where a window in fullscreen-mode would trigger a window_moved event upon entering mission-control [#347](https://github.com/koekeishiya/yabai/issues/347)
- Allow borders to join native-fullscreen spaces because some applications create sub-windows that are visible in fullscreen spaces [#353](https://github.com/koekeishiya/yabai/issues/353)
- Ignore mouse-up event after clicking on a window maximize button [#354](https://github.com/koekeishiya/yabai/issues/354)
- Proper error reporting for rules and signals [#357](https://github.com/koekeishiya/yabai/issues/357)
- Properly preserve layout of spaces belonging to 4k and 5k displays after waking from sleep [#259](https://github.com/koekeishiya/yabai/issues/259)
- Add guard to make sure that we cannot incorrectly tile a window twice in case of weird macOS event behaviour [#348](https://github.com/koekeishiya/yabai/issues/348)
- Properly detect when mission-control is exited (with dock visible, hidden, and while a fullscreen space is active) [#319](https://github.com/koekeishiya/yabai/issues/319)

## [2.1.3] - 2019-11-29
### Changed
- Fix regression causing window_destroyed signal to not be triggered (after adding app and title filter) [#308](https://github.com/koekeishiya/yabai/issues/308)
- Fixed an invalid memory access when using mouse-drag to warp a window to another display when both displays contain only a single window [#309](https://github.com/koekeishiya/yabai/issues/309)
- Reset zoom of all nodes in the subtree of the node that got removed [#289](https://github.com/koekeishiya/yabai/issues/289)
- Adding/removing nodes to/from the tree should properly reset the zoom-state [#227](https://github.com/koekeishiya/yabai/issues/227)
- Ability to enable/disable debug output at runtime [#312](https://github.com/koekeishiya/yabai/issues/312)
- Fix improper calculation of overlapping parts of status_bar when truncating title, and resolve an invalid free if the title could not be truncated [#313](https://github.com/koekeishiya/yabai/issues/313)
- Automatically offset the position of the status_bar if the macOS Menubar is not set to autohide [#220](https://github.com/koekeishiya/yabai/issues/220)
- Remove the line drawn at the bottom of the status_bar in a *poorly dimmed* version of the background color.

## [2.1.2] - 2019-11-10
### Changed
- Fix regression causing windows to be added more than once in some circumstances [#297](https://github.com/koekeishiya/yabai/issues/297)

## [2.1.1] - 2019-11-10
### Changed
- Remove buffer-size restriction when reading data from socket [#221](https://github.com/koekeishiya/yabai/issues/221)
- Replace strtok with custom function to parse key-value arguments in rules and signals [#307](https://github.com/koekeishiya/yabai/issues/307)

## [2.1.0] - 2019-11-09
### Added
- Config option *window_border_radius* to specify roundness of corners [#281](https://github.com/koekeishiya/yabai/issues/281)
- Config option *window_border_placement* to specify placement of window borders (exterior, interior, inset) [#216](https://github.com/koekeishiya/yabai/issues/216)
- Config option *active_window_border_topmost* to specify if the active border should always stay on top of other windows (off, on) [#216](https://github.com/koekeishiya/yabai/issues/216)
- Ability to label spaces, making the given label an alias that can be passed to any command taking a `<SPACE_SEL>` parameter [#119](https://github.com/koekeishiya/yabai/issues/119)
- New command to adjust the split percentage of a window [#184](https://github.com/koekeishiya/yabai/issues/184)

### Changed
- Don't draw borders for minimized or hidden windows when a display is (dis)connected [#250](https://github.com/koekeishiya/yabai/issues/250)
- Sticky windows are no longer automatically topmost. New option to toggle window always on top through command or rule. New attribute topmost returned in window queries. [#255](https://github.com/koekeishiya/yabai/issues/255)
- Prevent the last user-space from being destroyed or moved to another display, because macOS does not actually support this [#182](https://github.com/koekeishiya/yabai/issues/182)
- Properly read window titles on macOS Catalina [#278](https://github.com/koekeishiya/yabai/issues/278)
- Smart swap/warp for window drag actions - the decision to swap or warp is based on where in the window the cursor is [#142](https://github.com/koekeishiya/yabai/issues/142)
- Fix subtle lock-free multithreading bug in the event processing code [#240](https://github.com/koekeishiya/yabai/issues/240)
- Changing border properties should not cause borders of minimized windows or hidden applications to be redrawn [#305](https://github.com/koekeishiya/yabai/issues/305)
- Automatically truncate title of window drawn in status_bar if it would overlap with other parts of the bar [#214](https://github.com/koekeishiya/yabai/issues/214)
- Remove throttling for mouse-drag move and lessen throttling for mouse-drag resize [#285](https://github.com/koekeishiya/yabai/issues/285)
- Properly float/unfloat windows that are tiled/untiled when marked as managed=on|off with a rule [#297](https://github.com/koekeishiya/yabai/issues/297)

## [2.0.1] - 2019-09-04
### Changed
- *window_opacity_duration* was a *copy-pasta* job [#208](https://github.com/koekeishiya/yabai/issues/208)

## [2.0.0] - 2019-09-03
### Added
- Commands to toggle mission-control, show-desktop, and application expose [#147](https://github.com/koekeishiya/yabai/issues/147)
- Command to close windows that provide a close button in its titlebar [#84](https://github.com/koekeishiya/yabai/issues/84)
- Expose window shadow as a property in window queries and add shadow as an option to *window --toggle* [#171](https://github.com/koekeishiya/yabai/issues/171)
- Config option to set the duration of the transition between active / normal window opacity [#208](https://github.com/koekeishiya/yabai/issues/208)

### Changed
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
- Verify that a message was given before trying to connect to the running yabai instance [#197](https://github.com/koekeishiya/yabai/issues/197)
- Properly re-zoom a window when toggling its border [#211](https://github.com/koekeishiya/yabai/issues/211)
- Workaround to make sure a window actually set the proper dimensions [#226](https://github.com/koekeishiya/yabai/issues/226) [#188](https://github.com/koekeishiya/yabai/issues/188)
- Moving window to a different space using rules could leave an empty tile [#232](https://github.com/koekeishiya/yabai/issues/232)
- Windows spawned while the owning application is hidden should not cause an empty tile to be created [#233](https://github.com/koekeishiya/yabai/issues/233)
- Properly handle destruction and re-creation of view when layout changes between bsp and float [#230](https://github.com/koekeishiya/yabai/issues/230)

## [1.1.2] - 2019-07-15
### Changed
- Float native macOS fullscreen spaces [#130](https://github.com/koekeishiya/yabai/issues/130)
- Write error messsages returned to the client through *stderr* [#131](https://github.com/koekeishiya/yabai/issues/131)
- Allow window focus command to work without the existence of a focused window [#133](https://github.com/koekeishiya/yabai/issues/133)

## [1.1.1] - 2019-07-14
### Changed
- The status bar should be disabled by default, if setting is missing from the config [#126](https://github.com/koekeishiya/yabai/issues/126)

## [1.1.0] - 2019-07-14
### Added
- Make loading scripting-addition more robust - validating version and functionality [#108](https://github.com/koekeishiya/yabai/issues/108)
- Merge options for query constraints with command selectors to have a unified method for addressing displays, spaces, and windows,
  as well as allowing commands to specify both a *selected* and a *given entity* of its type [#112](https://github.com/koekeishiya/yabai/issues/112)

### Changed
- Dragging a tiled window to another display using the mouse will cause the window to be warped to that display upon release [#103](https://github.com/koekeishiya/yabai/issues/103)
- Escape quotes in window titles returned through query commands [#114](https://github.com/koekeishiya/yabai/issues/114)
- Extend window, space and display properties exposed through *query* commands [#116](https://github.com/koekeishiya/yabai/issues/116)
- Native macOS fullscreen spaces can now be addressed using their mission-control index, and can also be moved [#117](https://github.com/koekeishiya/yabai/issues/117)
- Opacity changes only apply to windows that properly identify as  "standard" or "dialog" windows [#120](https://github.com/koekeishiya/yabai/issues/120)
- The status bar should now properly draw above any potential window that overlaps its frame [#124](https://github.com/koekeishiya/yabai/issues/124)
- Support *XDG_CONFIG_HOME* when locating the config file [#125](https://github.com/koekeishiya/yabai/issues/125)
- Allow every single config setting to be applied at runtime with immediate effect. Run config asynchronously after initialization [#122](https://github.com/koekeishiya/yabai/issues/122)

## [1.0.6] - 2019-07-09
### Changed
- No longer necessary to depend on the scripting-addition to focus a window [#102](https://github.com/koekeishiya/yabai/issues/102)
- Extend definition of *WINDOW_SEL* to include *smallest* and *largest* [#105](https://github.com/koekeishiya/yabai/issues/105)

## [1.0.5] - 2019-07-07
### Changed
- Fix missing quotation of string value outputted through *window query* commands [#90](https://github.com/koekeishiya/yabai/issues/90)

## [1.0.4] - 2019-07-06
### Changed
- Fixed an issue that prevented *yabai* from running under multiple users simultaneously [#95](https://github.com/koekeishiya/yabai/issues/95)
- Extend window properties exposed through *query* commands [#90](https://github.com/koekeishiya/yabai/issues/90)
- Extend definition of *WINDOW_SEL*, *SPACE_SEL* and *DISPLAY_SEL* to include *first*, *last*, and *recent* [#85](https://github.com/koekeishiya/yabai/issues/85)

## [1.0.3] - 2019-06-30
### Changed
- Prevent *status_bar* and *window borders* from displaying in native fullscreen spaces [#71](https://github.com/koekeishiya/yabai/issues/71)
- Fixed an issue with the *status_bar* where *has_battery* and *charging* would not be default initialized when macOS report that there are zero power sources [#60](https://github.com/koekeishiya/yabai/issues/60)
- Properly destroy the underlaying *view* when a *space* changes layout [#81](https://github.com/koekeishiya/yabai/issues/81)

## [1.0.2] - 2019-06-25
### Changed
- Hide power indicator from the status-bar if a battery could not be found [#60](https://github.com/koekeishiya/yabai/issues/60)
- Disable focus follows mouse while the *mouse_modifier* key is held down [#62](https://github.com/koekeishiya/yabai/issues/62)
- Silence meaningless warning reported by the scripting-bridge framework [#55](https://github.com/koekeishiya/yabai/issues/55)
- Extend definition of *WINDOW_SEL* to include *mouse*, targetting the window below the cursor [#66](https://github.com/koekeishiya/yabai/issues/66)
- Allow all *config* (except *status_bar*) settings to be edited at runtime [#69](https://github.com/koekeishiya/yabai/issues/69)
- Window should not be added to the window-tree twice when deminimized on an inactive display [#70](https://github.com/koekeishiya/yabai/issues/70)
- Expose *window_placement* as a config setting to specify if windows become the first or second child [#65](https://github.com/koekeishiya/yabai/issues/65)

## [1.0.1] - 2019-06-23
### Added
- First official release

[Unreleased]: https://github.com/koekeishiya/yabai/compare/v2.2.0...HEAD
[2.2.0]: https://github.com/koekeishiya/yabai/compare/v2.1.3...v2.2.0
[2.1.3]: https://github.com/koekeishiya/yabai/compare/v2.1.2...v2.1.3
[2.1.2]: https://github.com/koekeishiya/yabai/compare/v2.1.1...v2.1.2
[2.1.1]: https://github.com/koekeishiya/yabai/compare/v2.1.0...v2.1.1
[2.1.0]: https://github.com/koekeishiya/yabai/compare/v2.0.1...v2.1.0
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
