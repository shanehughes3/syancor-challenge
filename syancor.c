#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#define MAX_INT 32768
#define VAL_ERR 65535
#define REG_ERR 65534

typedef struct {
	uint16_t memory[MAX_INT];
	uint16_t registers[8];
	uint16_t *mem_ptr;
	void (*instructions[22])();
	uint16_t *stack;
	uint16_t *STACK_LOW_BOUND;
} State;

State *initialize();
uint16_t read_source(State *state);
void run(State *state);

void _exit(State *state);
void _set(State *state);
void _push(State *state);
void _pop(State *state);
void _eq(State *state);
void _add(State *state);

void _jmp(State *state);
void _jt(State *state);
void _jf(State *state);

void _readChar(State *state);

void _noop(State *state);

int main(int argc, char *argv[])
{
	State *state = initialize();
	read_source(state);

	run(state);

	return 0;
}

State *initialize()
{
	State *state = (State *) malloc(sizeof(State));
	state->instructions[0] = _exit;
	state->instructions[1] = _set;
	state->instructions[2] = _push;
	state->instructions[3] = _pop;
	state->instructions[4] = _eq;
	state->instructions[5] = _noop;
	state->instructions[6] = _jmp;
	state->instructions[7] = _jt;
	state->instructions[8] = _jf;
	state->instructions[9] = _add;
	state->instructions[10] = _noop;
	state->instructions[11] = _noop;
	state->instructions[12] = _noop;
	state->instructions[13] = _noop;
	state->instructions[14] = _noop;
	state->instructions[15] = _noop;
	state->instructions[16] = _noop;
	state->instructions[17] = _noop;
	state->instructions[18] = _noop;
	state->instructions[19] = _readChar;
	state->instructions[20] = _noop;
	state->instructions[21] = _noop;
	state->mem_ptr = state->memory;
	for (int i = 0; i < 8; ++i) {
		state->registers[i] = 0;
	}
	return state;
}

uint16_t read_source(State *state) 
{
	FILE *input;
	uint16_t buff;
	
	if ((input = fopen("challenge.bin", "r")) == NULL ||
	    input == NULL) {
		fprintf(stderr, "synacor: Error opening challenge.bin");
		exit(-1);
	}
	while (fread(&buff, sizeof(uint16_t), 1, input) > 0) {
		*(state->mem_ptr++) = buff;
	}
	fclose(input);
	state->stack = state->STACK_LOW_BOUND = state->mem_ptr;
	state->mem_ptr = state->memory;
	
	return 0;
}

void run(State *state)
{
	uint16_t op;
	while (1) {
		op = *state->mem_ptr++;
		if (op > 21) {
			fprintf(stderr,
				"synacor: invalid opcode %d - 0x%.4lx\n",
				op, --state->mem_ptr - state->memory);
			free(state);
			exit(-2);
		}
		if (op != 19) {
			printf("op %d\n", op);
		}
		(*state->instructions[op])(state);
	}
}

uint16_t check_val(uint16_t *val, State *state) {
	if (*val > MAX_INT + 8) {
		fprintf(stderr,
			"synacor: invalid input value %d - 0x%.4lx\n",
			*val, val - state->memory);
		return VAL_ERR;
	} else {
		return *val;
	}
}

uint16_t check_reg(uint16_t *reg, State *state) {
	if (*reg < MAX_INT || *reg > MAX_INT + 8) {
		fprintf(stderr,
			"synacor: invalid register value %d - 0x%.4lx\n",
			*reg, reg - state->memory);
		return REG_ERR;
	} else {
		return *reg - MAX_INT;
	}
}

void _exit(State *state)
{
	printf("synacor: done, exiting\n");
	free(state);
	exit(0);
}

