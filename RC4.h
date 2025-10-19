#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#define MAX_SIZE 256

typedef struct {
    uint8_t S[MAX_SIZE];
    uint8_t key[MAX_SIZE];
    int keylen;
    uint8_t i; // Biến trạng thái i
    uint8_t j; // Biến trạng thái j
} RC4;

// Hàm khởi tạo và hoán vị S-box (KSA)
void rc4_setup(RC4* ctx);
// Hàm tạo dòng keystream (PRGA)
void rc4_stream(RC4* ctx, uint8_t* out, int length);
int hex_char_to_value(char c);
int parse_hex_string(const char* hex_str, uint8_t* out, int max_len);