#pragma once
#include "defs.h"

CLASS(StringView)
{
    char *name;
    u8 length;
};
StringView typedef Token;

VECTOR_TYPE(Token);

STRING DumpFile(const STRING path);

vector_Token ReadTokens(const STRING in);