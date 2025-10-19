#include "user.h"
#include "RC4.h"

#define MAX_LINE_LEN 512
#define MAX_KEY_LEN 256
#define DEFAULT_KEYSTREAM_LEN 256

//Hàm xử lý nếu người dùng muốn nhập thủ công
void start_interactive_mode() {
    char input_buffer[MAX_LINE_LEN];
    uint8_t user_key[MAX_KEY_LEN];
    char plaintext[MAX_LINE_LEN];
    uint8_t* keystream = NULL;
    uint8_t* ciphertext = NULL;

    while (1) {
        printf("\n---------------------------------------------------\n");
        printf("Nhap tiep Key hoac nhan exit de thoat!\n");
        printf("> ");

        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) break;
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        if (strncmp(input_buffer, "exit", 4) == 0) {
            printf("Thoat.\n");
            break;
        }

        int key_len = strlen(input_buffer); // Độ dài key là độ dài chuỗi

        if (key_len == 0) {
            printf("Key khong the rong. Nhap lai key!\n");
            continue;
        }
        if (key_len > MAX_KEY_LEN) {
            printf("Key qua dai (toi da %d bytes). Nhap lai key.\n", MAX_KEY_LEN);
            continue;
        }

        // Sao chép trực tiếp các byte ký tự vào mảng key
        memcpy(user_key, input_buffer, key_len);

        printf("Nhap PLaintext:\n");
        printf("> ");
        if (!fgets(plaintext, sizeof(plaintext), stdin)) break;
        plaintext[strcspn(plaintext, "\n")] = 0;

        int plaintext_len = strlen(plaintext);

        RC4 ctx;
        ctx.keylen = key_len;
        memcpy(ctx.key, user_key, key_len);

        rc4_setup(&ctx);

        keystream = (uint8_t*)malloc(DEFAULT_KEYSTREAM_LEN);
        if (!keystream) {
            printf("Phan bo bo nho that bai!\n");
            continue;
        }
        rc4_stream(&ctx, keystream, DEFAULT_KEYSTREAM_LEN);

        int encrypt_len = plaintext_len;
        if (plaintext_len > DEFAULT_KEYSTREAM_LEN) {
            encrypt_len = DEFAULT_KEYSTREAM_LEN;
            printf("Warning: Plaintext is longer than %d bytes. Only the first %d bytes will be encrypted.\n",
                DEFAULT_KEYSTREAM_LEN, DEFAULT_KEYSTREAM_LEN);
        }

        ciphertext = (uint8_t*)malloc(encrypt_len);
        if (!ciphertext) {
            printf("Phan bo bo nho that bai!\n");
            free(keystream);
            continue;
        }

        for (int i = 0; i < encrypt_len; i++) {
            ciphertext[i] = (uint8_t)plaintext[i] ^ keystream[i];
        }

        printf("\n--- Ciphertext (%d bytes) ---\n", encrypt_len);
        for (int i = 0; i < encrypt_len; i++) {
            printf("%02X ", ciphertext[i]);
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
        }
        printf("\n");

        free(keystream);
        free(ciphertext);
    }
}