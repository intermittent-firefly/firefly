# Resources

We provide resources which will assist you working with firefly.

* bin/: Pre-compiled 64 bits RISC-V binaries compliant with firefly simulation. All binaries come from benchmarks provided by [riscv-tests](https://github.com/riscv/riscv-tests);
* riscv-tests/: To facilitate building new binaries compliant to firefly, we provide routines which handle interrupts, and communicate with the simulation environment. To use them, just copy the containing files to [riscv-tests/benchmarks/common](https://github.com/riscv/riscv-tests/tree/master/benchmarks/common). These routines assume that you are using RocketDefaultConfig, they may not work properly if you change PLIC's base address.
* votlages/: Voltage input files which can be used in firefly simulations. 