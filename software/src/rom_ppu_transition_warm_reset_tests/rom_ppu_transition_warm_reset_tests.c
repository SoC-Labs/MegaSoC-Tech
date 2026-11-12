/* BUG-40: SWRESET while ROM_PPU is moving ON -> OFF (dyn wake off), no wait. Same source as BOOT-017 (a). */
#define BOOT017_CASE_B 1
#define BOOT017_NO_WAIT 1
#include "../rom_ppu_off_warm_reboot_tests/rom_ppu_off_warm_reboot_tests.c"
