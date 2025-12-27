#include <stddef.h>
#include "defs.h"
#include "token.h"

ENUM(AsmArgType,
     {
         ARG_IMM,
         ARG_LAB,
         ARG_REG,
         ARG_MEM,
     }  //
);

ENUM(AsmArgProf,
     {
         NOA = 0,
         ABS = 1,
         REG = 2,
         MEM = 3,
         REL = 4,
         IMM = 5,

         BYT = 1 << 5,
         WOR = 2 << 5,
         SZV = 3 << 5,

     }  //
);

CLASS(AsmOpcode)
{
    const STRING name;
    const u8 code;
    AsmArgProf prof[2];
};

CLASS(AsmRegister)
{
    const STRING name;
    const u8 code;
    AsmArgProf size;
};

const AsmOpcode INSTRUCTION_SET[] = {
    {"add", 0x00, {REG | MEM | BYT, REG | BYT}},
    {"add", 0x02, {REG | MEM | WOR, REG | WOR}},

    {"mov", 0x88, {REG | MEM | BYT, REG | BYT}},
    {"mov", 0x8A, {REG | MEM | WOR, REG | WOR}},
    {"mov", 0x8B, {REG | WOR, REG | MEM | WOR}},

    {"xor", 0x20, {REG | MEM | WOR, REG | WOR} /*?? i made this opcode up, the mnem is real*/},

    {"ret", 0xC3, {NOA}},
};
const u16 NUM_INSTRUCTIONS = sizeof(INSTRUCTION_SET) / sizeof(INSTRUCTION_SET[0]);

const AsmRegister REGISTERS[] = {
    {"ax", 0x00, WOR},
    {"cx", 0x01, WOR},
    {"dx", 0x02, WOR},
    {"bx", 0x03, WOR},

    {"sp", 0x04, WOR},
    {"bp", 0x05, WOR},
    {"si", 0x06, WOR},
    {"di", 0x07, WOR},

};
const u8 NUM_REGISTERS = sizeof(REGISTERS) / sizeof(REGISTERS[0]);

const AsmOpcode *FindInstructionNameOnly(const StringView name)
{
    for (u8 i = 0; i != NUM_INSTRUCTIONS; ++i)
        if (!memcmp(INSTRUCTION_SET[i].name, name.name, name.length))
            return &INSTRUCTION_SET[i];
    return NULL;
}
size_t FindRegisterIndex(const StringView name)
{
    for (u8 i = 0; i != NUM_REGISTERS; ++i)
        if (!memcmp(REGISTERS[i].name, name.name, name.length))
            return i;
    return (size_t)-1;
}
const AsmRegister *FindRegister(const StringView name)
{
    const size_t idx = FindRegisterIndex(name);

    return idx == (size_t)-1 ? NULL : &REGISTERS[idx];
}

CLASS(AsmArg)
{
    AsmArgType type;
    u8 indirection;
    char operation;
    AsmArg *operand;
    u64 value;
};
VECTOR_TYPE(AsmArg);

ENUM(AsmInstrucType,
     {
         ASM_LABEL,  // label
         ASM_INSTR,  // instruction
         ASM_DIREC,  // directive
     }  //
);

CLASS(AsmInstruc)
{
    AsmInstrucType type;
    StringView name;
    vector_AsmArg args;
    u8 bitsize_estimate;
};

CLASS(AsmLabel)
{
    StringView name;
    u64 offset;
};

VECTOR_TYPE(AsmInstruc);
VECTOR_TYPE(AsmLabel);

VECTOR_TYPE(u8);

CLASS(AsmUnit)
{
    vector_AsmInstruc instructions;
    vector_AsmLabel labels;

    u8 working_bitsize;  //

    bool has_shrinkables;

    vector_u8 bytes;
};

size_t FindLabelIndex(AsmUnit *unit, StringView s)
{
    for (size_t i = 0; i != unit->labels.size; ++i)
    {
        if (unit->labels.at(i).name.length != s.length)
            continue;
        if (!memcmp(unit->labels.at(i).name.name, s.name, s.length))
            return i;
    }
    return (size_t)-1;
}

AsmLabel *FindLabel(AsmUnit *unit, StringView s)
{
    const size_t out = FindLabelIndex(unit, s);
    if (out != (size_t)-1)
        return &unit->labels.at(out);
    else
        return NULL;
}

