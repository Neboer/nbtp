#ifndef NBTP_NBTP_H
#define NBTP_NBTP_H

#include <stdlib.h>
#include <pthread.h>
#include <curl/curl.h>

#define big_block_size 0x300000 // 3M per block
#define big_block_line 1024 // 1024 pixels per line

#define smallest_block_size 300
#define smallest_block_line 10

#define max_headers_length 100

struct binary_data {
    unsigned char* content;
    size_t size;
};

struct upload_thread_info {
    size_t current_download_speed;
    int block_id;
    char* upload_result_url;
    CURLcode upload_failed_code;
    pthread_mutex_t* upload_failed_notification_mutex;
    short upload_status;
    pthread_t thread;
};

struct data_with_id {
    struct binary_data data;
    int id;
};

#endif //NBTP_NBTP_H
