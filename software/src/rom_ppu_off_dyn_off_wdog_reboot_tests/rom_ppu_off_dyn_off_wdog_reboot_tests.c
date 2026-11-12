/* BOOT-017 case (b) with a watchdog reset instead of SWRESET (BUG-40, 2026-11-11). Same source as case (a). */
#define BOOT017_CASE_B 1
#define BOOT017_WDOG 1
#include "../rom_ppu_off_warm_reboot_tests/rom_ppu_off_warm_reboot_tests.c"