vector_AsmArg ParseArgs(AsmUnit *unit, const vector_Token tokens, size_t *index)
{
    size_t i = *index;
    vector_AsmArg args = MAKE_VECTOR(AsmArg);

    while (i < tokens.size)
    {
        AsmArg arg;
        if (tokens.at(i).name[0] == '[')
        {
            arg.indirection = 1;
        }
        else
        {
            arg.indirection = 0;
            if (isalpha(tokens.at(i).name[0]))
            {
                // check if it is an instruction / label declaration
                if (FindInstructionNameOnly(tokens.at(i)) || tokens.at(i + 1).name[0] == ':')
                    break;

                const size_t reg = FindRegisterIndex(tokens.at(i));
                if (reg != (size_t)-1)
                {
                    arg.type = ARG_REG;
                    arg.value = reg;
                }
                else
                {
                    arg.type = ARG_LAB;
                    arg.value = FindLabelIndex(unit, tokens.at(i));
                }
            }
            else if (isdigit(tokens.at(i).name[0]))
            {
                char *end_of_num = tokens.at(i).name + tokens.at(i).length - 1;
                // assume decimal for now. TODO: add hexadecimal / binary
                arg.value = strtoull(tokens.at(i).name, &end_of_num, 10);

                arg.type = ARG_IMM;
            }
            else if (tokens.at(i).name[0] == '$')
            {
                arg.type = ARG_MEM;  //
            }
            else
            {
                printf("Unknown type '%*.s'\n", tokens.at(i).length, tokens.at(i).name);
            }
            ++i;
        }
        PUSH(args, arg);

        if (tokens.at(i).name[0] != ',')
            break;
        else
            ++i;
    }
    *index = i;
    return args;
}

void ParseInstructions(AsmUnit *unit, const vector_Token tokens)
{
    size_t i = 0;
    while (i < tokens.size)
    {
        AsmInstruc instruc;

        if (i + 1 < tokens.size && tokens.at(i + 1).name[0] == ':')
        {
            instruc.type = ASM_LABEL;
            instruc.name = tokens.at(i);

            i += 2;
        }
        else if (tokens.at(i).name[0] == '.')
        {
            instruc.type = ASM_DIREC;
            //
        }
        else
        {
            instruc.type = ASM_INSTR;
            instruc.name = tokens.at(i);

            ++i;
            instruc.args = ParseArgs(unit, tokens, &i);
        }
        PUSH(unit->instructions, instruc);
    }
}

void PutLabels(AsmUnit *unit, const vector_Token tokens)
{
    for (size_t i = 1; i != tokens.size; ++i)
    {
        if (tokens.at(i).name[0] == ':')
        {
            AsmLabel label;
            label.name = tokens.at(i - 1);

            PUSH(unit->labels, label);
        }
    }
}

bool SizeMatchProf(AsmArgProf prof, u64 v)
{
    if (v < 0xFF && prof & BYT)
        return true;
    if ((v < 0xFFFF) && (prof & WOR || prof & SZV))
        return true;

    return false;
}

const AsmOpcode *FindInstruction(AsmInstruc *ins, AsmUnit *unit)
{
    for (size_t i = 0; i < NUM_INSTRUCTIONS; ++i)
    {
        const AsmOpcode *op = &INSTRUCTION_SET[i];

        u8 olen = strlen(op->name);

        if (ins->name.length != olen)
            continue;

        if (!memcmp(ins->name.name, op->name, olen))
        {
            // todo: finish comparing args

            for (u8 j = 0; j < ins->args.size; ++j)
            {
                AsmArg *arg = &ins->args.at(j);
                switch (arg->type)
                {
                case ARG_IMM:
                    if (!(op->prof[j] & IMM) && !SizeMatchProf(op->prof[j], arg->value))
                        goto no_match;
                    break;
                case ARG_LAB:
                {
                    AsmLabel *lb = &unit->labels.at(arg->value);
                    if (!(op->prof[j] & IMM) && !SizeMatchProf(op->prof[j], lb->offset))
                        goto no_match;
                }
                break;
                case ARG_MEM:
                {
                    // todo
                }
                break;
                case ARG_REG:
                    if (!(op->prof[j] & REG) || !(REGISTERS[arg->value].size & op->prof[j]))
                        goto no_match;
                    break;
                }
            }

            printf("It's a match! Returning instruction profile for opcode %02hhX\n", op->code);

            return &INSTRUCTION_SET[i];
        }
    }
no_match:
    printf("It's not a match! Returning null\n");

    return NULL;
}

