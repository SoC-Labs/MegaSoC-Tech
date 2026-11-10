# MegaSoC Tech

Part of the MegaSoC SoC reference design.


## Pre-requisites
You will need the following IP from Arm, all available through Arm Academic Access (AAA)

-   Cortex A53
-   GIC 400
-   PL011
-   PL022
-   PCK600
-   SIE300
-   NIC400

You will also need installed
-   Socrates
-   A simulator (e.g. QuestaSim, VCS)

For FPGA development you will need a Vivado installation

## Usage
We suggest that you use this with the [megaSoC project repository](https://github.com/SoC-Labs/MegaSoC-Project) as the top level

## Memory Map
The address map is documented in the MegaSoC-Project datasheet (`docs/src/writing_software.rst`, "Address Map").

Security: megaSoC has no TrustZone partitioning. Every NIC-400 slave interface is configured Secure, so all transactions inside the interconnect are Secure and no region is Secure-only. See `socrates/README.md` and Jira MBT-68.

## Support
For support please go to [soclabs.org](https://soclabs.org/)

## Contributing
Open to collaborations, if you're interested please head over to soclabs.org and register your interest on the megasoc reference design project

## Authors and acknowledgment
Daniel Newbrook (d.newbrook@soton.ac.uk)
Shiva Sangati
Anastasios Dimitriou

## Project status
Under Development
