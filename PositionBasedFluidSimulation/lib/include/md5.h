#pragma once

#include<stdint.h>


// digest[16] = md5(initial_msg[initial_len])
void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest);