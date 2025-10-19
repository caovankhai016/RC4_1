
#include "RC4.h"
#include "read_test.h"

static long parse_dec_offset(const char* line) {
    long offset = 0;
    int i = 0;
    // Bỏ qua phần "DEC "
    while (line[i] != '\0' && (line[i] < '0' || line[i] > '9')) {
        i++;
    }
    // Đọc các chữ số
    while (line[i] >= '0' && line[i] <= '9') {
        offset = offset * 10 + (line[i] - '0');
        i++;
    }
    return offset;
}

static void run_single_test(const uint8_t* key, int keylen, FILE* file) {
    char line[MAX_LINE_LEN];
    RC4 ctx;
    uint8_t generated_stream[TEST_VECTOR_BLOCK_SIZE];
    uint8_t expected_stream[TEST_VECTOR_BLOCK_SIZE];
    long current_pos = 0;
    int tests_passed = 0;
    int tests_failed = 0;
    uint8_t dummy_byte; // Dùng để bỏ qua byte

    printf("\n--- Chay Test ---\n");
    printf("Key (%d bytes): ", keylen);
    for (int i = 0; i < keylen; i++) printf("%02x", key[i]);
    printf("\n--------------------\n");

    ctx.keylen = keylen;
    memcpy(ctx.key, key, keylen);

    rc4_setup(&ctx);

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "Key length:", 11) == 0 || strlen(line) < 2) {
            fseek(file, -(long)strlen(line), SEEK_CUR);
            break;
        }

        if (strncmp(line, "DEC", 3) == 0) {
            long offset = parse_dec_offset(line);

            // Tìm vị trí bắt đầu của chuỗi hex (sau dấu ':')
            char* hex_part = NULL;
            for (int i = 0; line[i] != '\0'; i++) {
                if (line[i] == ':') {
                    hex_part = &line[i + 1];
                    break;
                }
            }
            if (!hex_part) continue;

            parse_hex_string(hex_part, expected_stream, TEST_VECTOR_BLOCK_SIZE);

            for (long i = current_pos; i < offset; i++) {
                rc4_stream(&ctx, &dummy_byte, 1);
            }

            // Tạo 16 byte keystream để kiểm tra
            rc4_stream(&ctx, generated_stream, TEST_VECTOR_BLOCK_SIZE);
            current_pos = offset + TEST_VECTOR_BLOCK_SIZE;

            int mismatch = 0;
            for (int i = 0; i < TEST_VECTOR_BLOCK_SIZE; i++) {
                if (generated_stream[i] != expected_stream[i]) {
                    mismatch = 1;
                    break;
                }
            }

            if (mismatch == 0) {
                printf("  [PASS] Offset %ld\n", offset);
                tests_passed++;
            }
            else {
                printf("  [FAIL] Offset %ld\n", offset);
                tests_failed++;
            }
        }
    }
    printf("Result: %d passed, %d failed.\n", tests_passed, tests_failed);
}


int run_tests_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening test file");
        return 1;
    }

    printf("Successfully opened test file: %s\n", filename);

    char line[MAX_LINE_LEN];
    uint8_t key[MAX_KEY_LEN];
    int keylen = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "key:", 4) == 0) {
            // Tìm vị trí "0x"
            char* key_hex_start = NULL;
            for (int i = 0; line[i + 1] != '\0'; i++) {
                if (line[i] == '0' && (line[i + 1] == 'x' || line[i + 1] == 'X')) {
                    key_hex_start = &line[i + 2];
                    break;
                }
            }

            if (key_hex_start) {
                keylen = parse_hex_string(key_hex_start, key, MAX_KEY_LEN);
                run_single_test(key, keylen, file);
            }
        }
    }

    fclose(file);
    return 0;
}