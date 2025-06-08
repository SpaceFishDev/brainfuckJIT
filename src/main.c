#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

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
        uint8_t *ops = malloc(4);
        uint8_t *args = malloc(1);
        ops[0] = 0x80;
        ops[1] = 0x04;
        ops[2] = 0x24;
        args[0] = (uint8_t)ins.rep;
        return (executable_ins){ops, args, 3, 1};
    }
    case MINUS:
    {
        uint8_t *ops = malloc(4);
        uint8_t *args = malloc(1);
        ops[0] = 0x80;
        ops[1] = 0x2c;
        ops[2] = 0x24;
        args[0] = (uint8_t)ins.rep;
        return (executable_ins){ops, args, 3, 1};
    }
    case LEFT:
    {
        uint8_t *arr = malloc(4);
        INT_TO_U8_ARRAY(ins.rep, arr);
        uint8_t *ops = malloc(3);
        ops[0] = 0x48;
        ops[1] = 0x81;
        ops[2] = 0xC4;
        return (executable_ins){ops, arr, 3, 4};
    }
    case RIGHT:
    {
        uint8_t *arr = malloc(4);
        INT_TO_U8_ARRAY(ins.rep, arr);
        uint8_t *ops = malloc(3);
        ops[0] = 0x48;
        ops[1] = 0x81;
        ops[2] = 0xEC;
        return (executable_ins){ops, arr, 3, 4};
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
        ops[i++] = 0xE6;
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
        ops[i++] = 0xE6;
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
            pos += 10;
        }
        else if (instructions[i].type == LOOP_CLOSE)
        {
            uint8_t *close_ops = malloc(5);
            int n = 0;
            close_ops[n++] = 0xE9;
            uint8_t *close_args = close_ops + sizeof(uint8_t);

            --scope;
            loop_t l;
            for (int j = 0; j < nloop; ++j)
            {
                if (loops[j].lpos == loop_pos && loops[j].scope == scope)
                {
                    l = loops[j];
                }
            }
            int p = pos - l.pos;
            p = -p;
            printf("P: %d\n", p);
            INT_TO_U8_ARRAY(p, close_args);
            exec_instructions[i] = (executable_ins){close_ops, 0, 5, 0};
            if (scope == 0)
            {
                loop_pos++;
            }
            pos += 5;
            n = 0;
            uint8_t *ops = malloc(10);
            ops[n++] = 0x80;
            ops[n++] = 0x3C;
            ops[n++] = 0x24;
            ops[n++] = 0x00;
            ops[n++] = 0x0F;
            ops[n++] = 0x84;
            int ip = 0;
            ip = (l.pos + 10) - pos;
            ip = -ip;
            uint8_t *arg = ops + 6;
            INT_TO_U8_ARRAY(ip, arg);
            exec_instructions[l.ins_pos] = (executable_ins){ops, 0, 10, 0};
        }
        else
        {
            pos += exec_instructions[i].n_op + exec_instructions[i].n_arg;
        }
    }
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
        // print_exec(insts[i]);
    }
    FILE *out = fopen("out.bin", "wb");
    for (int i = 0; i < count; ++i)
    {
        fwrite(insts[i].ops, 1, insts[i].n_op, out);
        fwrite(insts[i].args, 1, insts[i].n_arg, out);
    }
    fclose(out);
}