#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MEMSZ 65536 //16k

#define WORD_TO_BIN_P "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define WORD_TO_BIN(byte)  \
	((byte) & 0x8000 ? '1' : '0'), ((byte) & 0x4000 ? '1' : '0'), ((byte) & 0x2000 ? '1' : '0'), ((byte) & 0x1000 ? '1' : '0'), \
	((byte) & 0x0800 ? '1' : '0'), ((byte) & 0x0400 ? '1' : '0'), ((byte) & 0x0200 ? '1' : '0'), ((byte) & 0x0100 ? '1' : '0'), \
	((byte) & 0x0080 ? '1' : '0'), ((byte) & 0x0040 ? '1' : '0'), ((byte) & 0x0020 ? '1' : '0'), ((byte) & 0x0010 ? '1' : '0'), \
	((byte) & 0x0008 ? '1' : '0'), ((byte) & 0x0004 ? '1' : '0'), ((byte) & 0x0002 ? '1' : '0'), ((byte) & 0x0001 ? '1' : '0')


enum instruct {
	COND_GT	= 0b0000000000000001,
	COND_EQ	= 0b0000000000000010,
	COND_LT	= 0b0000000000000100,
	ALU_SW	= 0b0000000000001000,
	ALU_ZX	= 0b0000000000010000,
	ALU_OP0 = 0b0000000000100000,
	ALU_OP1 = 0b0000000001000000,
	ALU_U	= 0b0000000010000000,
	DEST_AM = 0b0000000100000000,
	DEST_D	= 0b0000001000000000,
	DEST_A	= 0b0000010000000000,
	// UNUSED 0b0000100000000000
	CF_UMEM = 0b0001000000000000,
	// UNUSED 0b0010000000000000
	CF_SACT = 0b0100000000000000,
	CF_CI	= 0b1000000000000000
};

enum sinstruct {
	SF_STOP	= 0b0000000000000001,
	SF_HOLD	= 0b0000000000000010,
	SF_DUMP	= 0b0000000000000100,
	SF_CLRS = 0b0000000000001000,
	// UNUSED 0b0000000000010000,
	// UNUSED 0b0000000000100000,
	// UNUSED 0b0000000001000000,
	// UNUSED 0b0000000010000000,
	// UNUSED 0b0000000100000000,
	// UNUSED 0b0000001000000000,
	// UNUSED 0b0000010000000000,
	// UNUSED 0b0000100000000000
	// UNUSED 0b0001000000000000,
	// UNUSED 0b0010000000000000
	//INVALID 0b0100000000000000,
	//INVALID 0b1000000000000000
};

#define SFLAG(flag) (instruction & flag)

static uint16_t	A;
static uint16_t	D;
static uint16_t	MEM[MEMSZ];
static uint16_t	PC;
static uint16_t	*ROM;
static uint16_t	ROMSZ;
static bool j;

uint16_t run_ALU(uint16_t X, uint16_t Y, bool use_ram, bool swap, bool zerox, bool op0, bool op1, bool do_arith) {
	uint16_t pX, pY;
	if (use_ram) {
		pY = MEM[Y];
	} else {
		pY = Y;
	}
	
	if (swap) {
		pX = pY;
		pY = X;
	} else {
		pX = X;
	}

	if (zerox)
		pX = 0;
	
	if (do_arith) {
		if (op0)
			pY = 1;
		return (op1 ? pX - pY : pX + pY);
	} else {
		char op = ((op1 << 1) | op0);
		switch (op) {
			case 0:
				return (pX & pY);
			case 1:
				return (pX | pY);
			case 2:
				return (pX ^ pY);
			case 3:
				return ~pX;
		}
	}
}

