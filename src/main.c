#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

char *get_prog(int argc, char **argv)
{
    char *prog = "main.bf";
    for (int i = 0; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-i"))
        {
            ++i;
            if (argc <= i)
            {
                return prog;
            }
            return argv[i];
        }
    }
    return prog;
}

char *readFile(char *path)
{
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    int sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = malloc(sz + 1);
    buffer[sz] = 0;
    fread(buffer, sz, 1, fp);
    return buffer;
}

enum
{
    PLUS,
    MINUS,
    LOOP_OPEN,
    LOOP_CLOSE,
    PRINT,
    READ,
    RIGHT,
    LEFT,
    COMMENT,
};

typedef struct
{
    int type;
    int rep;
} Instruction;

Instruction lex(char *src, int pos)
{
    switch (src[pos])
    {
    case '+':
    {
        ++pos;
        int c = 1;
        while (src[pos] == '+')
        {
            ++c;
            ++pos;
        }
        return (Instruction){PLUS, c};
    }
    case '-':
    {
        ++pos;
        int c = 1;
        while (src[pos] == '-')
        {
            ++c;
            ++pos;
        }
        return (Instruction){MINUS, c};
    }
    case '>':
    {
        ++pos;
        int c = 1;
        while (src[pos] == '>')
        {
            ++c;
            ++pos;
        }
        return (Instruction){RIGHT, c};
    }
    case '<':
    {
        ++pos;
        int c = 1;
        while (src[pos] == '<')
        {
            ++c;
            ++pos;
        }
        return (Instruction){LEFT, c};
    }
    case '[':
    {
        return (Instruction){LOOP_OPEN, 1};
    }
    case ']':
    {
        return (Instruction){LOOP_CLOSE, 1};
    }
    case '.':
    {
        return (Instruction){PRINT, 1};
    }
    case ',':
    {
        return (Instruction){READ, 1};
    }
    default:
    {
        return (Instruction){COMMENT, 1};
    }
    }
}
Instruction *lex_all(char *src, int *count)
{
    int len = strlen(src);
    int pos = 0;
    Instruction *instructions = malloc(sizeof(Instruction));
    int sz = 0;
    while (len > pos)
    {
        Instruction ins = lex(src, pos);
        if (ins.type == COMMENT)
        {
            ++pos;
        }
        else
        {
            pos += ins.rep;
            instructions = realloc(instructions, sz * sizeof(Instruction) + sizeof(Instruction));
            instructions[sz] = ins;
            ++sz;
        }
    }
    *count = sz;
    return instructions;
}

void print_ins(Instruction i)
{
    printf("INS: %d, %d\n", i.type, i.rep);
}

typedef struct
{
    uint8_t *ops;
    uint8_t *args;
    int n_op;
    int n_arg;
} executable_ins;

#define INT_TO_U8_ARRAY(val, arr)          \
    do                                     \
    {                                      \
        (arr)[0] = (uint8_t)((val) >> 0);  \
        (arr)[1] = (uint8_t)((val) >> 8);  \
        (arr)[2] = (uint8_t)((val) >> 16); \
        (arr)[3] = (uint8_t)((val) >> 24); \
    } while (0)

executable_ins compile(Instruction ins)
{
    switch (ins.type)
    {
    case PLUS:
    {
        uint8_t *ops = malloc(2);
        uint8_t *args = malloc(1);
        ops[0] = 0x80;
        ops[1] = 0x03;
        args[0] = (uint8_t)ins.rep;
        return (executable_ins){ops, args, 2, 1};
    }
    case MINUS:
    {
        uint8_t *ops = malloc(2);
        uint8_t *args = malloc(1);
        ops[0] = 0x80;
        ops[1] = 0x2b;
        args[0] = (uint8_t)ins.rep;
        return (executable_ins){ops, args, 2, 1};
    }
    case LEFT:
    {
        uint8_t *arr = malloc(1);
        arr[0] = (uint8_t)ins.rep;
        uint8_t *ops = malloc(3);
        ops[0] = 0x48;
        ops[1] = 0x81;
        ops[2] = 0xEB;
        return (executable_ins){ops, arr, 3, 1};
    }
    case RIGHT:
    {
        uint8_t *arr = malloc(1);
        arr[0] = (uint8_t)ins.rep;
        uint8_t *ops = malloc(3);
        ops[0] = 0x48;
        ops[1] = 0x83;
        ops[2] = 0xc3;
        return (executable_ins){ops, arr, 3, 1};
    }
    case PRINT:
    {
        uint8_t *ops = malloc(20);
        int i = 0;
        ops[i++] = 0xb8;
        ops[i++] = 0x01;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0xbf;
        ops[i++] = 0x01;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x48;
        ops[i++] = 0x89;
        ops[i++] = 0xDe;
        ops[i++] = 0xBA;
        ops[i++] = 0x01;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x0F;
        ops[i++] = 0x05;
        return (executable_ins){ops, 0, i, 0};
    }
    case READ:
    {
        uint8_t *ops = malloc(20);
        int i = 0;
        ops[i++] = 0xb8;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0xBF;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x48;
        ops[i++] = 0x89;
        ops[i++] = 0xDE;
        ops[i++] = 0xBA;
        ops[i++] = 0x01;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x00;
        ops[i++] = 0x0F;
        ops[i++] = 0x05;
        return (executable_ins){ops, 0, i, 0};
    }
    }
    return (executable_ins){0, 0, 0, 0};
}

