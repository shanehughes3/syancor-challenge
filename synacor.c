#define _GNU_SOURCE
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
	uint16_t stack[MAX_INT];
	uint16_t *stack_ptr;
} State;

State *initialize();
uint16_t read_source(State *state);
void run(State *state);

void _exit(State *state);
void _set(State *state);
void _push(State *state);
void _pop(State *state);
void _eq(State *state);
void _gt(State *state);
void _jmp(State *state);
void _jt(State *state);
void _jf(State *state);
void _add(State *state);
void _mult(State *state);
void _mod(State *state);
void _and(State *state);
void _or(State *state);
void _not(State *state);
void _rmem(State *state);
void _wmem(State *state);
void _call(State *state);
void _ret(State *state);
void _out(State *state);
void _in(State *state);
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
	state->instructions[5] = _gt;
	state->instructions[6] = _jmp;
	state->instructions[7] = _jt;
	state->instructions[8] = _jf;
	state->instructions[9] = _add;
	state->instructions[10] = _mult;
	state->instructions[11] = _mod;
	state->instructions[12] = _and;
	state->instructions[13] = _or;
	state->instructions[14] = _not;
	state->instructions[15] = _rmem;
	state->instructions[16] = _wmem;
	state->instructions[17] = _call;
	state->instructions[18] = _ret;
	state->instructions[19] = _out;
	state->instructions[20] = _in;
	state->instructions[21] = _noop;
	state->mem_ptr = state->memory;
	for (int i = 0; i < 8; ++i) {
		state->registers[i] = 0;
	}
	state->stack_ptr = state->stack;
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
		/* if (op != 19) { */
		/* 	printf("op %d\n", op); */
		/* } */
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
	uint16_t val;
	if (state->stack_ptr > state->stack + MAX_INT) {
		fprintf(stderr,
			"synacor: stack overflow - 0x%.4lx\n",
			state->mem_ptr - 1 - state->memory);
		free(state);
		exit(2);
	}
	if ((val = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(2);
	}
	if (val >= MAX_INT) {
		*state->stack_ptr++ = state->registers[val - MAX_INT];
	} else {
		*state->stack_ptr++ = val;
	}
}

void _pop(State *state)
{
	uint16_t reg;
	if (state->stack_ptr < state->stack) {
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
	state->registers[reg] = *--state->stack_ptr;
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

void _gt(State *state)
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
	state->registers[out_reg] = (val_a > val_b) ? 1 : 0;
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

void _mult(State *state)
{
	uint16_t out_reg, in_a, in_b, val_a, val_b;
	if ((out_reg = check_reg(state->mem_ptr++, state)) == REG_ERR ||
	    (in_a = check_val(state->mem_ptr++, state)) == VAL_ERR ||
	    (in_b = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(10);
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
	state->registers[out_reg] = (val_a * val_b) % MAX_INT;
}

void _mod(State *state)
{
	uint16_t out_reg, in_a, in_b, val_a, val_b;
	if ((out_reg = check_reg(state->mem_ptr++, state)) == REG_ERR ||
	    (in_a = check_val(state->mem_ptr++, state)) == VAL_ERR ||
	    (in_b = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(11);
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
	state->registers[out_reg] = val_a % val_b;
}

void _and(State *state)
{
	uint16_t out_reg, in_a, in_b, val_a, val_b;
	if ((out_reg = check_reg(state->mem_ptr++, state)) == REG_ERR ||
	    (in_a = check_val(state->mem_ptr++, state)) == VAL_ERR ||
	    (in_b = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(12);
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
	state->registers[out_reg] = val_a & val_b;
}

void _or(State *state)
{
	uint16_t out_reg, in_a, in_b, val_a, val_b;
	if ((out_reg = check_reg(state->mem_ptr++, state)) == REG_ERR ||
	    (in_a = check_val(state->mem_ptr++, state)) == VAL_ERR ||
	    (in_b = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(13);
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
	state->registers[out_reg] = val_a | val_b;
}

void _not(State *state) {
	uint16_t out_reg, val;
	if ((out_reg = check_reg(state->mem_ptr++, state)) == REG_ERR ||
	    (val = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(14);
	}
	// have to mask off most significant bit per specs
	if (val >= MAX_INT) {
		state->registers[out_reg] =
			~state->registers[val - MAX_INT] & 0x7fff;
	} else {
		state->registers[out_reg] = ~val & 0x7fff; 
	}
}

void _rmem(State *state)
{
	uint16_t out_reg, in_addr, val;
	if ((out_reg = check_reg(state->mem_ptr++, state)) == REG_ERR ||
	    (in_addr = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(15);
	}
	if (in_addr >= MAX_INT) {
		val = state->memory[state->registers[in_addr - MAX_INT]];
	} else {
		val = state->memory[in_addr];
	}
	state->registers[out_reg] = val;
}

void _wmem(State *state)
{
	uint16_t addr, input_val, val;
	if ((addr = check_val(state->mem_ptr++, state)) == VAL_ERR ||
	    (input_val = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(16);
	}
	if (input_val >= MAX_INT) {
		val = state->registers[input_val - MAX_INT];
	} else {
		val = input_val;
	}
	if (addr >= MAX_INT) {
		state->memory[state->registers[addr - MAX_INT]] = val;
	} else {
		state->memory[addr] = val;
	}
}

void _call(State *state)
{
	uint16_t to_val;
	if ((to_val = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(17);
	}
	*state->stack_ptr++ = state->mem_ptr - state->memory;
	if (to_val >= MAX_INT) {
		state->mem_ptr =
			state->memory + state->registers[to_val - MAX_INT];
	} else {
		state->mem_ptr = state->memory + to_val;
	}
}

void _ret(State *state) {
	if (state->stack_ptr < state->stack) {
		fprintf(stderr,
			"synacor: cannot ret from empty stack - 0x%.4lx",
			state->mem_ptr - 1 - state->memory);
		free(state);
		exit(18);
	}
	state->mem_ptr = state->memory + *--state->stack_ptr;
}

void _out(State *state)
{
	uint16_t in, val;
	if ((in = check_val(state->mem_ptr++, state)) == VAL_ERR) {
		free(state);
		exit(19);
	}
	if (in >= MAX_INT) {
		val = state->registers[in - MAX_INT];
	} else {
		val = in;
	}
	printf("%c", val);
}

void _in(State *state)
{
	static size_t len;
	static char *buffer = NULL;
	static int is_set = 0;
	static char *buffer_ptr;
	uint16_t out_reg;
	if ((out_reg = check_reg(state->mem_ptr++, state)) == REG_ERR) {
		free(state);
		exit(20);
	}

	if (!is_set || *buffer_ptr == '\0') {
		printf("> ");
		getline(&buffer, &len, stdin);
		buffer_ptr = buffer;
		is_set = 1;
	}
	state->registers[out_reg] = *buffer_ptr++;
}

void _noop(State *state)
{
	uint8_t op = *(state->mem_ptr - 1);
	if (op != 21) {
		printf("%d\n", op);
	}
	return;
}
