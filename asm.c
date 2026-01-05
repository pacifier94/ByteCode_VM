#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_LINES   4096
#define MAX_LABELS  512
#define MAX_LINE    256

typedef struct {
    char name[64];
    uint32_t addr;
} Label;

typedef struct {
    int line_no;
    char text[MAX_LINE];
} Instruction;

typedef struct {
    const char* name;
    uint8_t opcode;
} Opcode;

Opcode OPCODES[] = {
    {"PUSH",  0x01}, {"POP",  0x02}, {"DUP",  0x03}, {"HALT", 0xFF},
    {"ADD",   0x10}, {"SUB",  0x11}, {"MUL",  0x12}, {"DIV",  0x13}, {"CMP", 0x14},
    {"JMP",   0x20}, {"JZ",   0x21}, {"JNZ",  0x22},
    {"STORE", 0x30}, {"LOAD", 0x31},
    {"CALL",  0x40}, {"RET",  0x41}
};

uint8_t find_opcode(const char* name, int* found) {
    for (size_t i = 0; i < sizeof(OPCODES)/sizeof(OPCODES[0]); i++) {
        if (strcmp(name, OPCODES[i].name) == 0) {
            *found = 1;
            return OPCODES[i].opcode;
        }
    }
    *found = 0;
    return 0;
}

int find_label(Label* labels, int count, const char* name, uint32_t* out) {
    for (int i = 0; i < count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            *out = labels[i].addr;
            return 1;
        }
    }
    return 0;
}

void write_be32(FILE* f, int32_t v) {
    fputc((v >> 24) & 0xFF, f);
    fputc((v >> 16) & 0xFF, f);
    fputc((v >> 8) & 0xFF, f);
    fputc(v & 0xFF, f);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: asm input.asm [output.bin]\n");
        return 1;
    }

    const char* input = argv[1];
    const char* output = (argc > 2) ? argv[2] : "program.bin";

    FILE* in = fopen(input, "r");
    if (!in) {
        perror("Input file");
        return 1;
    }

    Label labels[MAX_LABELS];
    Instruction instr[MAX_LINES];
    int label_count = 0, instr_count = 0;
    uint32_t pc = 0;

    char line[MAX_LINE];

    /* ---------- PASS 1: LABELS ---------- */
    int line_no = 0;
    while (fgets(line, sizeof(line), in)) {
        line_no++;

        char* comment = strchr(line, ';');
        if (comment) *comment = '\0';

        char* s = strtok(line, "\r\n");
        if (!s || !*s) continue;

        size_t len = strlen(s);
        if (s[len - 1] == ':') {
            s[len - 1] = '\0';
            strcpy(labels[label_count].name, s);
            labels[label_count].addr = pc;
            label_count++;
        } else {
            instr[instr_count].line_no = line_no;
            strcpy(instr[instr_count].text, s);
            instr_count++;

            pc += 1; // opcode
            if (strchr(s, ' ')) pc += 4; // argument
        }
    }

    rewind(in);

    /* ---------- PASS 2: BYTECODE ---------- */
    FILE* out = fopen(output, "wb");
    if (!out) {
        perror("Output file");
        fclose(in);
        return 1;
    }

    for (int i = 0; i < instr_count; i++) {
        char buf[MAX_LINE];
        strcpy(buf, instr[i].text);

        char* mnemonic = strtok(buf, " ");
        char* arg = strtok(NULL, " ,");

        int found;
        uint8_t opcode = find_opcode(mnemonic, &found);
        if (!found) {
            printf("Error: Unknown instruction '%s' at line %d\n",
                   mnemonic, instr[i].line_no);
            return 1;
        }

        fputc(opcode, out);

        if (arg) {
            uint32_t val;
            if (find_label(labels, label_count, arg, &val)) {
                write_be32(out, (int32_t)val);
            } else {
                write_be32(out, atoi(arg));
            }
        }
    }

    fclose(in);
    fclose(out);

    printf("Assembled successfully. Labels found: ");
    for (int i = 0; i < label_count; i++) {
        printf("%s ", labels[i].name);
    }
    printf("\n");

    return 0;
}
