#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	ADD,
	ADDS,
	SUB,
	SUBS,
	AND,
	ANDS,
	ORR,
	EOR,
	LSL,
	LSR,
	ASR,
	MUL,
	UMULH,
	SMULH,
	UDIV,
	SDIV,
	LDUR,
	STUR,
	LDURB,
	STURB,
	LDURH,
	STURH,
	LDURSW,
	ADDI,
	ADDIS,
	SUBI,
	SUBIS,
	ANDI,
	ORRI,
	EORI,
	MOVZ,
	MOVK,
	MOVN,
	MOV,
	CBZ,
	CBNZ,
	B,
	BL,
	BR,
	CMP,
	CMPI,
	NOP,
	RET,
	SXTW,
	SXTB,
	SXTH,
	UXTB,
	UXTH,
	UXTW,
	B_EQ,
	B_NE,
	B_GT,
	B_LT,
	B_GE,
	B_LE,
	NUM_INSTRUCTIONS
} Opcode;

typedef enum {
	R_TYPE,
	I_TYPE,
	D_TYPE,
	B_TYPE,
	CB_TYPE,
	IM_TYPE,
	UNKNOWN_TYPE
} InstructionFormat;

InstructionFormat instruction_formats[NUM_INSTRUCTIONS] = {
	[ADD] = R_TYPE,	  [ADDS] = R_TYPE,	[SUB] = R_TYPE,	   [SUBS] = R_TYPE,
	[AND] = R_TYPE,	  [ANDS] = R_TYPE,	[ORR] = R_TYPE,	   [EOR] = R_TYPE,
	[LSL] = R_TYPE,	  [LSR] = R_TYPE,	[ASR] = R_TYPE,	   [MUL] = R_TYPE,
	[UMULH] = R_TYPE, [SMULH] = R_TYPE, [UDIV] = R_TYPE,   [SDIV] = R_TYPE,
	[LDUR] = D_TYPE,  [STUR] = D_TYPE,	[LDURB] = D_TYPE,  [STURB] = D_TYPE,
	[LDURH] = D_TYPE, [STURH] = D_TYPE, [LDURSW] = D_TYPE, [ADDI] = I_TYPE,
	[ADDIS] = I_TYPE, [SUBI] = I_TYPE,	[SUBIS] = I_TYPE,  [ANDI] = I_TYPE,
	[ORRI] = I_TYPE,  [EORI] = I_TYPE,	[MOVZ] = IM_TYPE,  [MOVK] = IM_TYPE,
	[MOVN] = IM_TYPE, [MOV] = IM_TYPE,	[CBZ] = CB_TYPE,   [CBNZ] = CB_TYPE,
	[B] = B_TYPE,	  [BL] = B_TYPE,	[BR] = B_TYPE,	   [CMP] = R_TYPE,
	[CMPI] = I_TYPE,  [NOP] = R_TYPE,	[RET] = R_TYPE,	   [SXTW] = R_TYPE,
	[SXTB] = R_TYPE,  [SXTH] = R_TYPE,	[UXTB] = R_TYPE,   [UXTH] = R_TYPE,
	[UXTW] = R_TYPE,  [B_EQ] = B_TYPE,	[B_NE] = B_TYPE,   [B_GT] = B_TYPE,
	[B_LT] = B_TYPE,  [B_GE] = B_TYPE,	[B_LE] = B_TYPE,
};

typedef struct {
	char *rd, *rn, *rm, *shamt;
} RVals;
typedef struct {
	char *rd, *rn, *imm12;
} IVals;
typedef struct {
	char *rt, *rn, *addr9;
} DVals;
typedef struct {
	char *imm26;
} BVals;
typedef struct {
	char *rt, *imm19;
} CBVals;
typedef struct {
	char *rd, *imm16, *sh;
} IMVals;

typedef struct {
	Opcode type;
	InstructionFormat format;
	union {
		RVals R;
		IVals I;
		DVals D;
		BVals B;
		CBVals CB;
		IMVals IM;
	} values;
} Instruction;

typedef struct {
	int instructions_count;
	Instruction *instructions;
} Inputs;

static void skip_spaces(const char *s, size_t *i) {
	while (s[*i] == ' ' || s[*i] == '\t')
		(*i)++;
}

