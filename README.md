# Firefly - The Intermittent Rocket Simulator

Firefly extends rocket-chip simulation environment to enable simulation in intermittent scenarios. By providing a voltage input, and a threshold value, it is possible to analyze Rocket's execution trace to measure power losses impact, and test different energy fault handle routines. Besides that, firefly generates a memory trace which can be used with NVmain to assess memory energy consumption with different non-volatile memory technologies.

Cloning:
```
git clone --recurse-submodules https://github.com/intermittent-firefly/firefly.git
```
or
```
git clone https://github.com/intermittent-firefly/firefly.git
cd firefly
git submodule update --init --recursive
```

## Dependencies

Firefly depends on rocket-chip, NVmain, and Python 3.6 (and matplotlib). 

```
sudo apt install -y python3 python3-pip
pip3 install matplotlib
```

If you never used rocket-chip, you will need [rocket-tools](https://github.com/chipsalliance/rocket-tools), which will provide the riscv toolchain. To build rocket, it is necessary to have a `$RISCV` environment variable pointing to rocket-tools install path. You will find more details in rocket-chip/tools documentation.

If you already have rocket-tools (and exported RISCV variable), just type:

```
./build.sh
```

It will build two versions of rocket-chip (normal and debug) with firefly extension, and NVMain. A `./bin` folder will be created containing binaries used by firefly.

## Usage

To simulate a binary with firefly, you **must** provide a voltage threshold, a voltage input file, and a compliant binary. An example command line:

```
./firefly -i resources/voltages/1.txt -t 0.7 resources/bin/dhrystone.riscv
```

What is happening? Firefly is launched with `resources/voltages/1.txt` as voltage input file, a threshold of 0.7 V (`-t 0.7`), and dhrystone binary is simulated. For convenience, we provide voltage files and compliant binaries (all from [riscv-tests](https://github.com/riscv/riscv-tests) in resources folder. If you want to compile new binaries, we provide files to make an easy, integration with riscv-tests.

## How does firefly work?

Firefly is composed by python scripts, and C++ code which extends rocket's simulation environment. The python part administrate the workflow, launching simulations, and organizing data. The C++ code is where the work is really done. At each simulation, a voltage trace file is read. At each step number of simulation cycles, a new entry of voltage trace file is read, and the value is compared to the threshold. Whenever the new value is below the threshold, an external interrupt is fired. By default, a new voltage entry is read at every [120 cycles](https://github.com/Zeldax64/firefly/blob/193ef09fb9ef38126d53a1fa2d512964340567e9/riscv/firefly-rocket/csrc/firefly.cc#L13).

In Rocket, external interrupts are routed to PLIC. If PLIC is configured to accept external interrupts, then the Rocket core will jump to an interrupt routine. To create a communication interface between the simulation environment and the processor, we constantly probe the mscratch CSR. We chose this register because it is never written by hardware, and any write or read to a CSR is almost instantaneous.

This enable the simulation environment to know Rocket's states along the binary execution, e.g. if its handling a power emergency interruption, turned off or executing the application.

## To cite this work:

@inproceedings{firefly,
  author    = {Hiago Rocha and
               Guilherme Korol and
               Michael Guilherme Jordan and
               Arthur M. Krause and
               Ronaldo Silveira and
               Caio Vieira and
               Philippe O. A. Navaux and
               Gabriel L. Nazar and
               Luigi Carro and
               Antonio Carlos Schneider Beck},
  title     = {Firefly: An Open-source Rocket-based Intermittent Framework},
  booktitle = {33rd Symposium on Integrated Circuits and Systems Design, {SBCCI}
               2020, Campinas, Brazil, August 24-28, 2020},
  pages     = {1--6},
  publisher = {{IEEE}},
  year      = {2020}
}
