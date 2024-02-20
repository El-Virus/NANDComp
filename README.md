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
- Constants are defined by specifying a '&' character before name = value. E.g. `&constVal = 0`.
- Macros are defined by adding a '%' character before the name, and a '$' sign before the arguments. Macros end with "%%".
	- Macros allow for variadic arguments with "$$", which can be accessed with "$i$" where i is the index of the vargument, it also allows for the combination of all vargs using a certain operation (In order to be evaluated by the preprocessor (see below))

E.g.
```
%MACRO_NAME $arg
A = $arg
A = A + 1
%%

%EG_VARGS_MACRO $fixed $$ $this_arg_will_be_ignored
#A will be set to the 2nd variadic argument, in this case the 3rd argument
A = $1$
D = A
#This will expand into ,($fixed * ($0$ + $1$ + $2$ + ...)) to be evaluated by the preprocessor
A = ,($fixed * ($+$))
%%

#This will set D to 6 and A to 32 (4 * (2 + 6))
EG_VARGS_MACRO 4 2 6
```
- Files can be included by inserting a '@' character before a filename (That line will be replaced with the code in the file).
- The assembler will look for "macros.src" and include it automatically (Note that only constants and macros will be parsed unless manually included).
- You can declare labels by using the LABEL keyword
- An additional instruction that does not exist within NandGame, "MACH", has been added, similar to the C instruction, "ASM", can be used to insert code in a lower level programming language, in this case, machine code.
	- It takes a single argument, a number (either decimal, binary or hexadecimal) which will be inserted as an instruction.
- The "SIM" macro (defined in macros.src) interacts with the advanced simulator (see below), it takes one or more parameters, which are:
	- CLRS (Clear screen), DUMP (Dump the registers, current instruction, and memory value at A, either if a jump is about to be or has just been perormed), "HOLD" (Wait for user keypress before continuing) and "STOP" (Stop the simulator).
		- The parameters will run in the order in which they're mentioned above.
- The preprocessor will evaluate the contents of basic arithmeticological operations (~ (or !) * / % + - & ^ (XOR) |) inside of ",()" according to the order of operations. E.g. `A = ,(4 | (5 + 1) / 2)` will be evaluated to `A = 7`
	- The preprocessor will work with numbers with up to 32 bits, but keep in mind that the the maximum number in a "A = num" instruction has 15 bits

### Compiling the Assembler
Just compile the Assembler and Misc CPP files with a C++17 capable compiler.

## Advanced simulator (sim.c)
For those who are interested on running code, rather than looking at a code based implementation the of NandGame computer and being able to run code slowly, can use the faster advanced simulator, which will understand "SIM" instructions (mentioned above) that make code easier to debug.

The assembler accepts a "-d" flag which will make it perform a dump after each instruction. (Ignoring "SIM" instructions)

### Compiling the simulator
It should be compilable using your favourite C compiler, it's a standalone file.

## License
This work is licensed under a [Creative Commons Attribution-NonCommercial 4.0 International License](http://creativecommons.org/licenses/by-nc/4.0/).