void debug_dump(bool last) {
	printf("╔═══════════════════Debug Dump═══════════════════╗\n");
if (j)
	printf("║NOTE: Last instruction was a jump\t\t ║\n");
if (last && !j)
	printf("║PC was: %d, which pointed to:\t "WORD_TO_BIN_P"║\n", (PC - 1), WORD_TO_BIN(ROM[PC - 1]));
else
	printf("║PC is: %d, which points to:\t "WORD_TO_BIN_P"║\n", PC, WORD_TO_BIN(ROM[PC]));
	printf("║Registers are as follows:\t\t\t ║\n");
	printf("║⌠Register A\t=%d\t=%#x\t="WORD_TO_BIN_P"║\n", A, A, WORD_TO_BIN(A));
	printf("║⌡Register D\t=%d\t=%#x\t="WORD_TO_BIN_P"║\n", D, D, WORD_TO_BIN(D));
if (!(last && j))
	printf("║Memory at A\t=%d\t=%#x\t="WORD_TO_BIN_P"║\n", MEM[A], MEM[A], WORD_TO_BIN(MEM[A]));
if (!last)
	if (j)
		printf("║We're about to jump %d lines.\t\t\t ║\n", (A - PC));
	printf("╚════════════════════════════════════════════════╝\n");
}

int main(int argc, char** argv) {
	bool debug_mode = false;
	char *filename;
	//Parse args
    if (argc < 2 || argc > 3) {
        printf("Usage: %s [-d] <program.bit>\n", argv[0]);
        return 1;
    } else if (argc == 2) {
        filename = argv[1];
    } else if (argc == 3) {
        if (!strcmp(argv[1], "-d"))
            debug_mode = true;
        filename = argv[2];
    }

	//Read file
	FILE *bitf = fopen(filename, "r");
	if (bitf == NULL) {
		printf("File could not be read.\n");
		exit(1);
	}

	char readl[18];
	while (fgets(readl, 18, bitf)) {
		assert(readl[16] == '\n');
		assert(ROMSZ != UINT16_MAX);
		ROMSZ++;
		ROM = realloc(ROM, sizeof(uint16_t) * ROMSZ); //OK in accordance to 7.22.3.5
		ROM[ROMSZ-1] = (uint16_t)strtoul(readl, NULL, 2);
	}

	fclose(bitf);

	//Process
	uint16_t instruction = ROM[0];
	uint16_t R;
	for (; PC < ROMSZ; instruction = ROM[++PC]) {
		//Handle data
		if (!SFLAG(CF_CI)) {
			A = instruction;
			goto ddb;
		}

		//Handle sim
		if (SFLAG(CF_SACT)) {
			if (SFLAG(SF_CLRS)) {
#ifdef _WIN32
				system("cls");
#else
				system("clear");
#endif
			}

			if (SFLAG(SF_DUMP) && (!debug_mode || SFLAG(SF_CLRS)) && PC != 0)
				debug_dump(true);
			
			if (SFLAG(SF_HOLD)) {
				printf("!Simulator paused, press any key to resume.");
				getchar();
			}

			if (SFLAG(SF_STOP)) {
				if (debug_mode) {
					printf("!End State Dump:\n");
					debug_dump(true);
				}
				break;
			}
			continue;
		}

		//Handle ALU
		R = run_ALU(D, A, SFLAG(CF_UMEM), SFLAG(ALU_SW), SFLAG(ALU_ZX), SFLAG(ALU_OP0), SFLAG(ALU_OP1), SFLAG(ALU_U));

		//Store result
		if (SFLAG(DEST_AM))
			MEM[A] = R;
		
		if (SFLAG(DEST_A))
			A = R;
		
		if (SFLAG(DEST_D))
			D = R;

		//Check jump conditions
		j = false;
		
		if (SFLAG(COND_GT) && R > 0)
			j = true;
		
		if (SFLAG(COND_EQ) && R == 0)
			j = true;

		if (SFLAG(COND_LT) && (int16_t)R < 0)
			j = true;
		goto ddba;
ddb:
		j = false;
ddba:
		//Dump if debug
		if (debug_mode)
			debug_dump(false);
		
		//Perform the jump
		if (j)
			PC = (A - 1);

	}
	return 0;
}
