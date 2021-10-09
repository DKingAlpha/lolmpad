# LOLM Pad
Dedicated Gamepad Remapping Tool for League of Legends **Mobile**

The source code is a quick hack developed on `2160*1080` screen with `Xbox One Controller`.

Do expect trash code everywhere.

It may not work on your phone out of box. You need to update source code accrodingly if specs do not meet.

# Features
1. LeftStick to Move, LeftStickClick + A/X/Y/B to upgrade QWER 
2. RightStick to Select Target, RightStickClick to Confirm Target
3. A => Attack, B=> Recall, LS/RS/LT/RT => QWER, X/Y => DF
4. Back => Scoreboard, Start => Ward
5. Support target selection while pressing LS/RS/LT/RT and Ward
6. DPAD up/down: Buy Item 1/2.  DPAD left/right: Attack Minion/Turret

# Benefits
1. Android-less linux rootkit. Fast I/O. Instant response.
2. Safe. Undetectable<sup>*</sup> to game process.

# Implementaion
1. Read input from gamepad
2. Keep gamepad status, generate legal multitouch events accordingly.
3. Write multitouch events to touchscreen device directly. Let android suck it.


# Prerequisites 
1. rooted Android
2. basic knowledge of `adb shell`, command line and C/C++.

# Quick Start
0. Update source code.
1. Download Android NDK
2. `cd <lolmpad>`
3. `<Android NDK Path>/ndk-build.cmd`
4. `adb push libs\arm64-v8a\lolmpad /data/local/tmp/ && chmod +x /data/local/tmp/lolmpad`
5. In **Android ADB shell** or **termux**, run /data/local/tmp/lolmpad as `su`
6. Start game, rotate left.

# What to update
1. `lolmpad.cpp: main()`. Update input device paths. One for screen, one for controller.
    * `getevent -li`. screen device has ABS_MT_ keywords.
2. `padhelper.h: namespace ButtonPos`. Paste original fullscreen game screenshot in `mspaint.exe` to get button positions.
    * Game must be left rotated, otherwise more source code need to get fixed.
3. `[optional]` if some keys on your gamepad do not work, try checking names starts with `BTN_` and `KEY_`.
    * run `getevent -l /dev/input/eventX` in adb shell to get actual key input.
