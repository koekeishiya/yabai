### Description

**yabai** started as a C99 re-rewrite of [*chunkwm*](https://github.com/koekeishiya/chunkwm), originally supposed to be its first RC version.

However due to major architectural changes, supported systems, and changes to functionality, it is being released separately.
There are multiple reasons behind these changes, based on the experience I've gained through experimenting with, designing, and using both *kwm*
and *chunkwm*. Some of these changes are performance related while other changes have been made to keep the user experience simple and more complete,
attemps to achieve a seamless integration with the operating system (when possible), proper error reporting, and yet still keep the property of being
customizable.

### Requirements

**yabai** is officially supported on **macOS Mojave 10.14.4**. [*System Integrity Protection*](https://support.apple.com/en-us/HT204899) must be disabled and [*chunkwm-sa*](https://github.com/koekeishiya/chwm-sa)
must be installed for **yabai** to function properly. The *scripting-addition* is a bundle of code that we inject into *Dock.app* to elevate our privileges when communicating with the *WindowServer*.
The *WindowServer* is a single point of contact for all applications. It is central to the implementation of the GUI frameworks and many other services.

**yabai** must be given permission to utilize the *Accessibility API*, and will request access upon launch. The application must be restarted after access has been granted.
It is recommended to first [*codesign*](https://github.com/koekeishiya/yabai/blob/master/CODESIGN.md) the binary such that access can persist through builds/updates.
You can read more about codesigning [here](https://developer.apple.com/library/archive/documentation/Security/Conceptual/CodeSigningGuide/Procedures/Procedures.html#//apple_ref/doc/uid/TP40005929-CH4-SW2).

The *Mission Control* setting [*displays have separate spaces*](https://support.apple.com/library/content/dam/edam/applecare/images/en_US/osx/separate_spaces.png) must be enabled.

**yabai** stores a lock file at `/tmp/yabai_$USER.lock` to keep multiple instances from launcing by the same user.

**yabai** stores a unix domain socket at `/tmp/yabai_$USER.socket` to listen for commands.

**DISCLAIMER:** Use at your own discretion. I take no responsibility if anything should happen to your machine while trying to install, test or otherwise use this software in any form.
You aknowledge that you understand the potential risk that may come from disabling [*System Integrity Protection*](https://support.apple.com/en-us/HT204899) on your system, and I make
no recommendation as to whether you should or should not disable SIP.

### Install

**Homebrew**:

```
TODO
```

**Source**:

Requires xcode-8 command-line tools.

```
git clone https://github.com/koekeishiya/yabai
make install      # release version
make              # debug version
```

**Install scripting-addition**:

```
sudo yabai --install-sa
```

### Uninstall

**yabai** is a single binary application and is trivial to uninstall.

**Uninstall scripting-addition**:

```
sudo yabai --uninstall-sa
```

**Remove config and tmp files**:
```
rm ~/.yabairc
rm /tmp/yabai_$USER.lock
rm /tmp/yabai_$USER.socket
```

**Homebrew**:

```
TODO
```

**Source**:

```
Remove the cloned git-repository
```

### Configuration

The default configuration file is a shell-script located at `~/.yabairc`, thus the executable permission-bit must be set.
A different location can be specified with the *--config | -c* argument.

Keyboard shortcuts can be defined with [*skhd*](https://github.com/koekeishiya/skhd) or any other suitable software you may prefer.

Sample configuration files can be found in the [examples](https://github.com/koekeishiya/yabai/tree/master/examples) directory.

Refer to the [*documentation*](https://github.com/koekeishiya/yabai/blob/master/doc/yabai.asciidoc) for further information.