executable_ins *compile_all_basic(Instruction *instructions, int count)
{

    executable_ins *insts = malloc(sizeof(executable_ins) * count);
    for (int i = 0; i < count; ++i)
    {
        insts[i] = compile(instructions[i]);
    }
    return insts;
}

typedef struct
{
    int scope;
    int lpos;
    int pos;
    int ins_pos;
} loop_t;

void resolve_loops(executable_ins *exec_instructions, Instruction *instructions, int count)
{
    int pos = 0;
    int scope = 0;
    int loop_pos = 0;
    loop_t *loops;
    int nloop = 0;
    for (int i = 0; i < count; ++i)
    {
        if (instructions[i].type == LOOP_OPEN)
        {
            loop_t l = (loop_t){scope, loop_pos, pos, i};
            if (nloop == 0)
            {
                loops = malloc(sizeof(loop_t));
            }
            ++nloop;
            loops = realloc(loops, sizeof(loop_t) * nloop);
            loops[nloop - 1] = l;
            scope++;
            pos += 9;
        }
        else if (instructions[i].type == LOOP_CLOSE)
        {
            --scope;
            if (scope == 0)
            {
                loop_pos++;
            }
            pos += 5;
        }
        else
        {
            pos += exec_instructions[i].n_op + exec_instructions[i].n_arg;
        }
    }
    scope = 0;
    loop_pos = 0;
}

void print_exec(executable_ins ins)
{
    printf("exec_ins:\nops:\n");
    printf("%d\n", ins.n_op);
    for (int i = 0; i < ins.n_op; ++i)
    {
        printf("%x\n", ins.ops[i]);
    }
    printf("args:\n");
    for (int i = 0; i < ins.n_arg; ++i)
    {
        printf("%x\n", ins.args[i]);
    }
}

int main(int argc, char **argv)
{
    char *program = get_prog(argc, argv);
    printf("Interpreting: %s\n", program);
    char *file = readFile(program);
    printf("File:\n```\n%s\n```\n", file);
    int count;
    Instruction *instructions = lex_all(file, &count);
    for (int i = 0; i < count; ++i)
    {
        print_ins(instructions[i]);
    }
    executable_ins *insts = compile_all_basic(instructions, count);
    resolve_loops(insts, instructions, count);
    for (int i = 0; i < count; ++i)
    {
        print_exec(insts[i]);
    }
    uint8_t *buffer;
    int sz = 0;
    for (int i = 0; i < count; ++i)
    {
        sz += insts[i].n_op + insts[i].n_arg;
    }
    buffer = calloc(1, sz + 1);
    int idx = 0;
    for (int i = 0; i < count; ++i)
    {
        for (int j = 0; j < insts[i].n_op; ++j)
        {
            buffer[idx] = insts[i].ops[j];
            ++idx;
        }
        for (int j = 0; j < insts[i].n_arg; ++j)
        {
            buffer[idx] = insts[i].args[j];
            ++idx;
        }
        if (insts[i].args)
        {
            free(insts[i].args);
        }
        if (insts[i].ops)
        {
            free(insts[i].ops);
        }
    }
    buffer[idx++] = 0xC3;
    for (int i = 0; i < idx; ++i)
    {
        printf("%d: %x\n", i, buffer[i]);
    }
    FILE *f = fopen("out.bin", "wb");
    fwrite(buffer, idx, 1, f);
    fclose(f);
    void *executable_memory = mmap(NULL, idx, PROT_READ | PROT_WRITE | PROT_EXEC,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (executable_memory == MAP_FAILED)
    {
        perror("Cannot execute, memory map failed\n");
    }
    printf("Executable memory is mapped\n");
    for (int i = 0; i < idx; ++i)
    {
        ((uint8_t *)executable_memory)[i] = buffer[i];
    }
    uint8_t *memory = malloc(1024 * 8);
    memory += 1024 * 4;

    __asm__ __volatile__(
        "mov %[input], %%rbx"
        :
        : [input] "r"(memory)
        : "rbx");
    __asm__ __volatile__(
        "call *%0"
        :
        : "r"(executable_memory)
        : "memory");
}