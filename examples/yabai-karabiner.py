#-karabiner # yabai-karabiner.py
#
# This script is not written by the author of `skhd` or `yabai`!
# ..And *NOT* so well tested.
#
# ## Requirements
# Python3  
#
# ## About Karabiner-Elements
# It works like `skhd`. `skhd` makes long delay in my computor,
# so I'm prefering Karabiner-Elements: https://github.com/tekezo/Karabiner-Elements  
#
# ## Description
# Prints JSON string for Karabiner-Elements.  
# If "gen" is given as a first argument, `yabai-karabiner.json` is created where this script is.  

# ***********************************
# *********** JSON builder **********
# ***********************************

import json
import sys
from pathlib import Path # https://docs.python.org/3/library/pathlib.html

args = sys.argv

class YabaiBuilder:
    def __init__(self):
        self.cfg = {
            "title": "yabai",
            "rules": []
        }

        # constants
        self.yabai = "/usr/local/bin/yabai"
        self.description_prefix = "yabai/"

        # manupulations of the rule being configured
        self.mans = None

    def new(self, description):
        """Creates new rule"""
        mans = []
        bind_ = {
            "description": f"{self.description_prefix}{description}",
            "manipulators": mans
        }
        self.cfg["rules"].append(bind_)
        self.mans = mans
        return self

    def bind(self, action, key, modifiers):
        """Binds action to the current rule"""
        action = action.replace("yabai", self.yabai)
        bind_ = {
            "type": "basic",
            "from": {
                "key_code": key,
            },
            "to": [
                {
                    "shell_command": f"{self.yabai} {action}"
                }
            ]
        }
        if modifiers is not None:
            bind_["from"]["modifiers"] = {
                "mandatory": modifiers
            }

        self.mans.append(bind_)
        return self

    def to_json(self):
        """Prints the current configuration as JSON"""
        return json.dumps(self.cfg, indent=4) # , sort_keys=True)

# ************************************
# *********** configuration **********
# ************************************

# This is where to setup your configuration

cfg = YabaiBuilder()

### following `example/skhdrc`

# Tips:
# - use "option" instead of "alt" ("alt" doesn't work)
# - use "spacebar" to represent spacebar keycode

# window (focus, swap, move, balance)
cfg.new("window/focus (opt - [hjkl])") \
    .bind("-m window --focus west",  "h", ["option"]) \
    .bind("-m window --focus south", "j", ["option"]) \
    .bind("-m window --focus north", "k", ["option"]) \
    .bind("-m window --focus east",  "l", ["option"])

cfg.new("window/swap (opt + shift - [hjkl])") \
    .bind("-m window --swap west",  "h", ["option", "shift"]) \
    .bind("-m window --swap south", "j", ["option", "shift"]) \
    .bind("-m window --swap north", "k", ["option", "shift"]) \
    .bind("-m window --swap east",  "l", ["option", "shift"])

cfg.new("window/move (cmd + shift - [hjkl])") \
    .bind("-m window --warp west",  "h", ["command", "shift"]) \
    .bind("-m window --warp south", "j", ["command", "shift"]) \
    .bind("-m window --warp north", "k", ["command", "shift"]) \
    .bind("-m window --warp east",  "l", ["command", "shift"])

cfg.new("window/balance (opt + cmd - 0)") \
    .bind("-m space --balance",  "0", ["option", "command"])

# floating window (place)
cfg.new("float/place (opt + shift - [arrow])") \
    .bind("-m window --grid 1:1:0:0:1:1",  "up_arrow",    ["option", "shift"]) \
    .bind("-m window --grid 1:2:0:0:1:1",  "left_arrow",  ["option", "shift"]) \
    .bind("-m window --grid 1:2:1:0:1:1",  "right_arrow", ["option", "shift"]) \