void _set(State *state)
{
	uint16_t reg;
	uint16_t val;

	if ((reg = check_reg(state->mem_ptr++, state)) == REG_ERR ||
	    (val = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(1);
	}

	if (val >= MAX_INT) {
		state->registers[reg] = state->registers[val - MAX_INT];
	} else {
		state->registers[reg] = val;
	}
}

void _push(State *state)
{
	// TODO check for stack overflow
	uint16_t val;
	if ((val = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(2);
	}
	if (val >= MAX_INT) {
		*state->stack++ = state->registers[val - MAX_INT];
	} else {
		*state->stack++ = val;
	}
}

void _pop(State *state)
{
	uint16_t reg;
	if (state->stack <= state->STACK_LOW_BOUND) {
		fprintf(stderr,
			"synacor: cannot pop from empty stack - 0x%.4lx",
			state->mem_ptr - 1 - state->memory);
		free(state);
		exit(3);
	}
	if ((reg = check_reg(state->mem_ptr++, state)) == REG_ERR) {
		free(state);
		exit(3);
	}
	state->registers[reg] = *state->stack--;
}

void _eq(State *state)
{
	uint16_t out_reg, in_a, in_b, val_a, val_b;
	if ((out_reg = check_reg(state->mem_ptr++, state)) == REG_ERR ||
	    (in_a = check_val(state->mem_ptr++, state)) == VAL_ERR ||
	    (in_b = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(4);
	}
	if (in_a >= MAX_INT) {
		val_a = state->registers[in_a - MAX_INT];
	} else {
		val_a = in_a;
	}
	if (in_b >= MAX_INT) {
		val_b = state->registers[in_b - MAX_INT];
	} else {
		val_b = in_b;
	}
	state->registers[out_reg] = (val_a == val_b) ? 1 : 0;
}

void _jmp(State *state)
{
	uint16_t val;
	if ((val = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(6);
	}

	if (val >= MAX_INT) {
		state->mem_ptr =
			state->memory + state->registers[val - MAX_INT];
	} else {
		state->mem_ptr = state->memory + val;
	}
}

void _jt(State *state)
{
	uint16_t in_val, to_val;
	if ((in_val = check_val(state->mem_ptr++, state)) == VAL_ERR ||
	    (to_val = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(7);
	}
	if (in_val == 0 ||
	    (in_val >= MAX_INT && state->registers[in_val - MAX_INT] == 0)) {
		return;
	}
	if (to_val >= MAX_INT) {
		state->mem_ptr =
			state->memory + state->registers[to_val - MAX_INT];
	} else {
		state->mem_ptr = state->memory + to_val;
	}
}


void _jf(State *state)
{
	uint16_t in_val, to_val;
	if ((in_val = check_val(state->mem_ptr++, state)) == VAL_ERR ||
	    (to_val = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(8);
	}
	if ((in_val != 0 && in_val < MAX_INT) ||
	    (in_val >= MAX_INT && state->registers[in_val - MAX_INT] != 0)) {
		return;
	}
	if (to_val >= MAX_INT) {
		state->mem_ptr =
			state->memory + state->registers[to_val - MAX_INT];
	} else {
		state->mem_ptr = state->memory + to_val;
	}
}

void _add(State *state)
{
	uint16_t out_reg, in_a, in_b, val_a, val_b;
	if ((out_reg = check_reg(state->mem_ptr++, state)) == REG_ERR ||
	    (in_a = check_val(state->mem_ptr++, state)) == VAL_ERR ||
	    (in_b = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(9);
	}
	if (in_a >= MAX_INT) {
		val_a = state->registers[in_a - MAX_INT];
	} else {
		val_a = in_a;
	}
	if (in_b >= MAX_INT) {
		val_b = state->registers[in_b - MAX_INT];
	} else {
		val_b = in_b;
	}
	state->registers[out_reg] = (val_a + val_b) % MAX_INT;
}

void _readChar(State *state)
{
	char c = (char) *state->mem_ptr++;
	printf("%c", c);
}

void _noop(State *state)
{
	uint8_t op = *(state->mem_ptr - 1);
	if (op != 21) {
		printf("%d\n", op);
	}
	return;
}
