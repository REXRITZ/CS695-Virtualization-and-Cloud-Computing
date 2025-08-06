# CS695 Assignment 2 - Part 2

hypervisor-warmup is a example program to demonstrate running of two VM using KVM API. Tested in Intel processors with the VMX hardware virtualization extensions and AMD processors with AMD-V hardware virtualization extensions.

## How to run
Performing ```make``` will compile all the files in the program.

````bash
make
````
To run emu1.c:
````bash
./emu1
````
To run emu2.c:
````bash
./emu2
````
To run emu3.c:
````bash
./emu3
````
To run emu4.c:
````bash
./emu4 <filename>
````

Performing a ```make clean``` will remove the executable and the object files. You should perform this while updating the code and recompiling.
