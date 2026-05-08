#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_LINES 2048
#define MAX_LINE 256
#define MAX_STACK 64
#define MAX_SYMBOLS 64

char *program[MAX_LINES];
int program_size = 0;

//====================================================
// SYMBOL TABLE
//====================================================

typedef struct {
    char name[32];
    uint8_t addr;
} Symbol;

Symbol table[MAX_SYMBOLS];
int symbol_count = 0;

// Reserve 0 for "no condition"
uint8_t next_free_addr = 1;

//====================================================
// OPCODES
//====================================================

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

//====================================================
// 25-BIT PACKING
// cond [24:20]
// op   [19:15]
// s1   [14:10]
// s2   [9:5]
// s3   [4:0]
//====================================================

uint32_t pack(uint8_t cond,
              uint8_t op,
              uint8_t s1,
              uint8_t s2,
              uint8_t s3)
{
    return ((cond & 0x1F) << 20) |
           ((op   & 0x1F) << 15) |
           ((s1   & 0x1F) << 10) |
           ((s2   & 0x1F) << 5)  |
           (s3    & 0x1F);
}

//====================================================
// FILE LOADING
//====================================================

void load_file(FILE *f) {
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), f)) {

        if (program_size >= MAX_LINES) {
            printf("Program too large\n");
            exit(1);
        }

        program[program_size] = strdup(line);

        if (!program[program_size]) {
            printf("Memory allocation failed\n");
            exit(1);
        }

        program_size++;
    }
}

//====================================================
// SYMBOL LOOKUP
//====================================================

uint8_t get_address(const char *name) {

    // Check existing symbols
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(table[i].name, name) == 0) {
            return table[i].addr;
        }
    }

    // Check limit
    if (symbol_count >= MAX_SYMBOLS) {
        printf("Symbol table full\n");
        exit(1);
    }

    if (next_free_addr > 31) {
        printf("Out of RAM addresses\n");
        exit(1);
    }

    // Allocate new address
    strcpy(table[symbol_count].name, name);
    table[symbol_count].addr = next_free_addr++;

    symbol_count++;

    return table[symbol_count - 1].addr;
}

//====================================================
// TEMPORARY CONDITION STORAGE
//====================================================

uint8_t allocate_temp() {

    if (next_free_addr > 31) {
        printf("Out of RAM for temp variables\n");
        exit(1);
    }

    return next_free_addr++;
}

uint8_t combine_conditions(uint8_t a, uint8_t b) {

    if (a == 0) return b;
    if (b == 0) return a;

    uint8_t temp = allocate_temp();

    return temp;
}

//====================================================
// BLOCK FINDER
//====================================================

int find_block_end(int start) {

    int depth = 1;
    int i = start;

    while (i < program_size && depth > 0) {

        char *line = program[i];

        if (strncmp(line, "FOR", 3) == 0 ||
            strncmp(line, "IF", 2) == 0)
        {
            depth++;
        }
        else if (strncmp(line, "END", 3) == 0)
        {
            depth--;
        }

        i++;
    }

    return i - 1;
}

//====================================================
// INSTRUCTION EMISSION
//====================================================

void emit(char *line, FILE *out, uint8_t cond) {

    char temp[MAX_LINE];
    strcpy(temp, line);

    char *op = strtok(temp, " \t\n");
    char *a  = strtok(NULL, " \t\n");
    char *b  = strtok(NULL, " \t\n");
    char *c  = strtok(NULL, " \t\n");

    if (!op || !a || !b || !c)
        return;

    uint8_t addr_a = get_address(a);
    uint8_t addr_b = get_address(b);
    uint8_t addr_c = get_address(c);

    // Debug output
    printf("%s %s %s %s -> %u %u %u\n",
           op, a, b, c,
           addr_a, addr_b, addr_c);

    uint32_t instr =
        pack(cond,
             get_opcode(op),
             addr_a,
             addr_b,
             addr_c);

    fwrite(&instr, sizeof(instr), 1, out);
}

//====================================================
// BLOCK PROCESSOR
//====================================================

void process_block(int start,
                   int end,
                   FILE *out,
                   uint8_t *cond_stack,
                   int cond_top)
{
    for (int i = start; i < end; i++) {

        char line_copy[MAX_LINE];
        strcpy(line_copy, program[i]);

        char *cmd = strtok(line_copy, " \t\n");

        if (!cmd)
            continue;

        //-----------------------------------------
        // FOR LOOP
        //-----------------------------------------

        if (strcmp(cmd, "FOR") == 0) {

            char *count_str =
                strtok(NULL, " \t\n");

            int count =
                count_str ? atoi(count_str) : 0;

            int block_end =
                find_block_end(i + 1);

            for (int c = 0;
                 c < count;
                 c++)
            {
                process_block(
                    i + 1,
                    block_end,
                    out,
                    cond_stack,
                    cond_top
                );
            }

            i = block_end;
            continue;
        }

        //-----------------------------------------
        // IF CONDITION
        //-----------------------------------------

        if (strcmp(cmd, "IF") == 0) {

            char *cond_var =
                strtok(NULL, " \t\n");

            if (!cond_var) {
                printf("IF missing condition\n");
                exit(1);
            }

            uint8_t new_cond =
                get_address(cond_var);

            // Nested IF
            if (cond_top >= 0) {

                uint8_t combined =
                    combine_conditions(
                        cond_stack[cond_top],
                        new_cond
                    );

                // Emit AND operation
                uint32_t instr =
                    pack(
                        0,
                        6,
                        cond_stack[cond_top],
                        new_cond,
                        combined
                    );

                fwrite(
                    &instr,
                    sizeof(instr),
                    1,
                    out
                );

                new_cond = combined;
            }

            cond_stack[++cond_top] =
                new_cond;

            int block_end =
                find_block_end(i + 1);

            process_block(
                i + 1,
                block_end,
                out,
                cond_stack,
                cond_top
            );

            cond_top--;

            i = block_end;
            continue;
        }

        //-----------------------------------------
        // NORMAL INSTRUCTION
        //-----------------------------------------

        uint8_t active_cond =
            (cond_top >= 0)
            ? cond_stack[cond_top]
            : 0;

        emit(
            program[i],
            out,
            active_cond
        );
    }
}

//====================================================
// MAIN
//====================================================

int main() {

    FILE *in =
        fopen("input.raus", "r");

    FILE *out =
        fopen("output.bin", "wb");

    if (!in || !out) {
        printf("File error\n");
        return 1;
    }

    load_file(in);

    uint8_t cond_stack[MAX_STACK];
    int cond_top = -1;

    process_block(
        0,
        program_size,
        out,
        cond_stack,
        cond_top
    );

    fclose(in);
    fclose(out);

    // Free allocated memory
    for (int i = 0;
         i < program_size;
         i++)
    {
        free(program[i]);
    }

    printf("Assembly complete\n");

    return 0;
}