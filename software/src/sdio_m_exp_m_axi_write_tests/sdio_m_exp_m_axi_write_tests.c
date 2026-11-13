//==============================================================================
// sdio_m_exp_m_axi_write_tests.c -- MEGASOC-ILL-008, SDIO_M manager x WRITE bin
//
// The makefile sets EXP_M_PROBE_ONLY=2 and EXP_M_SKIP_RECOVERY, so this is the
// read-probe of sdio_m_exp_m_reach_tests alone: sdio_read() with the DMA
// destination at EXP_M (0x6000_0000) -> AXI WRITE at the absent EXP_M target,
// which must end in DECERR (CMD.DMA_ERR + CMD.ERR set), no hang. See the knob
// notes in the included file for why recovery is skipped (BUG-42).
//==============================================================================
#include "../sdio_m_exp_m_reach_tests/sdio_m_exp_m_reach_tests.c"
