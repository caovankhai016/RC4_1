#pragma once
#include <stdio.h>
#include "RC4.h"
#define MAX_LINE_LEN 512
#define MAX_KEY_LEN 256
#define TEST_VECTOR_BLOCK_SIZE 16

int run_tests_from_file(const char* filename);