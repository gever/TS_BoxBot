## Tinkering School Boxbot

This project is part of the **Tinkering School Mars Mission**, and is
designed to help bring high-quality robotics experiences to classrooms everywhere.
https://tinkeringschool.com/mars

For more information, collaboration, or kudos, contact: mars@tinkeringschool.com
If you would like to join the project and help us grow the curriculum, iterate the
robotics platforms, improve and extend the software, then consider joining our discord.

Donations to help us bring this to more people can be made at: https://tinkeringschool.com/donate

This work licensed under a Creative Commons Attribution 4.0 license. That means you can use
it any way you like, including commercially, provided that you attribute it to us, The Institute for Applied Tinkering,
and include a link to https://tinkeringschool.com

https://creativecommons.org/licenses/by/4.0/

```
debugging this issue which shows up when we build on a Mac:
Archiving .pio/build/esp32dev/libFrameworkArduino.a
Indexing .pio/build/esp32dev/libFrameworkArduino.a
Linking .pio/build/esp32dev/firmware.elf
/Users/shoyt23/.platformio/packages/toolchain-xtensa-esp32/bin/../lib/gcc/xtensa-esp32-elf/8.4.0/../../../../xtensa-esp32-elf/bin/ld: .pio/build/esp32dev/firmware.elf section `.dram0.bss' will not fit in region `dram0_0_seg'
/Users/shoyt23/.platformio/packages/toolchain-xtensa-esp32/bin/../lib/gcc/xtensa-esp32-elf/8.4.0/../../../../xtensa-esp32-elf/bin/ld: DRAM segment data does not fit.
/Users/shoyt23/.platformio/packages/toolchain-xtensa-esp32/bin/../lib/gcc/xtensa-esp32-elf/8.4.0/../../../../xtensa-esp32-elf/bin/ld: DRAM segment data does not fit.
/Users/shoyt23/.platformio/packages/toolchain-xtensa-esp32/bin/../lib/gcc/xtensa-esp32-elf/8.4.0/../../../../xtensa-esp32-elf/bin/ld: region `dram0_0_seg' overflowed by 5184 bytes
collect2: error: ld returned 1 exit status
*** [.pio/build/esp32dev/firmware.elf] Error 1
```