void EncodeInstruction(AsmUnit *unit, size_t index)
{
    bool has_mod = false;
    u8 mod_rm = 0;
    u8 opcode;
    u16 disp[2] = {0};

    AsmInstruc *ins = &unit->instructions.at(index);

    const AsmOpcode *op = FindInstruction(&unit->instructions.at(index), unit);

    if (!op)
    {
        printf("Failed to find instruction profile for '%*.s'\n", ins->name.length, ins->name.name);
        exit(EXIT_FAILURE);
    }

    opcode = op->code;
    printf("Opcode %02hhx\n", opcode);
}

void EncodeBytes(AsmUnit *unit)
{
    unit->has_shrinkables = false;

    for (size_t i = 0; i < unit->instructions.size; ++i)
    {
        AsmInstruc *ins = &unit->instructions.at(i);
        switch (ins->type)
        {
        case ASM_LABEL:
        {
            AsmLabel *label = FindLabel(unit, ins->name);
            if (label->offset != unit->bytes.size)
            {
                unit->has_shrinkables = true;
                label->offset = unit->bytes.size;
            }
            printf("Label placed at %016llx\n", label->offset);
        }
        break;
        case ASM_INSTR:
            EncodeInstruction(unit, i);
            break;
        case ASM_DIREC:
            break;
        }
    }
    // unit->bytes.size = 0;
    // unit->has_shrinkables = true;
}

// void EstimateLabels(AsmUnit *unit)
// {
//     size_t count = 0;
//     for (size_t i = 0; i != unit->instructions.size; ++i)
//     {
//         switch (unit->instructions.at(i).type)
//         {
//         case ASM_INSTR:

//             EncodeInstruction(unit, i, true);

//             count += unit->instructions.at(i).bitsize_estimate;
//             break;

//         case ASM_LABEL:

//         case ASM_DIREC:
//             break;
//         }
//     }
//     //
// }

int main(void)
{
    printf("\n");

    STRING content = DumpFile("../tests/test.asm");

    vector_Token tokens = ReadTokens(content);
    AsmUnit unit = {
        .instructions = MAKE_VECTOR(AsmInstruc),
        .labels = MAKE_VECTOR(AsmLabel),
        .has_shrinkables = true,
        .bytes = MAKE_VECTOR(u8),
    };

    PutLabels(&unit, tokens);

    for (size_t i = 0; i != unit.labels.size; ++i)
        printf("label '%.*s'\n", unit.labels.at(i).name.length, unit.labels.at(i).name.name);

    ParseInstructions(&unit, tokens);

    for (size_t i = 0; i < unit.instructions.size; ++i)
    {
        if (unit.instructions.at(i).type == ASM_INSTR)
        {
            printf("Instruction: %.*s, %llu args\n",
                   unit.instructions.at(i).name.length,
                   unit.instructions.at(i).name.name,
                   unit.instructions.at(i).args.size);
            for (size_t j = 0; j < unit.instructions.at(i).args.size; ++j)
            {
                printf("\targ type ");

                switch (unit.instructions.at(i).args.at(j).type)
                {
                case ARG_IMM:
                    printf("immediate: ");
                    break;
                case ARG_LAB:
                    printf("label: ");
                    break;
                case ARG_MEM:
                    printf("memory: ");
                    break;
                case ARG_REG:
                    printf("register: ");
                    break;
                }
                printf("%llu\n", unit.instructions.at(i).args.at(j).value);
            }
        }
    }

    // EstimateLabels(&unit);
    // no need to estimate labels. we just need to start parsing, and at each label we find, we
    // check if its value matches up with the current byte count. if it doesnt, we set the flag for
    // a recount which means we will do another cycle after the current one

    while (unit.has_shrinkables)
    {
        printf("\n\n\nBeginning unit encoding pass..\n\n\n");
        unit.bytes.size = 0;
        EncodeBytes(&unit);
    }

    return 0;
}