static char *parse_token(const char *s, size_t *i, const char *stoppers) {
	char buf[32];
	size_t u = 0;

	skip_spaces(s, i);

	while (s[*i] != '\0' && strchr(stoppers, s[*i]) == NULL &&
		   u < sizeof(buf) - 1) {
		buf[u++] = s[*i];
		(*i)++;
	}

	buf[u] = '\0';
	return strdup(buf);
}

static void skip_comma_and_spaces(const char *s, size_t *i) {
	skip_spaces(s, i);
	if (s[*i] == ',')
		(*i)++;
	skip_spaces(s, i);
}

void setInstructionValues(Instruction *instruction, const char *line) {
	size_t i = 0;

	skip_spaces(line, &i);
	while (line[i] != '\0' && !isspace((unsigned char)line[i]))
		i++;
	if (instruction->format == R_TYPE) {
		instruction->values.R.rd = parse_token(line, &i, ", \t\n");
		skip_comma_and_spaces(line, &i);

		instruction->values.R.rn = parse_token(line, &i, ", \t\n");
		skip_comma_and_spaces(line, &i);

		instruction->values.R.rm = parse_token(line, &i, ", \t\n");

		instruction->values.R.shamt = NULL;
	} else if (instruction->format == I_TYPE) {
		instruction->values.I.rd = parse_token(line, &i, ", \t\n");
		skip_comma_and_spaces(line, &i);

		instruction->values.I.rn = parse_token(line, &i, ", \t\n");
		skip_comma_and_spaces(line, &i);

		skip_spaces(line, &i);
		if (line[i] == '#')
			i++;
		instruction->values.I.imm12 = parse_token(line, &i, " \t\n");
	} else if (instruction->format == D_TYPE) {
		instruction->values.D.rt = parse_token(line, &i, ", \t\n");
		skip_comma_and_spaces(line, &i);

		skip_spaces(line, &i);
		if (line[i] == '[')
			i++;

		instruction->values.D.rn = parse_token(line, &i, ", ]\t\n");
		skip_comma_and_spaces(line, &i);

		skip_spaces(line, &i);
		if (line[i] == '#')
			i++;
		instruction->values.D.addr9 = parse_token(line, &i, "] \t\n");

		skip_spaces(line, &i);
		if (line[i] == ']')
			i++;
	} else if (instruction->format == B_TYPE) {
		skip_spaces(line, &i);
		instruction->values.B.imm26 = parse_token(line, &i, " \t\n");
	} else if (instruction->format == CB_TYPE) {
		instruction->values.CB.rt = parse_token(line, &i, ", \t\n");
		skip_comma_and_spaces(line, &i);

		instruction->values.CB.imm19 = parse_token(line, &i, " \t\n");
	} else if (instruction->format == IM_TYPE) {
		instruction->values.IM.rd = parse_token(line, &i, ", \t\n");
		skip_comma_and_spaces(line, &i);

		skip_spaces(line, &i);
		if (line[i] == '#')
			i++;
		instruction->values.IM.imm16 = parse_token(line, &i, ", \t\n");

		skip_comma_and_spaces(line, &i);

		instruction->values.IM.sh = NULL;
		if (line[i] != '\0') {
			if (line[i] == 'L') {
				while (line[i] != '\0' && !isspace((unsigned char)line[i]))
					i++;
			}
			skip_spaces(line, &i);
			if (line[i] == '#')
				i++;
			instruction->values.IM.sh = parse_token(line, &i, " \t\n");
		}
	}
}

