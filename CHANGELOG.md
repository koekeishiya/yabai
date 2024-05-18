# Changelog

All notable changes to this project will be documented in this file.

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [7.1.1] - 2024-05-18
### Changed
- Assigning a window to scratchpad using rules would incorrectly hide that window immediately [#2203](https://github.com/koekeishiya/yabai/issues/2203)
- Moving windows to other spaces should once again work on macOS Sonoma 14.5 (and newer) [#2240](https://github.com/koekeishiya/yabai/issues/2240)
- Update scripting addition for macOS Sonoma 14.5 Intel (Apple Silicon is already supported in v7.1.0) [#2277](https://github.com/koekeishiya/yabai/issues/2277)

## [7.1.0] - 2024-04-04
### Added
- Added window commands `--raise ['<WINDOW_SEL>']` and `--lower ['<WINDOW_SEL>']` [#2198](https://github.com/koekeishiya/yabai/issues/2198)
- Added window commands `--scratchpad ['<LABEL>|recover']` and `--toggle <LABEL>` [#2203](https://github.com/koekeishiya/yabai/issues/2203)
- Added rule property `scratchpad='<LABEL>'` to automatically assign a window to a scratchpad [#2203](https://github.com/koekeishiya/yabai/issues/2203)
- Added window and rule query property `scratchpad` [#2203](https://github.com/koekeishiya/yabai/issues/2203)

### Changed
- Config *global setting* `auto_balance` is now categorized as a *space setting* instead [#2200](https://github.com/koekeishiya/yabai/issues/2200)
- Rule property space should have higher precedence than display [#2206](https://github.com/koekeishiya/yabai/issues/2206)
- Properly escape app and title, role and subrole, regex when listing rules [#2205](https://github.com/koekeishiya/yabai/issues/2205)
- Properly escape app and title regex when listing signals [#2207](https://github.com/koekeishiya/yabai/issues/2207)
- Fixed issue that could cause a crash when trying to detect windows opened before yabai launch [#2208](https://github.com/koekeishiya/yabai/issues/2208)
- Fixed issue that could cause a crash when terminating an application in rare cases where there is a large backlog of events [#2210](https://github.com/koekeishiya/yabai/issues/2210)

## [7.0.4] - 2024-03-30
### Changed
- Consecutive window resize operations would not work correctly because it used a cached value for the window frame [#2182](https://github.com/koekeishiya/yabai/issues/2182)
- Fix weird delay caused by interaction between macOS System APIs on Sonoma when using `--insert` in combination with other commands in rapid succession [#2188](https://github.com/koekeishiya/yabai/issues/2188)
- Change how background processes are detected and handled [#2190](https://github.com/koekeishiya/yabai/issues/2190) [#2168](https://github.com/koekeishiya/yabai/issues/2168) [#2194](https://github.com/koekeishiya/yabai/issues/2194)

## [7.0.3] - 2024-03-22
### Changed
- Whitelist zathura process for management because it incorrectly identifies as a background-only process [#2168](https://github.com/koekeishiya/yabai/issues/2168)
- Config should be executed if the exec-bit is set, but interpreted if it is not [#2169](https://github.com/koekeishiya/yabai/issues/2169)
- Window query property is-sticky would sometimes show the wrong value [#2175](https://github.com/koekeishiya/yabai/issues/2175)
- Query commands now take an optional list of properties to select which fields should be included in the output [#2180](https://github.com/koekeishiya/yabai/issues/2180)

## [7.0.2] - 2024-03-13
### Changed
- Decouple/normalize timer used for mouse_action resize throttling because event timestep varies between macOS versions [#2160](https://github.com/koekeishiya/yabai/issues/2160)

## [7.0.1] - 2024-03-13
### Changed
- Fix mouse_action resize throttling issue caused by friggin Apple event timestep garbage [#2160](https://github.com/koekeishiya/yabai/issues/2160)

## [7.0.0] - 2024-03-13
### Added
- Make space visible on display without stealing focus (the space must belong to the display) [#2113](https://github.com/koekeishiya/yabai/issues/2113)
- Restore application_activated and application_deactivated signals [#2122](https://github.com/koekeishiya/yabai/issues/2122)
- Restore system_woke signal [#2124](https://github.com/koekeishiya/yabai/issues/2124)
- Added new window rules command `--apply` to apply the effects of (specific or all existing rules), or an ad-hoc rule that should only apply once, to all known windows [#2123](https://github.com/koekeishiya/yabai/issues/2123)
- Added new argument `--one-shot` to window rules command `--add` to specify that this rule only runs once [#2123](https://github.com/koekeishiya/yabai/issues/2123)
- Window rules marked as `--one-shot` will be ignored completely by the `--apply` command [#2123](https://github.com/koekeishiya/yabai/issues/2123)
- Window queries include a new property `has-ax-reference` [#2126](https://github.com/koekeishiya/yabai/issues/2126)
- Window command `--sub-layer` and rule property `sub-layer` has been added to replace `--layer` and `layer` respectively [#2128](https://github.com/koekeishiya/yabai/issues/2128)
- Added new command `config window_animation_easing ..` to select easing function [#2131](https://github.com/koekeishiya/yabai/issues/2131)
- Added new command `space --equalize .. ` to reset split ratios of all nodes within a space to default value [#2133](https://github.com/koekeishiya/yabai/issues/2133)
- Added new command `space --switch ..` to focus a space (substitute with current focus) regardless of its display [#549](https://github.com/koekeishiya/yabai/issues/549)
- Added new command `config display_arrangement_order ..` to change how yabai interprets arrangement indices used to select displays [#550](https://github.com/koekeishiya/yabai/issues/550)
- Added new command `display --label ..` and display query property `label` (works like space labels) [#1616](https://github.com/koekeishiya/yabai/issues/1616)
- Added display query property `has-focus` [#1616](https://github.com/koekeishiya/yabai/issues/1616)

### Changed
- Preserve relative space ordering when moving spaces to other displays [#2114](https://github.com/koekeishiya/yabai/issues/2114)
- Make window animations compatible with window opacity fade effect [#2116](https://github.com/koekeishiya/yabai/issues/2116)
- Better filter for background processes [#2118](https://github.com/koekeishiya/yabai/issues/2118)
- Managed windows should snap back into place when moved incorrectly using the mouse [#1199](https://github.com/koekeishiya/yabai/issues/1199) [#2066](https://github.com/koekeishiya/yabai/issues/2066)
- Managed windows should correct their frame when modified by external means [#2117](https://github.com/koekeishiya/yabai/issues/2117)
- Window frame would sometimes not be set correctly when using window animations [#2120](https://github.com/koekeishiya/yabai/issues/2120)
- Allow resetting window opacity in window rules [#2127](https://github.com/koekeishiya/yabai/issues/2127)
- When adding new window rules, their effects will only apply to *windows that open after the rule has been added* [#2123](https://github.com/koekeishiya/yabai/issues/2123)
- Window queries will now list information about all windows even if yabai does not yet have an AX-reference. Windows that are missing an AX-reference cannot be acted upon until its space has become active [#2126](https://github.com/koekeishiya/yabai/issues/2126)
- Fixed window animation flickering [#2129](https://github.com/koekeishiya/yabai/issues/2129)
- Fixed weird issue with memory ownership when synthesizing events for autofocus [#2130](https://github.com/koekeishiya/yabai/issues/2130)
- Combine effects of all matching window rules before applying rule effects [#2123](https://github.com/koekeishiya/yabai/issues/2123)
- Window insert feedback visual should use same layer as parent window [#2132](https://github.com/koekeishiya/yabai/issues/2132)
- Updated scripting-addition mach loader/injection and payload to work for macOS Sonoma 14.4 [#2146](https://github.com/koekeishiya/yabai/issues/2146) [#2150](https://github.com/koekeishiya/yabai/issues/2150)
- Some space and display signals (where possible) now pass additional env.vars. containing the user-facing index that can be used with yabai commands [#2152](https://github.com/koekeishiya/yabai/issues/2152)
- Fixed some window commands that would incorrectly require a focused window to exist in order to operate on an arbitrary window through its id [#2153](https://github.com/koekeishiya/yabai/issues/2153)
- Window grid command will now apply the configured space window gap [#932](https://github.com/koekeishiya/yabai/issues/932)
- Config file no longer requires the exec-permission-bit to be set [#1993](https://github.com/koekeishiya/yabai/issues/1993)
- Allow `space --swap ..` command to swap spaces between displays. This works by swapping all windows (and interior state) rather than macOS spaces [#549](https://github.com/koekeishiya/yabai/issues/549)
- Focus-follows-mouse should now be a lot more responsive [#1958](https://github.com/koekeishiya/yabai/issues/1958)
- Fixed issue causing Dock to not hide if the cursor is moved before animation finishes [#1552](https://github.com/koekeishiya/yabai/issues/1552)
- Fixed issue causing Dock to not respect autohide-delay preference [#1951](https://github.com/koekeishiya/yabai/issues/1951)
- Fixed issue where using alt as mouse_modifier would trigger macOS functionality that hides all windows [#2154](https://github.com/koekeishiya/yabai/issues/2154) [#1809](https://github.com/koekeishiya/yabai/issues/1809)
- Fixed absurd issue where subscribing to mouse_events would cause delayed input in some cases when magic mouse is connected [#1877](https://github.com/koekeishiya/yabai/issues/1877)

### Removed
- When adding new window rules, their effects will only apply to *windows that open after the rule has been added*. To restore old behavior, run `yabai -m rule --apply` after adding all rules [#2123](https://github.com/koekeishiya/yabai/issues/2123)
- Window command `--layer` and rule property `layer` has been renamed to `--sub-layer` and `sub-layer` respectively [#2128](https://github.com/koekeishiya/yabai/issues/2128)

## [6.0.15] - 2024-02-22
### Changed
- Fix weird segfault with consumed mouse-event [#1801](https://github.com/koekeishiya/yabai/issues/1801)
- Implement visual feedback for mouse_drag actions, controlled by `insert_feedback_color` [#2109](https://github.com/koekeishiya/yabai/issues/2109)

## [6.0.14] - 2024-02-21
### Changed
- Mouse actions could stop working if the acting window was destroyed while a mouse action was in progress [#2038](https://github.com/koekeishiya/yabai/issues/2038)
- Replicate consumed mouse-click when no mouse-drag action is initiated [#1801](https://github.com/koekeishiya/yabai/issues/1801)

## [6.0.13] - 2024-02-19
### Changed
- Lock assigned handle while mouse_action resize is in progress [#2102](https://github.com/koekeishiya/yabai/issues/2102)
- Modify "no click-zone" when using focus_follows_mouse to activate an empty display [#1892](https://github.com/koekeishiya/yabai/issues/1892) [#863](https://github.com/koekeishiya/yabai/issues/863)

## [6.0.12] - 2024-02-12
### Added
- New config option `menubar_opacity` to make the menubar transparent (or even completely hidden, ignoring mouse-events) [#2091](https://github.com/koekeishiya/yabai/issues/2091)

### Changed
- Using `--toggle native-fullscreen` to exit native-fullscreen mode could sometimes leave the window unmanaged/untiled [#2090](https://github.com/koekeishiya/yabai/issues/2090)

## [6.0.11] - 2024-02-11
### Changed
- Adjustments to mission control integration on Ventura and Sonoma due to subtle issues with window operations that cross monitor boundaries [#2088](https://github.com/koekeishiya/yabai/issues/2088)

## [6.0.10] - 2024-02-10
### Changed
- Fixed caching issue when using focus_follows_mouse to switch focus between monitors with no windows [#2075](https://github.com/koekeishiya/yabai/issues/2075)
- Attempt to resolve issue where some applications do not incorrectly pass along NSRunningApplication isObservable and isFinishedLaunching properties [#1367](https://github.com/koekeishiya/yabai/issues/1367)
- Add compatibility between window animations and JankyBorders [#2087](https://github.com/koekeishiya/yabai/issues/2087)

### Removed
- Config option `window_animation_frame_rate` has been removed. Animations use CVDisplayLink to match monitor refresh rate [#2087](https://github.com/koekeishiya/yabai/issues/2087)

## [6.0.9] - 2024-02-04
### Changed
- Fix detection of windows that are minimized before yabai is launched [#1833](https://github.com/koekeishiya/yabai/issues/1833)

## [6.0.8] - 2024-02-04
### Changed
- All windows that report a non-standard window_level should be treated as floating (permanently), unless otherwise specified through manage=on rules [#2055](https://github.com/koekeishiya/yabai/issues/2055)
- Fix detection of windows that are minimized before yabai is launched [#1833](https://github.com/koekeishiya/yabai/issues/1833)

## [6.0.7] - 2024-01-25
### Changed
- Update scripting addition for macOS Sonoma 14.3 Intel (Apple Silicon is already supported in v6.0.6) [#2065](https://github.com/koekeishiya/yabai/issues/2065)
- Fix regression causing window_destroyed signal to not trigger [#2048](https://github.com/koekeishiya/yabai/issues/2048)
- Improved snappiness of window animations (time from trigger to start of animation) [#2060](https://github.com/koekeishiya/yabai/issues/2060)
- Change window sublayer instead of layer, resulting in a more pleasant user experience [#2062](https://github.com/koekeishiya/yabai/issues/2062)

## [6.0.6] - 2024-01-08
### Changed
- Cleanup handling of root-windows, child-windows, and window belonging to roles that are considered eligible for management [#2044](https://github.com/koekeishiya/yabai/issues/2044) [#2036](https://github.com/koekeishiya/yabai/issues/2036)

## [6.0.5] - 2024-01-07
### Changed
- Attempt to separate root-windows from child/sub-windows to improve window detection and management logic [#2044](https://github.com/koekeishiya/yabai/issues/2044)

## [6.0.4] - 2024-01-03
### Changed
- Changes to window detection logic [#2036](https://github.com/koekeishiya/yabai/issues/2036)

## [6.0.3] - 2024-01-03
### Changed
- Changing window layer using rules or `window --layer` commands will exempt that window from automatic layer changes [#1929](https://github.com/koekeishiya/yabai/issues/1929)
- Running `yabai --stop-service` should properly prevent yabai from starting after a reboot [#1921](https://github.com/koekeishiya/yabai/issues/1921)
- `mission_control_enter/exit` signals include an environment variable to identify which mode was activated/deactivated [#2026](https://github.com/koekeishiya/yabai/issues/2026)
- Stricter window type filter to avoid issues with e.g Text Completion, Input Source changes and other non-windows-that-report-as-windows [#1919](https://github.com/koekeishiya/yabai/issues/1919) [#1910](https://github.com/koekeishiya/yabai/issues/1910) [#1997](https://github.com/koekeishiya/yabai/issues/1997)
- Expand process blacklist filter to ignore irrelevant processes (background services and helper services)

## [6.0.2] - 2023-12-23
### Changed
- Update scripting addition for macOS Sonoma 14.2 and 14.2.1 [#2007](https://github.com/koekeishiya/yabai/issues/2007)
- Spaces in stack layout should no longer incorrectly trigger a layout refresh on mouse down [#1493](https://github.com/koekeishiya/yabai/issues/1493)
- Fix frame rounding issues causing small window displacement [#1680](https://github.com/koekeishiya/yabai/issues/1680)

## [6.0.1] - 2023-11-12
### Changed
- Update scripting addition for macOS 14.1.1 (and probably 14.1) [#1936](https://github.com/koekeishiya/yabai/issues/1936)
- Fix issue with focusing stacked windows due to layer changes [#1918](https://github.com/koekeishiya/yabai/issues/1918)
- Allow window swap command to work on windows that are in the same space (with layout: stack), using the stack window selectors [#1952](https://github.com/koekeishiya/yabai/issues/1952)
- Fix rare crash when processing window destroyed events [#1965](https://github.com/koekeishiya/yabai/issues/1965)

## [6.0.0] - 2023-10-10
### Added
- Window query property `layer` has been added [#1887](https://github.com/koekeishiya/yabai/issues/1887)

### Changed
- Fix issue causing window animations to flicker on macOS Ventura and Sonoma [#1879](https://github.com/koekeishiya/yabai/issues/1879)
- All managed (read: tiled) windows are now automatically placed in the *below* layer. All unmanaged (read: floating) windows will use the default macOS *normal* layer and appear above the tiled layer, replicating the `window_topmost` functionality in a robust way [#1887](https://github.com/koekeishiya/yabai/issues/1887)
- Automatic window opacity changes will now only apply to focus switches within the same space [#1887](https://github.com/koekeishiya/yabai/issues/1887)
- `space --create` command now takes an optional `<DISPLAY_SEL>` instead of an optional `<SPACE_SEL>`

### Removed
- Config option `window_topmost` has been removed [#1887](https://github.com/koekeishiya/yabai/issues/1887)
- Window command `--toggle` option `topmost` has been removed [#1887](https://github.com/koekeishiya/yabai/issues/1887)
- Window query property `is-topmost` has been removed [#1887](https://github.com/koekeishiya/yabai/issues/1887)
- Signal `application_activated` and `application_deactivated` has been removed; use `application_front_switched` instead [#1887](https://github.com/koekeishiya/yabai/issues/1887)
- Window borders (and all related options/properties) have been removed [#1889](https://github.com/koekeishiya/yabai/issues/1889)

## [5.0.9] - 2023-10-01
### Changed
- Updated scripting-addition to support macOS Sonoma 14.0 [#1772](https://github.com/koekeishiya/yabai/issues/1772)
- Fix window focusing (autofocus, and autoraise across multiple monitors [#109](https://github.com/koekeishiya/yabai/issues/109)) for macOS Sonoma [#1772](https://github.com/koekeishiya/yabai/issues/1772)
- Fix scripting-addition functionality that uses Dock.app connection to the WindowServer for macOS Sonoma [#1772](https://github.com/koekeishiya/yabai/issues/1772)
- Fix mission-control integration for macOS Sonoma [#1772](https://github.com/koekeishiya/yabai/issues/1772)
- Prevent *focus follows mouse* from activating the menubar in vertical display arrangements [#1857](https://github.com/koekeishiya/yabai/issues/1857)

## [5.0.8] - 2023-09-12
### Changed
- Workaround for macOS Ventura reporting bad window levels when running yabai as a service, causing issues with window topmost [#1704](https://github.com/koekeishiya/yabai/issues/1704)

## [5.0.7] - 2023-08-27
### Added
- Add launch argument `--help, -h` to print available options. [#1825](https://github.com/koekeishiya/yabai/issues/1825)

### Changed
- Allow window swap commands to work on windows that are in the same stack, using the stack window selectors [#960](https://github.com/koekeishiya/yabai/issues/960)
- Properly remove assigned label when a space is destroyed [#1678](https://github.com/koekeishiya/yabai/issues/1678)
- Add signals for `space_created` and `space_destroyed` [#1365](https://github.com/koekeishiya/yabai/issues/1365)

## [5.0.6] - 2023-05-27
### Changed
- Change launchd service to only restart automatically upon crashes. You will need to run `yabai --uninstall-service` and `yabai --install-service`. [#1755](https://github.com/koekeishiya/yabai/issues/1755)

## [5.0.5] - 2023-05-21
### Changed
- Create `LaunchAgents` folder in `~/Library` when installing service file, if the directory does not already exist [#1728](https://github.com/koekeishiya/yabai/issues/1728)
- Changed how the users home directory is determined when managing service file [#1742](https://github.com/koekeishiya/yabai/issues/1742)
- Calculate memory requirement of service file path and contents instead of using static storage [#1749](https://github.com/koekeishiya/yabai/issues/1749)
- Window selector `stack.first` and `stack.last` should return an error when there is no stack [#1748](https://github.com/koekeishiya/yabai/issues/1748)
- Fixed off-by-one when checking frame positions in `xxx_in_direction` [#1511](https://github.com/koekeishiya/yabai/issues/1511) [#1463](https://github.com/koekeishiya/yabai/issues/1463)

## [5.0.4] - 2023-05-01
### Added
- Added launch arguments to manage launchd service: `--install-service`, `--uninstall-service`, `--start-service`, `--restart-service`, `--stop-service` [#1619](https://github.com/koekeishiya/yabai/issues/1619)
- Check for `-arm64e_preview_abi` bootflag on Apple Silicon before attempting to load scripting addition.

## [5.0.3] - 2023-03-28
### Changed
- Updated scripting-addition to support macOS Ventura 13.3 [#1297](https://github.com/koekeishiya/yabai/issues/1297)
- Fixed issue with window focusing caused by incorrectly intercepting a synthesized mouse event [#1551](https://github.com/koekeishiya/yabai/issues/1551)
- Fixed issue with window warping across displays with only a single window tiled at both displays [#1577](https://github.com/koekeishiya/yabai/issues/1577)
- Fixed issue preventing window split from being toggled for windwos on an inactive space/display [#1557](https://github.com/koekeishiya/yabai/issues/1557)
- Make window zoom persistence configurable (*config window_zoom_persist*) [#1481](https://github.com/koekeishiya/yabai/issues/1481)
- Make frame rate of window animations configurable (*config window_animation_frame_rate*) [#148](https://github.com/koekeishiya/yabai/issues/148)

## [5.0.2] - 2022-12-16
### Changed
- Updated scripting-addition to support macOS Ventura 13.0.0-13.1.0 [#1297](https://github.com/koekeishiya/yabai/issues/1297)
- Properly escape application name when returned in window queries [#1489](https://github.com/koekeishiya/yabai/issues/1489)
- Remove window tags used for debugging purposes from result of window query because it could cause a crash under certain conditions when a window closes [#1475](https://github.com/koekeishiya/yabai/issues/1475)
- Change window placement of warp command to be more natural when warping windows within the same space [#1435](https://github.com/koekeishiya/yabai/issues/1435)

## [5.0.1] - 2022-09-26
### Changed
- Only allow *window_animation_duration* to be set if System Integrity Protection is partially disabled [#148](https://github.com/koekeishiya/yabai/issues/148)
- Output useless dummy .plist file for scripting addition to silence weird AppleScript warning [#1449](https://github.com/koekeishiya/yabai/issues/1449)

## [5.0.0] - 2022-09-23
### Added
- Support for animating window move/resize operations (*config window_animation_duration*) [#148](https://github.com/koekeishiya/yabai/issues/148)
- Command to manually specify the default *split_type* [#1423](https://github.com/koekeishiya/yabai/issues/1423)
- Window borders are now placed below and outside the window; new commands to specify hidpi, blur to act as a backdrop, and corner radius [#1430](https://github.com/koekeishiya/yabai/issues/1430)
- Add *split-child* to output of window query and introduce new options for WINDOW_SEL: *sibling*, *first_nephew*, *second_nephew*, *uncle*, *first_cousin*, *second_cousin* [#192](https://github.com/koekeishiya/yabai/issues/192)

### Changed
- Implemented a workaround to support *window_opacity_duration*, bypassing the Apple bug [#1406](https://github.com/koekeishiya/yabai/issues/1406)
- Fix regression causing a window to be moved to the active space of an inactive display when sent to an inactive space of an inactive display [#1053](https://github.com/koekeishiya/yabai/issues/1053)
- Fix regression causing hidden windows to not show in window queries [#1421](https://github.com/koekeishiya/yabai/issues/1421)
- Clamp split ratio instead of resetting when an out of range / invalid value is given [#1401](https://github.com/koekeishiya/yabai/pull/1401)
- Applying rule with property *manage=on* would cause both minimized and hidden windows to be managed, even though the window is not visible [#1418](https://github.com/koekeishiya/yabai/issues/1418)
- Fix regression causing window sticky to not work properly [#1424](https://github.com/koekeishiya/yabai/issues/1424)
- Make window zoom more flexible, allow parent-zoomed window to enter fullscreen and vice versa [#1429](https://github.com/koekeishiya/yabai/issues/1429)
- Fix border size issue when moving a window to a different display on macOS Big Sur [#1229](https://github.com/koekeishiya/yabai/issues/1229)
- Check Dock.app isFinishedLaunching property before attempting to inject scripting addition [#749](https://github.com/koekeishiya/yabai/issues/749)
- Properly update window ordering when a window is added to the top of a stack [#1311](https://github.com/koekeishiya/yabai/issues/1311)
- Properly update insertion point in a window stack when the marked window is removed from the stack [#1275](https://github.com/koekeishiya/yabai/issues/1275)
- Window zoom will now persist through changes to the bsp layout [#864](https://github.com/koekeishiya/yabai/issues/864)

### Removed
- Removed support for macOS High Sierra, Mojave, and Catalina.
- Removed launch arguments *--install-sa* and *--check-sa*. Running *--load-sa* will automatically install/update the scripting-addition when necessary [#1287](https://github.com/koekeishiya/yabai/issues/1287)

## [4.0.4] - 2022-09-08
### Changed
- Fix null-deref when an application is spawned with a non-standard window [#1399](https://github.com/koekeishiya/yabai/issues/1399)

## [4.0.3] - 2022-09-07
### Changed
- Allow combining commands for *config*, *space*, and *window* domains [#1371](https://github.com/koekeishiya/yabai/issues/1371)
- Fallback to using the AX API for *mouse_action move* if the scripting-addition is not available [#1376](https://github.com/koekeishiya/yabai/issues/1376)
- Add *role* and *subrole* filter to window rules [#1398](https://github.com/koekeishiya/yabai/issues/1398)

## [4.0.2] - 2022-08-24
### Changed
- Fixed an issue that in rare occasions caused yabai to freeze when focusing an inactve space on an inactive display [#1309](https://github.com/koekeishiya/yabai/issues/1309).
- Fixed an issue that caused a window to incorrectly become focused when assigned to a space through rules [#1370](https://github.com/koekeishiya/yabai/issues/1370)
- Fixed an issue that caused windows to snap back to its original position when moved between displays (to an inactive space) using mission-control [#820](https://github.com/koekeishiya/yabai/issues/820)

## [4.0.1] - 2022-05-17
### Changed
- The scripting-addition will now also remove the space switch animation when using cmd+tab, clicking on an item in the Dock, and using the numeric macOS mission-control keyboard shortcuts [#1235](https://github.com/koekeishiya/yabai/issues/1235)
- Improved logic used to determine the target window in a given direction [#1220](https://github.com/koekeishiya/yabai/issues/1220)
- Improve behaviour of *focus_follows_mouse autoraise*, preventing a window from being raised if it would occlude some other **floating window** [#1246](https://github.com/koekeishiya/yabai/issues/1246)
- The rule option *mouse_follows_focus* should now work properly (values were inverted) [#1267](https://github.com/koekeishiya/yabai/issues/1267)
- Remove minor shadow artifact from border windows [#1056](https://github.com/koekeishiya/yabai/issues/1056)

## [4.0.0] - 2022-03-16
### Added
- New config *window_origin_display* to specify which display a window should become managed at upon creation [#951](https://github.com/koekeishiya/yabai/issues/951)

### Changed
- Window borders no longer require SIP to be disabled [#1054](https://github.com/koekeishiya/yabai/issues/1054)
- WINDOW_SEL *prev* should correctly identify the correct window in a nested tree [#1114](https://github.com/koekeishiya/yabai/issues/1114)
- Fixed an issue with the way unix sockets were handled that would cause an incoming connection to drop in rare occasions [#1107](https://github.com/koekeishiya/yabai/issues/1107)
- Update scripting addition to support macOS 12.0.0 -> 12.3 [#1054](https://github.com/koekeishiya/yabai/issues/1054)
- Fixed an issue that would cause the target window to snap back to its previous position when moved between displays using the cursor inside mission-control [#820](https://github.com/koekeishiya/yabai/issues/820)
- Properly drain autoreleased objects (from Apple frameworks) [#751](https://github.com/koekeishiya/yabai/issues/751)
- Detect and manage windows that are moved into the first space of a display when a space with active windows on it is destroyed [#813](https://github.com/koekeishiya/yabai/issues/813)
- The command *space --balance* now takes an optional axis as its argument [#985](https://github.com/koekeishiya/yabai/issues/985)
- Malformed windows that are managed through rules should be eligible for window alpha, focus follows mouse, and window borders [#788](https://github.com/koekeishiya/yabai/issues/788)
- Improve behaviour of *focus_follows_mouse autoraise*, preventing a window from being raised if it would occlude some other window. autoraise is temporarily disabled while a menu is opened. [#407](https://github.com/koekeishiya/yabai/issues/407)
- Work around broken Apple code for retrieving menubar dimensions on Apple Silicon M1 [#793](https://github.com/koekeishiya/yabai/issues/793)
- Rework query attributes and define dataformat as part of the API [#775](https://github.com/koekeishiya/yabai/issues/775)
- Properly clear space.last-window and space.first-window query attributes when the last window is made floating [#786](https://github.com/koekeishiya/yabai/issues/786)
- Reworked signal system; events are no longer coupled 1-1 with observed system events.
Some events are now eligible for a new filter, *active*, only triggering for the application/window with key-focus.
The *window_focused* signal is now triggered when the key-window changes, regardless of whether its application is frontmost or not.
The *window_created* signal is now triggered for windows that are implicitly created at application launch.
The *window_destroyed* signal is now triggered for windows that are implicitly destroyed at application exit; it is also eligible for *app* filter [#581](https://github.com/koekeishiya/yabai/issues/581)

### Removed
- The following signals have been removed: *mouse_up*, *mouse_down*, *mouse_dragged*, *mouse_moved*, *mission_control_check_for_exit*, *menu_opened*, *system_woke*, *daemon_message*

## [3.3.10] - 2021-05-26
### Changed
- Update scripting addition to support macOS 10.15.7 Supplemental Update

## [3.3.9] - 2021-05-25
### Changed
- Update scripting addition to support macOS 11.4

## [3.3.8] - 2021-04-28
### Changed
- Update scripting addition to support macOS 11.3

## [3.3.7] - 2021-02-02
### Changed
- Update scripting addition to support macOS 11.2 [#823](https://github.com/koekeishiya/yabai/issues/823)

## [3.3.6] - 2020-12-18
### Changed
- Update scripting addition to support macOS 11.1 [#762](https://github.com/koekeishiya/yabai/issues/762)
- Try to workaround resizing issues in some applications due to weird undocumented accessibility properties [#109](https://github.com/koekeishiya/yabai/issues/109)
- A sticky window will implicitly be treated as floating, but should not actually set the floating property [#760](https://github.com/koekeishiya/yabai/issues/760)

## [3.3.5] - 2020-12-03
### Changed
- Improved SIP detection logic [#716](https://github.com/koekeishiya/yabai/issues/716)
- Windows that do not report a title at all should be treated as having the empty string as its title [#707](https://github.com/koekeishiya/yabai/issues/707)
- Allow *SPACE_SEL* to be used instead of *mission-control index* when specifying config options for a specific space [#705](https://github.com/koekeishiya/yabai/issues/705)
- Native fullscreen transitions would freeze on macOS Mojave due to internal API differences between macOS version [#690](https://github.com/koekeishiya/yabai/issues/690)
- Report proper error message when trying to use absolute resizing on a managed window [#661](https://github.com/koekeishiya/yabai/issues/661)
- Space/Window commands that utilize the scripting addition should correctly return a non-zero exit code upon failure [#181](https://github.com/koekeishiya/yabai/issues/181)
- Undo [#545](https://github.com/koekeishiya/yabai/issues/545) because it created weird issues with focus [#660](https://github.com/koekeishiya/yabai/issues/660)
- Allow enabling/disabling *mouse_follows_focus* for specific windows using rules, overriding the global setting [#675](https://github.com/koekeishiya/yabai/issues/675)
- Space labels are now allowed to start with a digit, as long as it is followed by at least one or more non-digit character(s) [#739](https://github.com/koekeishiya/yabai/issues/739)

## [3.3.4] - 2020-11-14
### Changed
- Fixed an issue (exposed on Big Sur, maybe didn't exist on prior macOS versions ??) that caused yabai' message receiver to block in *read* [#714](https://github.com/koekeishiya/yabai/issues/714)

## [3.3.3] - 2020-11-13
### Changed
- Prevent *window_opacity_duration* from being used on Big Sur, because of an Apple bug [#277](https://github.com/koekeishiya/yabai/issues/277)

## [3.3.2] - 2020-11-13
### Changed
- Focusing a space incorrectly returned a non-zero exit code even when the operation succeeded [#181](https://github.com/koekeishiya/yabai/issues/181)

## [3.3.1] - 2020-11-13
### Changed
- New self-signed certificate used to sign the released binaries. You will have to re-enable accessibility permissions after this install.
- Update scripting-addition to support macOS Big Sur 11.0.1 [#589](https://github.com/koekeishiya/yabai/issues/589)
- Return a non-zero exit code when focusing a space fails due to an issue with the scripting-addition [#181](https://github.com/koekeishiya/yabai/issues/181)

## [3.3.0] - 2020-09-03
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

[Unreleased]: https://github.com/koekeishiya/yabai/compare/v7.1.1...HEAD
[7.1.1]: https://github.com/koekeishiya/yabai/compare/v7.1.0...v7.1.1
[7.1.0]: https://github.com/koekeishiya/yabai/compare/v7.0.4...v7.1.0
[7.0.4]: https://github.com/koekeishiya/yabai/compare/v7.0.3...v7.0.4
[7.0.3]: https://github.com/koekeishiya/yabai/compare/v7.0.2...v7.0.3
[7.0.2]: https://github.com/koekeishiya/yabai/compare/v7.0.1...v7.0.2
[7.0.1]: https://github.com/koekeishiya/yabai/compare/v7.0.0...v7.0.1
[7.0.0]: https://github.com/koekeishiya/yabai/compare/v6.0.15...v7.0.0
[6.0.15]: https://github.com/koekeishiya/yabai/compare/v6.0.14...v6.0.15
[6.0.14]: https://github.com/koekeishiya/yabai/compare/v6.0.13...v6.0.14
[6.0.13]: https://github.com/koekeishiya/yabai/compare/v6.0.12...v6.0.13
[6.0.12]: https://github.com/koekeishiya/yabai/compare/v6.0.11...v6.0.12
[6.0.11]: https://github.com/koekeishiya/yabai/compare/v6.0.10...v6.0.11
[6.0.10]: https://github.com/koekeishiya/yabai/compare/v6.0.9...v6.0.10
[6.0.9]: https://github.com/koekeishiya/yabai/compare/v6.0.8...v6.0.9
[6.0.8]: https://github.com/koekeishiya/yabai/compare/v6.0.7...v6.0.8
[6.0.7]: https://github.com/koekeishiya/yabai/compare/v6.0.6...v6.0.7
[6.0.6]: https://github.com/koekeishiya/yabai/compare/v6.0.5...v6.0.6
[6.0.5]: https://github.com/koekeishiya/yabai/compare/v6.0.4...v6.0.5
[6.0.4]: https://github.com/koekeishiya/yabai/compare/v6.0.3...v6.0.4
[6.0.3]: https://github.com/koekeishiya/yabai/compare/v6.0.2...v6.0.3
[6.0.2]: https://github.com/koekeishiya/yabai/compare/v6.0.1...v6.0.2
[6.0.1]: https://github.com/koekeishiya/yabai/compare/v6.0.0...v6.0.1
[6.0.0]: https://github.com/koekeishiya/yabai/compare/v5.0.9...v6.0.0
[5.0.9]: https://github.com/koekeishiya/yabai/compare/v5.0.8...v5.0.9
[5.0.8]: https://github.com/koekeishiya/yabai/compare/v5.0.7...v5.0.8
[5.0.7]: https://github.com/koekeishiya/yabai/compare/v5.0.6...v5.0.7
[5.0.6]: https://github.com/koekeishiya/yabai/compare/v5.0.5...v5.0.6
[5.0.5]: https://github.com/koekeishiya/yabai/compare/v5.0.4...v5.0.5
[5.0.4]: https://github.com/koekeishiya/yabai/compare/v5.0.3...v5.0.4
[5.0.3]: https://github.com/koekeishiya/yabai/compare/v5.0.2...v5.0.3
[5.0.2]: https://github.com/koekeishiya/yabai/compare/v5.0.1...v5.0.2
[5.0.1]: https://github.com/koekeishiya/yabai/compare/v5.0.0...v5.0.1
[5.0.0]: https://github.com/koekeishiya/yabai/compare/v4.0.4...v5.0.0
[4.0.4]: https://github.com/koekeishiya/yabai/compare/v4.0.3...v4.0.4
[4.0.3]: https://github.com/koekeishiya/yabai/compare/v4.0.2...v4.0.3
[4.0.2]: https://github.com/koekeishiya/yabai/compare/v4.0.1...v4.0.2
[4.0.1]: https://github.com/koekeishiya/yabai/compare/v4.0.0...v4.0.1
[4.0.0]: https://github.com/koekeishiya/yabai/compare/v3.3.10...v4.0.0
[3.3.10]: https://github.com/koekeishiya/yabai/compare/v3.3.9...v3.3.10
[3.3.9]: https://github.com/koekeishiya/yabai/compare/v3.3.8...v3.3.9
[3.3.8]: https://github.com/koekeishiya/yabai/compare/v3.3.7...v3.3.8
[3.3.7]: https://github.com/koekeishiya/yabai/compare/v3.3.6...v3.3.7
[3.3.6]: https://github.com/koekeishiya/yabai/compare/v3.3.5...v3.3.6
[3.3.5]: https://github.com/koekeishiya/yabai/compare/v3.3.4...v3.3.5
[3.3.4]: https://github.com/koekeishiya/yabai/compare/v3.3.3...v3.3.4
[3.3.3]: https://github.com/koekeishiya/yabai/compare/v3.3.2...v3.3.3
[3.3.2]: https://github.com/koekeishiya/yabai/compare/v3.3.1...v3.3.2
[3.3.1]: https://github.com/koekeishiya/yabai/compare/v3.3.0...v3.3.1
[3.3.0]: https://github.com/koekeishiya/yabai/compare/v3.2.1...v3.3.0
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
