#!/usr/bin/env osascript

use framework "AppKit"

--------------------------------------------------------------------------------
# PROPERTY DECLARATIONS:
property this : a reference to current application
property NSWorkspace : a reference to NSWorkspace of this
property NSImage : a reference to NSImage of this

--------------------------------------------------------------------------------
# IMPLEMENTATION:
on run argv
  set icon to item 1 of argv
  set target to item 2 of argv

  setIcon to icon for target
end run
--------------------------------------------------------------------------------
# HANDLERS:
to setIcon to iconPath for filePath
  set sharedWorkspace to NSWorkspace's sharedWorkspace()
  set newImage to NSImage's alloc()
  set icon to newImage's initWithContentsOfFile:iconPath

  set success to sharedWorkspace's setIcon:icon forFile:filePath options:0
end setIcon
