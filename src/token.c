#include "token.h"

vector_Token ReadTokens(STRING in)
{
    vector_Token tokens = MAKE_VECTOR(Token);
    size_t i = 0;
    Token token;
    while (in[i])
    {
        if (isspace(in[i]))
        {
            ++i;
            continue;
        }

        token.name = &in[i];
        if (isalnum(in[i]))
        {
            token.length = 0;
            do
                ++token.length;
            while (in[++i] && isalnum(in[i]));
        }
        else
        {
            token.length = 1;
            ++i;
        }
        PUSH(tokens, token);
    }
    return tokens;
}

STRING DumpFile(const STRING path)
{
    FILE *in;
    fopen_s(&in, path, "rt");
    fseek(in, 0l, SEEK_END);
    long length = ftell(in);
    fseek(in, 0l, SEEK_SET);
    STRING content = malloc(sizeof(char) * length + 1);
    size_t read_length = fread(content, sizeof(char), length, in);
    content[read_length] = 0;
    fclose(in);
    return content;
}
