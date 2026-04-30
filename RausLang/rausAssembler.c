#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_LINES 2048
#define MAX_LINE 256
#define MAX_STACK 64

# define MAX_SYMBOLS 64

char *program[MAX_LINES];
int program_size = 0;


// opcodes
uint8_t get_opcode(const char *op) {
    if (strcmp(op, "ADD") == 0) return 1;
    if (strcmp(op, "SUB") == 0) return 2;
    if (strcmp(op, "EQU") == 0) return 3;
    if (strcmp(op, "LES") == 0) return 4;
    if (strcmp(op, "GRE") == 0) return 5;
    if (strcmp(op, "AND") == 0) return 6;
    if (strcmp(op, "OR_") == 0) return 7;
    if (strcmp(op, "NOT") == 0) return 8;
    if (strcmp(op, "RE_") == 0) return 9;
    if (strcmp(op, "WR_") == 0) return 10;
    if (strcmp(op, "SHO") == 0) return 11;
    return 0;
}


// 25 bit packing
uint32_t pack(uint8_t cond, uint8_t op, uint8_t s1, uint8_t s2, uint8_t s3) {
    return ((cond & 0x1F) << 20) | (op & 0X1F) << 15 | (s1 & 0X1F) << 10 | (s2 & 0X1F) << 10 | (s3 & 0X1F);
}

// Loading code file
void load_file(FILE *f) {
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), f)) {
        program[program_size] = strdup(line);
        program_size++;
    }
}

// condition and nested conditions handling
// create a temp ram location for conditional logic operation
typedef struct {
    char name[32];
    uint8_t addr;
} Symbol;

Symbol table[MAX_SYMBOLS];
int symbol_count = 0;

// reserve 0 for no condition
uint8_t next_free_addr = 1;

//----------------Symbol lookup------------------
uint8_t get_address(const char *name) {
    // check if already exist
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(table[i].name, name) == 0) {
            return table[i].addr;
        }
    }
    // allocate new
    if (next_free_addr > 31) {
        printf("Out of RAM addresses\n");
        exit(1);
    }

    strcpy(table[symbol_count].name, name);
    table[symbol_count].addr = next_free_addr;

    symbol_count++;
    return next_free_addr;
}

uint8_t allocate_temp() {
    if (next_free_addr > 31) {
        printf("Out of RAM for temp\n");
        exit(1);
    }

    return next_free_addr++;
}

uint8_t combine_conditions(uint8_t a, uint8_t b) {
    if (a == 0) return b;
    if (b == 0) return a;

    // allocation of a RAM slot
    uint8_t temp = allocate_temp();

    return temp; // ram location
}

// Block code handling
int find_block_end(int start) {
    int depth = 1;
    int i = start;

    while (i < program_size && depth > 0) {
        char *line = program[i];

        if (strncmp(line, "FOR", 3) == 0 || strncmp(line, "IF", 2) == 0) {
            depth++;
        }
        else if (strncmp(line, "END", 3) == 0) {
            depth--;
        }

        i++;

    }

    return i - 1;
}

// Emission of instructions
void emit(char *line, FILE *out, uint8_t cond) {
    char temp[MAX_LINE];
    strcpy(temp, line);

    char *op = strtok(temp, " \t\n");
    char *a = strtok(temp, " \t\n");
    char *b = strtok(temp, " \t\n");
    char *c = strtok(temp, " \t\n");

    if (!op || !a || !b || !c) return;

    uint32_t instr = pack(cond, get_opcode(op), get_address(a), get_address(b), get_address(c));

    fwrite(&instr, sizeof(instr), 1, out);
}


void process_block(int start, int end, FILE *out, uint8_t *cond_stack, int cond_top) {
    for (int i = start; i < end; i++) {
        char line_copy[MAX_LINE];
        strcpy(line_copy, program[i]);
        char *cmd = strtok(line_copy, " \t\n");
        if (!cmd) continue;

        //-----------FOR---------------
        if (strcmp(cmd, "FOR") == 0) {
            int count = atoi(strtok(NULL, " \t\n"));
            int block_end = find_block_end(i + 1);

            for (int c = 0; c < count; c++) {
                process_block(i + 1, block_end, out, cond_stack, cond_top);
            }

            i = block_end;
            continue;
        }

        //----IF-------
        if  (strcmp(cmd, "IF") == 0) {
            char *cond_var = strtok(NULL, " \t\n");
            uint8_t new_cond = get_address(cond_var);

            if (cond_top >= 0) {
                uint8_t combined = combine_conditions(cond_stack[cond_top], new_cond);

                // emit AND instructions into temp
                uint8_t instr = pack(0, 6, cond_stack[cond_top], new_cond, combined);
                fwrite(&instr, sizeof(instr), 1, out);
                new_cond = combined;
            }

            cond_stack[++cond_top] = new_cond;

            int block_end = find_block_end(i + 1);

            process_block(i + 1, block_end, out, cond_stack, cond_top);
            
            cond_top--;
            i = block_end;
            continue;
        }

        //--------Normal----------
        uint8_t active_cond = (cond_top >= 0) ? cond_stack[cond_top] : 0;
        emit(program[i], out, active_cond);
    }
}


// Processing blocks
void process_blocks(int start, int end, FILE *out, uint8_t *cond_stack, int cond_top);



// MAIN
int main() {
    FILE *in = fopen("input.raus", "r");
    FILE *out = fopen("output.bin", "wb");

    if (!in || !out) {
        printf("File error\n");
        return 1;
    }

    load_file(in);

    uint8_t cond_stack[MAX_STACK];
    int cond_top = -1;

    process_block(0, program_size, out, cond_stack, cond_top);

    fclose(in);
    fclose(out);

    printf("Assembly complete\n");
    return 0;

}



// codes
// write ram_allocation value_to_be_stored
// read ram_location
// add ram_location_val1 ram_location_val2 ram_allocation_results
// sub ram_location_val1 ram_location_val2 ram_allocation_results
// Less ram_location_val1 ram_location_val2 ram_allocation_results
// Greater ram_location_val1 ram_location_val2 ram_allocation_results
// Equality ram_location_val1 ram_location_val2 ram_allocation_results
// And ram_location_val1 ram_location_val2 ram_allocation_results
// Or ram_location_val1 ram_location_val2 ram_allocation_results
// Not ram_location_val1 ram_location_val2 ram_allocation_results
// if ram_location_condition command
// else ram_allocation_condition command
// while ram_allocation command plus parameters
// for 

// What is needed
// Reading a file line by line
// Text Parsing module
// Binary converter