Instruction parseInstructionFromUser(char *instruction_unparsed) {
	Instruction instruction;

	char instruction_str[16];
	size_t i = 0;

	while (instruction_unparsed[i] != '\0' && instruction_unparsed[i] != ' ' &&
		   i < sizeof instruction_str - 1) {
		instruction_str[i] = instruction_unparsed[i];
		i++;
	}
	instruction_str[i] = '\0';

	if (strcmp(instruction_str, "ADD") == 0) {
		instruction.type = ADD;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "ADDS") == 0) {
		instruction.type = ADDS;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "SUB") == 0) {
		instruction.type = SUB;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "SUBS") == 0) {
		instruction.type = SUBS;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "AND") == 0) {
		instruction.type = AND;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "ANDS") == 0) {
		instruction.type = ANDS;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "ORR") == 0) {
		instruction.type = ORR;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "EOR") == 0) {
		instruction.type = EOR;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "LSL") == 0) {
		instruction.type = LSL;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "LSR") == 0) {
		instruction.type = LSR;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "ASR") == 0) {
		instruction.type = ASR;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "MUL") == 0) {
		instruction.type = MUL;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "UMULH") == 0) {
		instruction.type = UMULH;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "SMULH") == 0) {
		instruction.type = SMULH;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "UDIV") == 0) {
		instruction.type = UDIV;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "SDIV") == 0) {
		instruction.type = SDIV;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "LDUR") == 0) {
		instruction.type = LDUR;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "STUR") == 0) {
		instruction.type = STUR;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "LDURB") == 0) {
		instruction.type = LDURB;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "STURB") == 0) {
		instruction.type = STURB;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "LDURH") == 0) {
		instruction.type = LDURH;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "STURH") == 0) {
		instruction.type = STURH;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "LDURSW") == 0) {
		instruction.type = LDURSW;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "ADDI") == 0) {
		instruction.type = ADDI;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "ADDIS") == 0) {
		instruction.type = ADDIS;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "SUBI") == 0) {
		instruction.type = SUBI;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "SUBIS") == 0) {
		instruction.type = SUBIS;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "ANDI") == 0) {
		instruction.type = ANDI;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "ORRI") == 0) {
		instruction.type = ORRI;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "EORI") == 0) {
		instruction.type = EORI;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "MOVZ") == 0) {
		instruction.type = MOVZ;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "MOVK") == 0) {
		instruction.type = MOVK;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "MOVN") == 0) {
		instruction.type = MOVN;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "MOV") == 0) {
		instruction.type = MOV;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "CBZ") == 0) {
		instruction.type = CBZ;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "CBNZ") == 0) {
		instruction.type = CBNZ;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "B") == 0) {
		instruction.type = B;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "BL") == 0) {
		instruction.type = BL;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "BR") == 0) {
		instruction.type = BR;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "CMP") == 0) {
		instruction.type = CMP;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "CMPI") == 0) {
		instruction.type = CMPI;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "NOP") == 0) {
		instruction.type = NOP;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "RET") == 0) {
		instruction.type = RET;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "SXTW") == 0) {
		instruction.type = SXTW;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "SXTB") == 0) {
		instruction.type = SXTB;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "SXTH") == 0) {
		instruction.type = SXTH;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "UXTB") == 0) {
		instruction.type = UXTB;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "UXTH") == 0) {
		instruction.type = UXTH;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "UXTW") == 0) {
		instruction.type = UXTW;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "B.EQ") == 0) {
		instruction.type = B_EQ;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "B.NE") == 0) {
		instruction.type = B_NE;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "B.GT") == 0) {
		instruction.type = B_GT;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "B.LT") == 0) {
		instruction.type = B_LT;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "B.GE") == 0) {
		instruction.type = B_GE;
		instruction.format = instruction_formats[instruction.type];
	} else if (strcmp(instruction_str, "B.LE") == 0) {
		instruction.type = B_LE;
		instruction.format = instruction_formats[instruction.type];
	} else {
		instruction.type = NUM_INSTRUCTIONS;
		instruction.format = UNKNOWN_TYPE;
		printf("Unknown instruction: %s\n", instruction_str);
	}

	setInstructionValues(&instruction, instruction_unparsed);

	return instruction;
}

void printInstruction(const Instruction *ins) {
	printf("Opcode = %d\n", ins->type);

	switch (ins->format) {

	case R_TYPE:
		printf("Format = R_TYPE\n");
		printf("  rd    = %s\n", ins->values.R.rd);
		printf("  rn    = %s\n", ins->values.R.rn);
		printf("  rm    = %s\n", ins->values.R.rm);
		printf("  shamt = %s\n", ins->values.R.shamt);
		break;

	case I_TYPE:
		printf("Format = I_TYPE\n");
		printf("  rd     = %s\n", ins->values.I.rd);
		printf("  rn     = %s\n", ins->values.I.rn);
		printf("  imm12  = %s\n", ins->values.I.imm12);
		break;

	case D_TYPE:
		printf("Format = D_TYPE\n");
		printf("  rt     = %s\n", ins->values.D.rt);
		printf("  rn     = %s\n", ins->values.D.rn);
		printf("  addr9  = %s\n", ins->values.D.addr9);
		break;

	case B_TYPE:
		printf("Format = B_TYPE\n");
		printf("  imm26 = %s\n", ins->values.B.imm26);
		break;

	case CB_TYPE:
		printf("Format = CB_TYPE\n");
		printf("  rt     = %s\n", ins->values.CB.rt);
		printf("  imm19  = %s\n", ins->values.CB.imm19);
		break;

	case IM_TYPE:
		printf("Format = IM_TYPE\n");
		printf("  rd     = %s\n", ins->values.IM.rd);
		printf("  imm16  = %s\n", ins->values.IM.imm16);
		printf("  sh     = %s\n", ins->values.IM.sh);
		break;

	default:
		printf("Format = UNKNOWN\n");
		break;
	}
}

