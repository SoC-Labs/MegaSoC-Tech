//==============================================================================
// sdio_m_exp_m_axi_read_tests.c -- MEGASOC-ILL-008, SDIO_M manager x READ leg
//
// The makefile sets EXP_M_PROBE_ONLY=1 and EXP_M_SKIP_RECOVERY, so this is the
// write-probe of sdio_m_exp_m_reach_tests alone: sdio_write() with the DMA
// source at EXP_M (0x6000_0000) -> AXI READ at the absent EXP_M target, which
// must end in DECERR (CMD.DMA_ERR + CMD.ERR set), no hang. One clean run per
// direction; the post-abort recovery step is skipped (BUG-42).
//==============================================================================
#include "../sdio_m_exp_m_reach_tests/sdio_m_exp_m_reach_tests.c"
