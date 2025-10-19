#include "RC4.h"
#include "read_test.h"
#include <ctype.h>


static long parse_dec_offset(const char* line) {
    long offset = 0;
    int i = 0;
    // Bỏ qua phần "DEC "
    while (line[i] != '\0' && !isdigit((unsigned char)line[i])) {
        i++;
    }
    // Đọc các chữ số
    while (isdigit((unsigned char)line[i])) {
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
    long current_pos_in_stream = 0;
    int tests_passed = 0;
    int tests_failed = 0;
    uint8_t dummy_byte;
    long current_file_pos = ftell(file); // Lưu vị trí hiện tại trong file

    printf("\n--- Chay Test ---\n");
    printf("Key (%d bytes): ", keylen);
    for (int i = 0; i < keylen; i++) printf("%02x", key[i]);
    printf("\n--------------------\n");

    ctx.keylen = keylen;
    memcpy(ctx.key, key, keylen);

    rc4_setup(&ctx); // Khởi tạo RC4 state cho key

    while (fgets(line, sizeof(line), file)) {
        current_file_pos = ftell(file); // Cập nhật vị trí sau khi đọc

        //loại bỏ khoảng trắng thừa
        char* start = line;
        while (isspace((unsigned char)*start)) start++; // Bỏ qua khoảng trắng đầu dòng
        char* end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end)) end--; // Bỏ qua khoảng trắng cuối dòng
        *(end + 1) = '\0'; // Kết thúc chuỗi tại vị trí ký tự không phải khoảng trắng cuối cùng

        //Kiểm tra xem có phải dòng bắt đầu test case mới hoặc dòng ngăn cách không
        if (strncmp(start, "key:", 4) == 0 || strncmp(start, "// ===", 6) == 0 || strlen(start) == 0) {
            fseek(file, current_file_pos - strlen(line), SEEK_SET); // Quay lại đầu dòng vừa đọc
            break; // Thoát vòng lặp đọc DEC
        }

        //Nếu là dòng DEC, xử lý nó
        if (strncmp(start, "DEC", 3) == 0) {
            long offset = parse_dec_offset(start);

            // Tìm vị trí bắt đầu của chuỗi hex (sau dấu ':')
            char* hex_part = strchr(start, ':');
            if (!hex_part) continue; // Bỏ qua nếu dòng DEC không đúng định dạng
            hex_part++; // Di chuyển con trỏ qua dấu ':'

            // Parse chuỗi hex mong đợi
            int parsed_len = parse_hex_string(hex_part, expected_stream, TEST_VECTOR_BLOCK_SIZE);
            if (parsed_len != TEST_VECTOR_BLOCK_SIZE) {
                printf("  Offset %ld: Expected %d bytes, parsed %d bytes from hex.\n", offset, TEST_VECTOR_BLOCK_SIZE, parsed_len);
            }


            if (offset > current_pos_in_stream) {
                for (long i = current_pos_in_stream; i < offset; i++) {
                    rc4_stream(&ctx, &dummy_byte, 1);
                }
            }
            else if (offset < current_pos_in_stream) {
                printf("  Offset %ld out of order (current stream pos %ld). Resetting RC4 state.\n", offset, current_pos_in_stream);
                rc4_setup(&ctx); // Reset state
                current_pos_in_stream = 0;
                for (long i = 0; i < offset; i++) {
                    rc4_stream(&ctx, &dummy_byte, 1);
                }
            }
            // Nếu offset == current_pos_in_stream thì không cần làm gì

           // Tạo 16 byte keystream để kiểm tra
            rc4_stream(&ctx, generated_stream, TEST_VECTOR_BLOCK_SIZE);
            current_pos_in_stream = offset + TEST_VECTOR_BLOCK_SIZE; // Cập nhật vị trí trong stream

            // So sánh
            int mismatch = memcmp(generated_stream, expected_stream, TEST_VECTOR_BLOCK_SIZE);

            if (mismatch == 0) {
                printf("  [PASS] Offset %ld\n", offset);
                tests_passed++;
            }
            else {
                printf("  [FAIL] Offset %ld\n", offset);
                // In ra chi tiết lỗi để debug (tùy chọn)
                printf("    Expected: "); for (int k = 0; k < TEST_VECTOR_BLOCK_SIZE; k++) printf("%02x ", expected_stream[k]); printf("\n");
                printf("    Generated:"); for (int k = 0; k < TEST_VECTOR_BLOCK_SIZE; k++) printf("%02x ", generated_stream[k]); printf("\n");
                tests_failed++;
            }
        }
        // Nếu không phải dòng DEC, key, // hoặc dòng trống, thì bỏ qua và đọc dòng tiếp
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
    int test_case_count = 0;

    while (fgets(line, sizeof(line), file)) {
        //Loại bỏ khoảng trắng thừa
        char* start = line;
        while (isspace((unsigned char)*start)) start++;
        char* end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end)) end--;
        *(end + 1) = '\0';

        // --- Tìm dòng key ---
        if (strncmp(start, "key:", 4) == 0) {
            test_case_count++;
            // Tìm vị trí "0x"
            char* key_hex_start = strstr(start, "0x"); // Tìm "0x"
            if (!key_hex_start) key_hex_start = strstr(start, "0X"); // Thử tìm "0X"

            if (key_hex_start) {
                key_hex_start += 2; // Bỏ qua "0x"
                keylen = parse_hex_string(key_hex_start, key, MAX_KEY_LEN);
                if (keylen > 0) {
                    run_single_test(key, keylen, file); // Truyền con trỏ file
                }
                else {
                    printf("Error parsing key on line: %s\n", line);
                }
            }
            else {
                printf("Could not find '0x' in key line: %s\n", line);
            }
        }
    }

    fclose(file);

    if (test_case_count == 0) {
        printf("\nWarning: No test cases found in the file (no lines starting with 'key:').\n");
        return 1; // Coi như lỗi nếu không tìm thấy test case nào
    }

    return 0; // Trả về 0 nếu chạy xong (dù pass hay fail)
}