# desktop (create, destroy, focus, send)
cfg.new("desktop/create (shift/opt + cmd - n)") \
    .bind(
         '-m space --create && \
    index="$(yabai -m query --spaces --display | jq \'map(select(."native-fullscreen" == 0))[-1].index\')" && \
    yabai -m window --space "${index}" && \
    yabai -m space --focus "${index}"',
        "n",
        ["shift", "command"]) \
    .bind(
         '-m space --create && \
    index="$(yabai -m query --spaces --display | jq \'map(select(."native-fullscreen" == 0))[-1].index\')" && \
    yabai -m space --focus "${index}"',
        "n",
        ["option", "command"])

cfg.new("desktop/delete (opt + cmd + w)") \
    .bind("-m space --destroy", "w", ["option", "command"])

cfg.new("desktop/focus (opt + cmd - [x,z,c,1..9,0])") \
    .bind("-m space --focus recent", "x", ["option", "command"]) \
    .bind("-m space --focus prev",   "z", ["option", "command"]) \
    .bind("-m space --focus next",   "c", ["option", "command"]) \
    .bind("-m space --focus 1",      "1", ["option", "command"]) \
    .bind("-m space --focus 2",      "2", ["option", "command"]) \
    .bind("-m space --focus 3",      "3", ["option", "command"]) \
    .bind("-m space --focus 4",      "4", ["option", "command"]) \
    .bind("-m space --focus 5",      "5", ["option", "command"]) \
    .bind("-m space --focus 6",      "6", ["option", "command"]) \
    .bind("-m space --focus 7",      "7", ["option", "command"]) \
    .bind("-m space --focus 8",      "8", ["option", "command"]) \
    .bind("-m space --focus 9",      "9", ["option", "command"]) \
    .bind("-m space --focus 10",     "0", ["option", "command"])

cfg.new("desktop/send_window (ctrl + opt - [x,z,c,1..9,0])") \
    .bind("-m space --focus recent; yabai -m space --focus recent", "x", ["option", "command"]) \
    .bind("-m space --focus prev;   yabai -m space --focus prev",   "z", ["option", "command"]) \
    .bind("-m space --focus next;   yabai -m space --focus next",   "c", ["option", "command"]) \
    .bind("-m space --focus 1;      yabai -m space --focus 1",      "1", ["option", "command"]) \
    .bind("-m space --focus 2;      yabai -m space --focus 1",      "2", ["option", "command"]) \
    .bind("-m space --focus 3;      yabai -m space --focus 1",      "3", ["option", "command"]) \
    .bind("-m space --focus 4;      yabai -m space --focus 1",      "4", ["option", "command"]) \
    .bind("-m space --focus 5;      yabai -m space --focus 1",      "5", ["option", "command"]) \
    .bind("-m space --focus 6;      yabai -m space --focus 1",      "6", ["option", "command"]) \
    .bind("-m space --focus 7;      yabai -m space --focus 1",      "7", ["option", "command"]) \
    .bind("-m space --focus 8;      yabai -m space --focus 1",      "8", ["option", "command"]) \
    .bind("-m space --focus 9;      yabai -m space --focus 1",      "9", ["option", "command"]) \
    .bind("-m space --focus 10;     yabai -m space --focus 10",     "0", ["option", "command"])

# monitor (focus, send)
cfg.new("monitor/focus (ctrl + opt - [x,z,c,1..3])") \
    .bind("-m display --focus recent", "x", ["control", "option"]) \
    .bind("-m display --focus prev",   "z", ["control", "option"]) \
    .bind("-m display --focus next",   "c", ["control", "option"]) \
    .bind("-m display --focus 1",      "1", ["control", "option"]) \
    .bind("-m display --focus 2",      "2", ["control", "option"]) \
    .bind("-m display --focus 3",      "3", ["control", "option"]) \

