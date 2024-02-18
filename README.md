# NANDComp
## About
NANDComp is an emulator of a CPU based on [NandGame](https://nandgame.com/).

It's written from a scratch and only uses standard libraries.

All the code originates from the NAND() function, one of the (What I like to call) "Logical Exceptions", which are pieces of code which can use arithmeticological operations.
At the moment the two "Logical Exceptions" are: NAND(The base function itself) & BITMAN(Namespace emulating bit operations).

## Running
The emulator reads the code it should run from a file containing the decimal encoded instructions.

After it finishes, it displays the last value stored in the A register.

## Compiling
Just grab your favourite compiler and compile all the CPP files in the base dir.

## Assembler
The assembler, is a little program that assembles code as described in the official NAND Game website into binary or decimal (if the -d flag is present) code.

### The language
- Constants are defined by specifying a '&' character before name = value. Eg. `&constVal = 0`.
- Macros are defined by adding a '%' character before the name, and a '$' sign before the arguments. Macros end with "%%". eg:
```
%MACRO_NAME $arg
A = $arg
A = A + 1
%%
```
- Files can be included by inserting a '@' character before a filename (That line will be replaced with the code in the file).
- The assembler will look for "macros.src" and include it automatically (Note that only constants and macros will be parsed unless manually included).
- You can declare labels by using the LABEL keyword
- Some instructions that do not exist within NandGame have been added:
	- The "MACH" instruction, similar to the C instruction, "ASM", can be used to insert code in a lower level programming language, in this case, machine code.
		- It takes a single argument, a number (either decimal, binary or hexadecimal) which will be inserted as an instruction.
	- The "SIM" instruction interacts with the advanced simulator (see below), it takes one or more parameters (defined in macros.src) separated by a comma.
		- The parameters are: CLRS (Clear screen), DUMP (Dump the registers, current instruction, and memory value at A, either if a jump is about to be performed or has just been perormed), "HOLD" (Wait for user keypress before continuing) and "STOP" (Stop the simulator).
			- The parameters will run in the order in which they're mentioned above.
- The preprocessor will evaluate the contents of basic arithmeticological operations
	- The preprocessor will work with numbers up to INT_MAX, but keep in mind that the the maximum number in a "A = num" instruction is SHORT_MAX

### Compiling the Assembler
Just compile the Assembler and Misc CPP files with a C++17 capable compiler.

## Advanced simulator (sim.c)
For those who are interested on running code, rather than looking at a code based implementation the of NandGame computer and being able to run code slowly, can use the faster advanced simulator, which will understand "SIM" instructions (mentioned above) that make code easier to debug.

The assembler accepts a "-d" flag which will make it perform a dump after each instruction. (Ignoring "SIM" instructions)

### Compiling the simulator
It should be compilable using your favourite C compiler, it's a standalone file.

## License
This work is licensed under a [Creative Commons Attribution-NonCommercial 4.0 International License](http://creativecommons.org/licenses/by-nc/4.0/).
