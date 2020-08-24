#ifndef NBTP_NBTP_H
#define NBTP_NBTP_H

#include <stdlib.h>

#define big_block_size 0x300000 // 3M per block
#define big_block_line 1024 // 1024 pixels per line

#define smallest_block_size 300
#define smallest_block_line 10

#define max_headers_length 100

struct binary_data {
    unsigned char* content;
    size_t size;
};


#endif //NBTP_NBTP_H