Inputs getUserInputs() {
	Inputs inputs;
	printf("Number of Instructions: ");
	scanf("%d", &inputs.instructions_count);
	printf("\n");

	inputs.instructions =
		(Instruction *)malloc(inputs.instructions_count * sizeof(Instruction));

	for (int i = 1; i <= inputs.instructions_count; i++) {
		char instruction_unparsed[64];

		printf("%i) ", i);
		scanf(" %[^\n]", instruction_unparsed);

		inputs.instructions[i - 1] =
			parseInstructionFromUser(instruction_unparsed);
		printInstruction(&inputs.instructions[i - 1]);
	}

	return inputs;
}

int isLoad(Instruction ins) {
	return ins.type == LDUR || ins.type == LDURB || ins.type == LDURH ||
		   ins.type == LDURSW;
}

int instructionReadsRegister(Instruction ins, const char *reg) {

	switch (ins.format) {

	case R_TYPE:
		return (ins.values.R.rn && strcmp(ins.values.R.rn, reg) == 0) ||
			   (ins.values.R.rm && strcmp(ins.values.R.rm, reg) == 0);

	case I_TYPE:
		return (ins.values.I.rn && strcmp(ins.values.I.rn, reg) == 0);

	case D_TYPE:
		if (ins.type == STUR || ins.type == STURH || ins.type == STURB) {
			return (ins.values.D.rn && strcmp(ins.values.D.rn, reg) == 0) ||
				   (ins.values.D.rt && strcmp(ins.values.D.rt, reg) == 0);
		}
		return 0;

	case CB_TYPE:
		return (ins.values.CB.rt && strcmp(ins.values.CB.rt, reg) == 0);

	case B_TYPE:
		return 0;

	default:
		return 0;
	}
}

void printChart(Inputs ins) {
	int stalls = 0;
	for (int i = 0; i < ins.instructions_count; i++) {

		for (int u = 0; u < i + stalls; u++)
			printf("     ");
		printf("|IF  |ID  |EX  |ME  |WB  |\n");
		Instruction prev = ins.instructions[i];
		Instruction next = ins.instructions[i + 1];

		if (isLoad(prev)) {
			char *loaded = prev.values.D.rt;

			if (instructionReadsRegister(next, loaded)) {
				stalls++;
			}
		}
	}
}
void printTotalCycleCount(Inputs ins) {
	int stalls = 0;
	for (int i = 0; i < ins.instructions_count; i++) {

		Instruction prev = ins.instructions[i];
		Instruction next = ins.instructions[i + 1];

		if (isLoad(prev)) {
			char *loaded = prev.values.D.rt;

			if (instructionReadsRegister(next, loaded)) {
				stalls++;
			}
		}
	}
	printf("Total Cycle Count: %d\n", 4 + ins.instructions_count + stalls);
}

int main(int argc, char **argv) {
	Inputs inputs;

	while (0 == 0) {
		printf("Performance Assessment\n");
		printf("----------------------\n");
		printf("1) Enter instructions\n");
		printf(
			"2) Print a chart of the pipelined stages of the instructions\n");
		printf("3) Print the total cycle count for the program\n");
		printf("4) Quit\n");

		int choice;
		printf("\nEnter selection: ");
		scanf("%d", &choice);

		switch (choice) {
		case 1:
			inputs = getUserInputs();
			break;
		case 2:
			printChart(inputs);
			break;
		case 3:
			printTotalCycleCount(inputs);
			break;
		case 4:
			return 0;
			break;
		default:
			break;
		}
	}

	return 0;
}
