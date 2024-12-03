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

build_sie300_sram_ctrl:
	@$(SIE300_IP_LOGICAL_DIR)/generate --config ./socrates/BP301_SRAM/config/SRAM_ctrl.yaml --output ./logical/sie300/
build_nic400:
	socrates_cli --project megasoc_tech -data ../ --flow build.configured.component configuredComponentName=nic400_megasoc_main
build_cortex_a53:
	mkdir $(SOCLABS_MEGASOC_TECH_DIR)/logical/CortexA53_1/
	mkdir $(SOCLABS_MEGASOC_TECH_DIR)/logical/CortexA53_1/verilog
	@$(CORTEX_A53_IP_LOGICAL_DIR)/shared/tools/bin/RenderCORTEXA53.pl -config $(SOCLABS_MEGASOC_TECH_DIR)/socrates/CortexA53_1/CORTEXA53.cfg -input $(CORTEX_A53_IP_LOGICAL_DIR)/cortexa53/verilog/CORTEXA53_unconfigured.v -output $(SOCLABS_MEGASOC_TECH_DIR)/logical/CortexA53_1/verilog/CORTEXA53.v

build_ip: build_nic400 build_cortex_a53 build_sie300_sram_ctrl

make_project:
	socrates_cli --project megasoc_tech -data ../ --flow AddNewProject

first_time_setup: make_project build_ip

all: make_project build_ip

clean:
	@rm -rf ./logical/Cortex-M55
	@rm -rf ./logical/nic400_millisoc_system
	@rm -rf ./logical/shared
