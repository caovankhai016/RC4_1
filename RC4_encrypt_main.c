#include "read_test.h"
#include "user.h"

int main() {
    int choice = 0;
    char buffer[10];

    while (1) {
        printf("\n=========================\n");
        printf("  RC4 Algorithm \n");
        printf("=========================\n");
        printf("1. Chay Test Vector\n");
        printf("2. Nhap thu cong (User)\n");
        printf("3. Exit\n");
        printf("Please enter your choice: ");

        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        choice = atoi(buffer);

        switch (choice) {
        case 1: {
            int result = run_tests_from_file("Test_vector.txt");
            if (result == 0) {
                printf("\nAll test completed.\n");
            }
            else {
                printf("\nTest failed to run.\n");
            }
            break;
        }
        case 2:
            start_interactive_mode();
            break;
        case 3:
            printf("Goodbye!\n");
            return 0;
        default:
            printf("Lua chon khong hop le. Thu lai.\n");
            break;
        }
        printf("\nNhan Enter de quay lai Menu...");
        getchar(); // Đợi người dùng nhấn Enter
    }

    return 0;
}