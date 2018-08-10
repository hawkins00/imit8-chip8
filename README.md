# imit8-chip8

Copyright © 2018 Chris Kim & Matt Hawkins

This is a virtual machine for interpreting [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) code. CHIP-8 is a minimal, interpreted language for creating simplistic games for 8-bit computers.

Though there are many other Chip-8 implementations in existence, this particular project is aimed at being extensible, and suitable for use in teaching environments.

Extensibility in this project is emphasized in the structure/design of the program.  Though the current code base starts with the CPU core and Display implementations needed to run Chip-8 code (ROMs), the structure of the project is designed such that "displays" and "cores" can be written for other platforms, then added in a plug-in/drop-in fashion.

Extensive logging and the ability to display the current opcode in execution is also possible.  Future implementation plans include being able to slow down execution and step through one instruction at a time, while viewing memory states, etc. to be able to demonstrate the workings of a CPU.

## Building
The project was built in Jetbrains CLion, and therefore includes the CMake files necessary to build and run the project there.  It is worth noting, however, that there is nothing proprietary in use in the current code base that requires specific libraries or platforms.  As such, a simple "g++ *.cpp" should be sufficient to compile a working binary on either Windows or Linux platforms.

## Use
To load and run a ROM, place its path as the lone paramater to the program:
./imit8-chip8 dir/romfile.ch8

## Future Plans
The graphic output of the VM is ascii- / console-based. The experience could be improved by using an OpenGL library for more responsive display updates. The library could also be used to create actual game beeps.

# License

This program is licensed under the "GPLv3 License". Please see the file `License.md` in this distribution for license terms.

