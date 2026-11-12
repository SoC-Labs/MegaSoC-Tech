# Socrates configurations

This folder holds the Arm Socrates project files from which the generated Arm IP
under `logical/` is produced (`nic400_megasoc_main`, `nic400_megasoc_system`,
`nic400_tlx_megasoc`, the Cortex-A53, the DMA-350 and the BP301 SRAM controllers).
The generated RTL is not tracked in this repository (see `.gitignore`); it is
regenerated locally with Socrates from these files, so **these files are the
configuration of record**.

## NIC-400 security (TrustZone) policy

Decision of 10 November 2026 (Jira MBT-68): **megaSoC does not use TrustZone
partitioning.** The interconnect is configured so that every master is Secure,
and no address region is marked Secure-only.

* **Every slave interface is configured `secure`.** In `nic400_megasoc_main`
  these are A53, SDIO_M, ADP, EXP_S, ETH_M and DMA_M; in `nic400_megasoc_system`
  it is DMA. For a NIC-400 slave interface the `secure` option means that the
  master's AxPROT[1] is *not* forwarded: the generated RTL ties the forwarded bit
  to 0 (for example `assign arprot_A53_m[1] = 1'b0;` in
  `nic400_asib_A53_megasoc_main.v`), so every transaction inside the network is
  Secure whatever the master drove. This is deliberate and is what "every master
  is Secure" means.
* **No master interface is `secure`.** FLASH and GIC (`nic400_megasoc_main`) and
  SYS_SRAM (`nic400_megasoc_system`) were changed from `secure` to `non_secure`
  on 10 November 2026. A `secure` master interface rejects Non-secure
  transactions with DECERR; since no Non-secure transaction can exist inside this
  network, those three settings never filtered anything and only suggested a
  partitioning that does not exist. The APB master interfaces were already
  `non_secure`.
* **What this means for software.** The Cortex-A53's security state
  (SCR_EL3.NS) has no effect on which bus regions it can reach. The GIC-400
  receives every access as Secure, so software must program the GIC through its
  Secure register view and keep its interrupts in Group 0 (nIRQ with
  GICC_CTLR.FIQEn = 0). This is what the bare-metal library does
  (`gic_init_ci_di` in `software/lib/sw_lib`) and what the Linux EL3 shim for
  the FPGA boards does (`shim_g0`). Firmware that expects the Non-secure view
  and Group 1 interrupts (a conventional EL3 monitor handing a Non-secure OS the
  Group 1 path) will not receive interrupts on this SoC.
* **Regeneration.** RTL generated before this change differs from RTL generated
  after it only in the constant region-security inputs of the decoders
  (`security58` and `security62` in the `nic400_megasoc_main` slave interfaces,
  and the SYS_SRAM equivalent in `nic400_megasoc_system`), which go from 1 to 0.
  The two are functionally identical: the decoder computes
  `remap_decode = ~(aprot & securityNN) & decode_int` with `aprot` forced to 0,
  so the constant never takes effect. There is no need to rebuild for this
  change alone.
* **If TrustZone is wanted later.** The configuration that enables it was
  worked out and demonstrated in RTL simulation under MBT-68: A53, EXP_S and
  DMA slave interfaces as *Per access* (forward AxPROT[1]), the GIC master
  interface accepting both security states (the GIC-400 selects its Secure or
  Non-secure register view from AxPROT[1] itself), a deliberate choice for the
  flash XIP window, and EL3 firmware that puts the OS interrupts in Group 1. See
  the MBT-68 comments for the per-interface table and the verification plan.
