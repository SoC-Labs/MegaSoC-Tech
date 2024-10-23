#-----------------------------------------------------------------------------
# megaSoC System  Top-Level Makefile 
# - Includes other Makefiles in flow directory
# A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
#
# Contributors
#
# David Flynn (d.w.flynn@soton.ac.uk)
# Daniel Newbrook (d.newbrook@soton.ac.uk)
# Copyright (C) 2021-4, SoC Labs (www.soclabs.org)
#-----------------------------------------------------------------------------

include ./make.cfg

build_pck600:
	socrates_cli --project megasoc_tech -data ../ --flow build.configured.component configuredComponentName=pck600_clk_ctrl_1
	socrates_cli --project megasoc_tech -data ../ --flow build.configured.component configuredComponentName=pck600_ppu_1
build_sie300_sram_ctrl:
	@$(SIE300_IP_LOGICAL_DIR)/generate --config ./socrates/BP301_SRAM/config/SRAM_ctrl.yaml --output ./logical/SMC
build_nic400:
	socrates_cli --project megasoc_tech -data ../ --flow build.configured.component configuredComponentName=nic400_megasoc_main
build_ip: 

make_project:
	socrates_cli --project megasoc_tech -data ../ --flow AddNewProject

first_time_setup: make_project build_ip

all: make_project build_ip

clean:
	@rm -rf ./logical/Cortex-M55
	@rm -rf ./logical/nic400_millisoc_system
	@rm -rf ./logical/shared
