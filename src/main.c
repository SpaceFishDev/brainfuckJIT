#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
}