# NANDComp
## About
NANDComp is an emulator of a CPU based on [NandGame](https://nandgame.com/).

It's written from a scratch and only uses standard libraries.

All the code originates from the NAND() function, one of the (What I like to call) "Logical Exceptions", which are pieces of code which can use if statements and arithmeticological operations that aren't originated from NAND.
At the moment the two "Logical Exceptions" are: NAND(Yhe base function itself) & BITMAN(Namespace emulating bit operations).

## Running
The emulator reads the code it should run from a file containing the decimal encoded instructions.

After it finishes, it displays the last value stored in the A register.

## Compiling
Just grab your favourite compiler and compile all the CPP files in the base dir.

## Assembler
The assembler, is a little program that assembles code as described in the official NAND Game website into binary or decimal (if the -d flag is present) code.
Constants are defined by specifying a "&" character before name = value. Eg. `&constVal = 0`. At the moment negative values are not supported (You can use 0xffff)

### Compiling the Assembler
Just compile the Assembler and Misc CPP files.

## License
This work is licensed under a [Creative Commons Attribution-NonCommercial 4.0 International License](http://creativecommons.org/licenses/by-nc/4.0/).