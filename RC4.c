#include "RC4.h" 
#include <string.h>

static void swap(uint8_t* a, uint8_t* b) {
    uint8_t temp = *a;
    *a = *b;
    *b = temp;
}

void rc4_setup(RC4* ctx) {
    uint8_t T[MAX_SIZE];
    int i, j;
    for (i = 0; i < MAX_SIZE; i++) {
        ctx->S[i] = (uint8_t)i;
        T[i] = ctx->key[i % ctx->keylen];
    }
    j = 0;
    for (i = 0; i < MAX_SIZE; i++) {
        j = (j + ctx->S[i] + T[i]) % MAX_SIZE;
        swap(&ctx->S[i], &ctx->S[j]);
    }
    ctx->i = 0;
    ctx->j = 0;
}

void rc4_stream(RC4* ctx, uint8_t* out, int length) {
    int t, index = 0;
    while (index < length) {
        ctx->i = (ctx->i + 1) % MAX_SIZE;
        ctx->j = (ctx->j + ctx->S[ctx->i]) % MAX_SIZE;

        swap(&ctx->S[ctx->i], &ctx->S[ctx->j]);

        t = (ctx->S[ctx->i] + ctx->S[ctx->j]) % MAX_SIZE;
        out[index] = ctx->S[t];
        index++;
    }
}

int hex_char_to_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

int parse_hex_string(const char* hex_str, uint8_t* out, int max_len) {
    int len = strlen(hex_str);
    int count = 0;
    for (int i = 0; i < len - 1 && count < max_len; ) {
        while (i < len && hex_char_to_value(hex_str[i]) == -1) {
            i++;
        }
        if (i >= len - 1) break;

        int val1 = hex_char_to_value(hex_str[i]);
        int val2 = hex_char_to_value(hex_str[i + 1]);

        if (val1 != -1 && val2 != -1) {
            out[count++] = (uint8_t)(val1 * 16 + val2);
            i += 2;
        }
        else {
            i++; 
        }
    }
    return count;
}