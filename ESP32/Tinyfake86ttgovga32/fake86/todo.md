I. Video
========
0. ~~Align pixels in #2 video mode. Now hey are messed up.~~
1. ~~Blinking cursor~~
2. ~~Auto switching palettes~~
3. ~~Correct implementation of the background color~~
4. ~~The same for the border~~
5. ~~Switchable colorburst (a possibility to turn it off for BW modes)~~
6. ~~Increase master video frequency to fit 640 pixels to the scanline~~ Made in a slightly different way :)
7. ~~Adjust colors (make, say, red more red than now). Especially light ones.~~
8. ~~Align different video modes horizontally~~
9. ~~Fix border width (top and bottom parts)~~
10. Support at least 320x200x16 Tandy video mode

II. Disks
=========
1. ~~B: as independent drive~~
2. Possibility to use disk images of different capacity, as it's possible on physical drives
3. ~~HDD (at least one)~~
4. ~~Activity LED~~

III. Hardware
=============
1. Possibility of attaching gamepads (NES-style or so)
2. DB-9 connector instead of present pair of RCAs
3. Reset button on the rear side
4. ~~Implement keyboard "typematic" feature~~
5. Reliable keyboard (get rid of missing or extra presses/releases)
6. Two-way communication with the keyboard (let STM32 serve LEDs, also this may be useful for gamepad support) ???
7. ~~Grey '+' and '-' in Fn layer~~

IV. Others
===========
1. ~~Make keyboard possible to use~~
2. RTC (?)
3. Correct implementation of system timer (and other timings significant for playing games)
4. Correct and full implementation of i8255 PIO and all the stuff connected to it.
4. ~~Nice looking on-screen menus.~~
5. ~~Make NumLock key switching layer~~
6. Functional debugger
  i. Changeable address in memory browser;
  ii. Editing values in regs browser
  iii. Editing values in memory browser
  iv. Port watcher/editor
  v. Stack browser 
