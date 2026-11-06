//-----------------------------------------------------------------------------
// MegaSoC Ethernet build: restore the default net type
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Listed first in flist/megasoc_tech_ETH.flist (INC_ETH builds only).
//
// The SDIO controller sources (logical/sdspi/rtl) set `default_nettype none
// and leave it set. A compiler directive stays in force for every file
// compiled after it, and the OpenCores ethmac sources rely on implicit nets
// (undeclared identifiers in port connections), so without this file they
// fail with "Identifier not declared". This restores the Verilog default
// before the Ethernet files are read.
//-----------------------------------------------------------------------------

`default_nettype wire
