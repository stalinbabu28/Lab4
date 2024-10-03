# RISC-V Simulator

## Usage

Create an input file containing the assembly code and make sure that it is present in the same directory as the simulator.
This code will run in an infinite loop waiting for instructions.
Run the simulator and then use a few commands on it after loading it by typing load <inputfilename>.

run : Run the program from the beginning to the end.
regs : Display the values of all registers.
mem <addr> <count> : Show the memory content from the starting address (addr) to (addr + count) address.
step : Execute the program one instruction at a time, displaying the state after each step.
break <line> : Sets a mark to stop the code execution once the line is reached, preserving registers and memory state.
del break <line>: Deletes the breakpoint at the specified line.
exit : exits the simulator.

The simulator assumes specific input formatting and does not support pseudo-instructions.
Error messages may not always be descriptive for complex input errors.


## Makefile usage

Make can be used the form the object file into our local repo:

```console
make all
```

make all will compile main.cpp file and gives the riscv_asm.exe file.\
we can execute the riscv_asm.exe file using:

```console
./riscv_asm
```

clean can be used to clean all the files:

```console
make clean
```