cfg.new("monitor/focus (ctrl + cmd - [x,z,c,1..3])") \
    .bind("-m display --focus recent; yabai -m display --focus recent", "x", ["control", "command"]) \
    .bind("-m display --focus prev;   yabai -m display --focus prev",   "z", ["control", "command"]) \
    .bind("-m display --focus next;   yabai -m display --focus next",   "c", ["control", "command"]) \
    .bind("-m display --focus 1;      yabai -m display --focus 1",      "1", ["control", "command"]) \
    .bind("-m display --focus 2;      yabai -m display --focus 1",      "2", ["control", "command"]) \
    .bind("-m display --focus 3;      yabai -m display --focus 1",      "3", ["control", "command"]) \

# window (move, size)
cfg.new("window/move (shift + ctrl - [wasd])") \
    .bind("-m window --move rel:-20:0", "a", ["shift", "control"]) \
    .bind("-m window --move rel:0:20",  "s", ["shift", "control"]) \
    .bind("-m window --move rel:0:-20", "w", ["shift", "control"]) \
    .bind("-m window --move rel:20:0",  "d", ["shift", "control"])

cfg.new("window/size/increase (shift + opt - [wasd])") \
    .bind("-m window --resize left:-20:0",  "a", ["shift", "option"]) \
    .bind("-m window --resize bottom:0:20", "s", ["shift", "option"]) \
    .bind("-m window --resize top:0:-20",   "w", ["shift", "option"]) \
    .bind("-m window --resize right:20:0",  "d", ["shift", "option"])

cfg.new("window/size/decrese (shift + cmd - [wasd])") \
    .bind("-m window --resize left:20:0",  "a", ["shift", "command"]) \
    .bind("-m window --resize bottom:0:-20", "s", ["shift", "command"]) \
    .bind("-m window --resize top:0:-20",   "w", ["shift", "command"]) \
    .bind("-m window --resize right:-20:0",  "d", ["shift", "command"])

# window (insert)
cfg.new("window/insert (ctrl + opt - [hjkl])") \
    .bind("-m window --insert west",  "a", ["control", "option"]) \
    .bind("-m window --insert south", "s", ["control", "option"]) \
    .bind("-m window --insert north", "w", ["control", "option"]) \
    .bind("-m window --insert east",  "d", ["control", "option"])

# window (tree)
cfg.new("window/tree (opt - [rxy])") \
    .bind("-m space --rotate 90",     "r", ["option"]) \
    .bind("-m space --mirror y-axis", "y", ["option"]) \
    .bind("-m space --mirror x-axis", "x", ["option"])

# window/toggle
cfg.new("window/toggle/zoom (opt (+ shift) - [df])") \
    .bind("-m window --toggle zoom-parent",       "d", ["option"]) \
    .bind("-m window --toggle zoom-fullscreen",   "f", ["option"]) \
    .bind("-m window --toggle native-fullscreen", "f", ["option", "shift"])

cfg.new("window/toggle/view (opt - a, opt + shift - b)") \
    .bind("-m space --toggle padding; yabai -m space --toggle gap", "a", ["option"]) \
    .bind("-m window --toggle border", "b", ["option", "shift"])

cfg.new("window/toggle/and_place (opt - [tsp])") \
    .bind("-m window --toggle float; yabai -m window --grid 4:4:1:1:2:2", "t", ["option"]) \
    .bind("-m window --toggle topmost", "s", ["option"]) \
    .bind(
   """-m window --toggle sticky;
yabai -m window --toggle topmost;
yabai -m window --grid 5:5:4:0:1:1""",
        "p",
        ["option"])

cfg.new("window/toggle/layout (opt + ctrl - [ad])") \
    .bind("-m space --layout bsp",   "a", ["option", "control"]) \
    .bind("-m space --layout float", "d", ["option", "control"])

# **************************
# *********** run **********
# **************************

# print the configuration as JSON
# or create `yabai-karabiner.json` where this script is
if len(sys.argv) > 1 and args[1] == "gen":
    path = Path(args[0]).with_name('yabai-karabiner.json')
    path.write_text(cfg.to_json())
    print(f"generated at: {path.resolve()}")
else:
    print(cfg.to_json())


