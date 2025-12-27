#pragma once
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLASS(name)                                                                                \
    typedef struct name name;                                                                      \
    struct name

#define ENUM(name, ...) typedef enum name __VA_ARGS__ name

#define VECTOR_TYPE(type)                                                                          \
    CLASS(vector_##type)                                                                           \
    {                                                                                              \
        type *data;                                                                                \
        size_t size;                                                                               \
        size_t capacity;                                                                           \
    }

#define MAKE_VECTOR(type) (vector_##type){.data = malloc(sizeof(type)), .size = 0, .capacity = 1}

#define PUSH(vec, val)                                                                             \
    {                                                                                              \
        if (++(vec).size > (vec).capacity)                                                         \
        {                                                                                          \
            (vec).capacity *= 2;                                                                   \
            (vec).data = realloc((vec).data, (vec).capacity * sizeof((vec).data[0]));              \
        }                                                                                          \
        (vec).data[(vec).size - 1] = val;                                                          \
    }

#define at(i) data[i]

#define POP(vec)                                                                                   \
    {                                                                                              \
        --(vec).size;                                                                              \
        if ((vec).capacity / 2 > (vec).size)                                                       \
        {                                                                                          \
            (vec).capacity /= 2;                                                                   \
            (vec).data = realloc((vec).data, (vec).capacity * sizeof((vec).data[0]));              \
        }                                                                                          \
    }

#define NEW(type, ...)                                                                             \
    ({                                                                                             \
        type *placeholder = malloc(sizeof(type));                                                  \
        *placeholder = (type)__VA_ARGS__;                                                          \
        placeholder;                                                                               \
    });

typedef char *STRING;

#define copystr(s)                                                                                 \
    ({                                                                                             \
        size_t l = strlen(s) + 1;                                                                  \
        STRING v = malloc(l);                                                                      \
        strcpy_s(v, l, s);                                                                         \
        v;                                                                                         \
    })

#define ullfromstr(s)                                                                              \
    strtoull(s,                                                                                    \
             ({                                                                                    \
                 STRING t = (s) + strlen(s) - 1;                                                   \
                 &t;                                                                               \
             }),                                                                                   \
             10)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
