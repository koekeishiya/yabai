#!/usr/bin/env python

import Cocoa
import sys

image = Cocoa.NSImage.alloc().initWithContentsOfFile_(sys.argv[1]);
binary = sys.argv[2];
options = 0;

result = Cocoa.NSWorkspace.sharedWorkspace().setIcon_forFile_options_(image, binary, options);
if result == 0: print("could not set icon for file..");
