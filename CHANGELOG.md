# Changelog

All notable changes to this project will be documented in this file.

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Implemented support for stacking multiple windows in the same region (bsp node) [#203](https://github.com/koekeishiya/yabai/issues/203)
- Implemented a fullscreen layout, using stacking as its backing mechanism [#337](https://github.com/koekeishiya/yabai/issues/337)

### Changed
- Fixed an issue that caused a window to not become unmanaged when a space with a single window changed to float [#586](https://github.com/koekeishiya/yabai/issues/586)
- Restore opacity back to full if *window_opacity* is disabled [#585](https://github.com/koekeishiya/yabai/issues/585)
- Prevent *window_opacity_duration* from being used on Catalina, because of an Apple bug [#277](https://github.com/koekeishiya/yabai/issues/277)
- Update scripting-addition to support macOS Big Sur 11.0 Build 20A5354i [#589](https://github.com/koekeishiya/yabai/issues/589)
- Border windows should not have shadows [#617](https://github.com/koekeishiya/yabai/issues/617)
- *external_bar* should not have to be set before regular padding [#615](https://github.com/koekeishiya/yabai/issues/615)
- Adjust reported mouse location to use when synthesizing events for ffm autofocus [#637](https://github.com/koekeishiya/yabai/issues/637)
- Extend *SPACE_SEL* and *DISPLAY_SEL* to include the option *mouse* [#644](https://github.com/koekeishiya/yabai/issues/644)

## [3.2.1] - 2020-06-17
### Changed
- Fixed a race condition upon receiving window destroy notifications from macOS because their API is garbage and reports duplicate notifications for the same window [#580](https://github.com/koekeishiya/yabai/issues/580)
- focus-follows-mouse *autofocus* needs to perform some validation in order to see if the window is focusable [#578](https://github.com/koekeishiya/yabai/issues/578)
- Properly set mouse location for synthesized events used by ffm autofocus [#545](https://github.com/koekeishiya/yabai/issues/545)

## [3.2.0] - 2020-06-14
### Added
- Re-introduce a more efficient window border system [#565](https://github.com/koekeishiya/yabai/issues/565)
- New command *window --opacity* to explicitly set the opacity of a window [#503](https://github.com/koekeishiya/yabai/issues/503)

### Changed
- Re-construct application switched and window created events in the correct order when the window is moved through a rule upon creation [#564](https://github.com/koekeishiya/yabai/issues/564)
- Improve interaction between *window_topmost* and windows that enter native-fullscreen mode [#566](https://github.com/koekeishiya/yabai/issues/566)
- Properly set focused window id cache upon window detection at first space activation [#567](https://github.com/koekeishiya/yabai/issues/567)
- Don't modify the properties of AXUnknown and AXPopover windows [#535](https://github.com/koekeishiya/yabai/issues/535)
- The window attribute *visible* should be 0 for minimized windows [#569](https://github.com/koekeishiya/yabai/issues/569)
- Prevent *mouse_action move* from placing the y-coordinate of a window outside valid display boundaries [#570](https://github.com/koekeishiya/yabai/issues/570)
- Properly allow the user to float windows that are forced to tile using window rules (manage=on) [#571](https://github.com/koekeishiya/yabai/issues/571)
- Improve visual feedback effect of the *window --insert* message selection [#572](https://github.com/koekeishiya/yabai/issues/572)
- Fix inconsistencies when mixing floating and sticky properties on a window [#574](https://github.com/koekeishiya/yabai/issues/574)

## [3.1.2] - 2020-06-09
### Changed
- Revert changes because they can trigger a loop, causing slow window move/resize [#16](https://github.com/koekeishiya/yabai/issues/16)

## [3.1.1] - 2020-06-08
### Changed
- If *focus follows mouse* is enabled, moving the cursor to a different display will now focus that display even if it is empty [#459](https://github.com/koekeishiya/yabai/issues/459)
- Extend definition of *DISPLAY_SEL* to include *DIR_SEL* so that displays can be targetted using cardinal directions [#225](https://github.com/koekeishiya/yabai/issues/225)
- When an application is launched or a window is created; tile the window on the space that has focus, rather than the display it spawned at [#467](https://github.com/koekeishiya/yabai/issues/467)
- Properly re-adjust window frame of managed windows if they break the assigned region in response to an event not invoked directly by the user [#16](https://github.com/koekeishiya/yabai/issues/16)
- Cardinal directions for *WINDOW_SEL* will only consider managed windows due to various issues with detecting the correct window [#562](https://github.com/koekeishiya/yabai/issues/562)

## [3.1.0] - 2020-06-05
### Added
- New command to output list of registered rules and remove by index [#511](https://github.com/koekeishiya/yabai/issues/511)
- New command to output list of registered signals and remove by index [#458](https://github.com/koekeishiya/yabai/issues/458)

### Changed
- Blacklist loginwindow and ScreenSaverEngine processes [#537](https://github.com/koekeishiya/yabai/issues/537)
- Resolve some issues with the lifetime of NSRunningApplication [#543](https://github.com/koekeishiya/yabai/issues/543)
- Resolve some obscure issue that would in some cases lead to a double-free upon process termination [#543](https://github.com/koekeishiya/yabai/issues/543)
- Properly clear insert feedback settings if active on the root node when it is closed [#546](https://github.com/koekeishiya/yabai/issues/546)
- Window selector **recent** should properly work across spaces [#544](https://github.com/koekeishiya/yabai/issues/544)

## [3.0.2] - 2020-05-22
### Changed
- Properly clear focus-follows-mouse cache upon space change [#528](https://github.com/koekeishiya/yabai/issues/528)
- Revised process type restrictions and observation requirements to correctly track some applications that don't identify correctly [#529](https://github.com/koekeishiya/yabai/issues/529)
- Translate newline (0x0a) and carriage return (0x0d) when outputting window titles through the query system [#533](https://github.com/koekeishiya/yabai/issues/533)

## [3.0.1] - 2020-05-09
### Changed
- Update scripting addition for macOS 10.15.5 Beta (19F72f) [#501](https://github.com/koekeishiya/yabai/issues/501)
- Allow calling `space --label` without an argument to intentionally remove a previously assigned label [#514](https://github.com/koekeishiya/yabai/issues/514)
- Fixed an issue where a window moved across displays using *mouse action move* would be invisible [#506](https://github.com/koekeishiya/yabai/issues/506)
- Fixed an issue where the last window on a display would not properly update the focused window of the original display upon move, causing selectors to not work as expected [#505](https://github.com/koekeishiya/yabai/issues/505)

## [3.0.0] - 2020-05-01
### Removed
- The deprecated rule option *topmost='<BOOL_SEL>'* has been removed.
- The built-in status bar has been removed. [#486](https://github.com/koekeishiya/yabai/issues/486)
- Window borders have been removed. [#487](https://github.com/koekeishiya/yabai/issues/487)

### Added
- New window commands `--minimize` and `--deminimize`. Minimized windows are now reported through window queries and there is a new attribute `minimized` to identify the current state [#379](https://github.com/koekeishiya/yabai/issues/379)
- New config option `external_bar` to specify special padding compatible with the `space --toggle padding` option [#454](https://github.com/koekeishiya/yabai/issues/454)

### Changed
- New self-signed certificate used to sign the released binaries because the previous one expired at april 21th, 2020. You will have to re-enable accessibility permissions after this install. Sorry about that.
- Window commands using cardinal directions use euclidean distance to identify best target window [#301](https://github.com/koekeishiya/yabai/issues/301)
- Config option `insert_window_border_color` changed to `insert_feedback_color` [#487](https://github.com/koekeishiya/yabai/issues/487)
- Fixed an issue that would invalidate a manual insertion point, if a mouse action triggered [#492](https://github.com/koekeishiya/yabai/issues/492)

## [2.4.3] - 2020-04-14
### Changed
- Changed how *mouse down* events are handled to reduce cycles spent in macOS event tap callback [#376](https://github.com/koekeishiya/yabai/issues/376)
- Fixed an issue that would cause a border to persist on fullscreen videos playing in Safari [#360](https://github.com/koekeishiya/yabai/issues/360)

## [2.4.2] - 2020-04-12
### Changed
- Fix memory leak that would occur if realloc failed when reading from a socket [#436](https://github.com/koekeishiya/yabai/issues/436)
- Increase number of window and space commands that return a non-zero exit code upon failure [#187](https://github.com/koekeishiya/yabai/issues/187)
- Fixed a repositioning issue that would occur when a managed window was moved through click & drag through macOS native titlebar [#473](https://github.com/koekeishiya/yabai/issues/473)

## [2.4.1] - 2020-03-01
### Changed
- Fixed a crash that could occur when reading from a socket (EBADF) or writing to a socket (SIGPIPE) [#430](https://github.com/koekeishiya/yabai/issues/430)

## [2.4.0] - 2020-03-01
### Added
- Support exclusion for command arguments of type REGEX [#173](https://github.com/koekeishiya/yabai/issues/173)
- Add ability to specify window layers; below (desktop), normal, and above (topmost) [#429](https://github.com/koekeishiya/yabai/issues/429)

### Changed
- Properly return focus to the active window on the current space when a window is moved through a rule [#418](https://github.com/koekeishiya/yabai/issues/418)
- Prevent space operations (create, destroy, focus, swap, move and send to display) from applying while mission-control is active or the display is animating [#417](https://github.com/koekeishiya/yabai/issues/417)

## [2.3.0] - 2020-02-14
### Added
- New command *space --swap SPACE_SEL* command to swap the selected space with a given space. The selected and given space must belong to the same display [#127](https://github.com/koekeishiya/yabai/issues/127)

### Changed
- Allow use of *DISPLAY_SEL* and *SPACE_SEL* for specifying display and space in rules [#378](https://github.com/koekeishiya/yabai/issues/378)
- Extend *space --move* command to operate on *SPACE_SEL* instead of prev/next. However, the selected and given space must belong to the same display [#127](https://github.com/koekeishiya/yabai/issues/127)
- Fixed issue where some window event would trigger a signal with an invalid window causing an invalid memory access [#412](https://github.com/koekeishiya/yabai/issues/412)

## [2.2.3] - 2020-02-12
### Changed
- Ignore minimized windows when an application is unhidden (required by some applications like Chrome..) [#300](https://github.com/koekeishiya/yabai/issues/300)
- Don't add window to the window tree when moved to a different space when the window is minimized (required by some applications like Chrome..) [#382](https://github.com/koekeishiya/yabai/issues/382)
- Config file is no longer required for yabai to start [#393](https://github.com/koekeishiya/yabai/issues/393)
- Clear umask before trying to install scripting addition [#400](https://github.com/koekeishiya/yabai/issues/400)
- Update scripting addition to work with macos Catalina 10.15.4 Beta [#404](https://github.com/koekeishiya/yabai/issues/404)
- Don't forward MOUSE_UP event to the target application if we consumed the corresponding MOUSE_DOWN event [#376](https://github.com/koekeishiya/yabai/issues/376)
- Only float small windows if they are NOT resizable [#179](https://github.com/koekeishiya/yabai/issues/179)
- Fixed an issue that caused filtering signals by window title to not work properly [#410](https://github.com/koekeishiya/yabai/issues/410)

## [2.2.2] - 2020-01-20
### Changed
- Fix use after free.. [#375](https://github.com/koekeishiya/yabai/issues/375)

## [2.2.1] - 2020-01-19
### Changed
- Specify minimum macOS target version 10.13 when compiling [#371](https://github.com/koekeishiya/yabai/issues/371)

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

[Unreleased]: https://github.com/koekeishiya/yabai/compare/v3.2.1...HEAD
[3.2.1]: https://github.com/koekeishiya/yabai/compare/v3.2.0...v3.2.1
[3.2.0]: https://github.com/koekeishiya/yabai/compare/v3.1.2...v3.2.0
[3.1.2]: https://github.com/koekeishiya/yabai/compare/v3.1.1...v3.1.2
[3.1.1]: https://github.com/koekeishiya/yabai/compare/v3.1.0...v3.1.1
[3.1.0]: https://github.com/koekeishiya/yabai/compare/v3.0.2...v3.1.0
[3.0.2]: https://github.com/koekeishiya/yabai/compare/v3.0.1...v3.0.2
[3.0.1]: https://github.com/koekeishiya/yabai/compare/v3.0.0...v3.0.1
[3.0.0]: https://github.com/koekeishiya/yabai/compare/v2.4.3...v3.0.0
[2.4.3]: https://github.com/koekeishiya/yabai/compare/v2.4.2...v2.4.3
[2.4.2]: https://github.com/koekeishiya/yabai/compare/v2.4.1...v2.4.2
[2.4.1]: https://github.com/koekeishiya/yabai/compare/v2.4.0...v2.4.1
[2.4.0]: https://github.com/koekeishiya/yabai/compare/v2.3.0...v2.4.0
[2.3.0]: https://github.com/koekeishiya/yabai/compare/v2.2.3...v2.3.0
[2.2.3]: https://github.com/koekeishiya/yabai/compare/v2.2.2...v2.2.3
[2.2.2]: https://github.com/koekeishiya/yabai/compare/v2.2.1...v2.2.2
[2.2.1]: https://github.com/koekeishiya/yabai/compare/v2.2.0...v2.2.1
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
