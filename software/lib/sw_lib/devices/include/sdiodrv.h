////////////////////////////////////////////////////////////////////////////////
//
// Filename:	sw/sdiodrv.h
// {{{
// Project:	SD-Card controller
//
// Purpose:	
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2016-2025, Gisselquist Technology, LLC
// {{{
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
// }}}
// License:	GPL, v3, as defined and found on www.gnu.org,
// {{{
//		http://www.gnu.org/licenses/gpl.html
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
#ifndef	SDIODRV_H
#define	SDIODRV_H
#include <stdint.h>

#define OPT_SDIODMA

// DMA address is exposed as two fixed 32-bit registers (LO then HI, per
// OPT_LITTLE_ENDIAN=1 in sdaxil.v) rather than a native `void *`. The
// SDIO_M_AXI master port is 44 bits wide (megasoc_tech_wrapper.sv), wider
// than a 32-bit pointer can hold, and a host `void *` also varies in size
// (4 bytes on ILP32, 8 on LP64/AArch64), so its layout can't be pinned to
// these fixed hardware offsets by struct member sizing alone.
typedef	struct SDIO_S {
	volatile uint32_t	sd_cmd, sd_data, sd_fifa, sd_fifb, sd_phy;
	volatile uint32_t	sd_dma_addr_lo, sd_dma_addr_hi;
	volatile uint32_t	sd_dma_length;
} SDIO;

typedef	struct	SDIODRV_S {
	SDIO		*d_dev;
	uint32_t	d_CID[4], d_OCR;
	char		d_SCR[8], d_CSD[16];
	uint16_t	d_RCA;
	uint32_t	d_sector_count, d_block_size;
} SDIODRV;

//struct	SDIODRV_S;

extern	struct	SDIODRV_S *sdio_init(SDIO *dev);
extern	int	sdio_write(struct SDIODRV_S *dev, const unsigned sector, const unsigned count, const char *buf);
extern	int	sdio_read(struct SDIODRV_S *dev, const unsigned sector, const unsigned count, char *buf);
extern	int	sdio_ioctl(struct SDIODRV_S *dev, char cmd, char *buf);
#endif
