#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#define MAX_INT 32768

uint8_t read_source(uint16_t *memory);
void run(uint16_t *memory, uint16_t registers[]);
size_t _exit(uint16_t *loc);
size_t _readChar(uint16_t *loc);
size_t _noop(uint16_t *loc);

size_t (*instructions[22])();


int main(int argc, char *argv[])
{
	uint16_t memory[MAX_INT];
	uint16_t registers[8];
	registers[0] = 0;

	instructions[0] = _exit;
	instructions[19] = _readChar;
	instructions[21] = _noop;

	read_source(memory);

	run(memory, registers);

	return 0;
}

uint8_t read_source(uint16_t *memory) 
{
	FILE *input;
	uint16_t buff;
	
	if ((input = fopen("challenge.bin", "r")) == NULL ||
		input == NULL) {
		fprintf(stderr, "comp: Error opening challenge.bin");
		exit(-1);
	}
	while (fread(&buff, sizeof(uint16_t), 1, input) > 0) {
		*(memory++) = buff;
	}
	fclose(input);
	
	return 0;
}

void run(uint16_t *memory, uint16_t registers[])
{
	uint16_t op;
	uint16_t *loc = memory;
	while (1) {
		op = *loc++;
		loc += (*instructions[op])(loc);
	}
}

size_t _exit(uint16_t *loc)
{
	exit(0);
}

size_t _readChar(uint16_t *loc)
{
	char c = (char) *loc;
	printf("%c", c);
	return 1;
}

size_t _noop(uint16_t *loc)
{
	return 0;
}
