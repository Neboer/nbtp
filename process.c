#include "nbtp.h"
#include <stdlib.h>
#include <string.h>
#include "codec.h"

#define upload_successful 2
#define upload_error 1
#define upload_forwarding 0
#define upload_uncreated -1
#define upload_fatal -2

void create_an_upload_process(struct binary_data input_original_block_data, struct upload_thread_info* report_destination){

}

short start_upload_progress_poll(short thread_counts,
                              struct data_with_id (*generate_next_data)(void),
                              void (*after_success_upload)(int, char*),
                              void (*after_failed_upload)(int, int),
                              void (*after_fatal_upload)(int),
                              struct upload_thread_info* threadInfoList){
    for (int i = 0; i < thread_counts; ++i) threadInfoList[i].upload_status = upload_uncreated;
    short loop_index = 0;
    while (1){
        loop_index = (loop_index + (short)1)%thread_counts;
        struct upload_thread_info current = threadInfoList[loop_index];
        // when a upload_status is set to upload_successful, the result URL must be accessible.
        if (current.upload_status == upload_successful||current.upload_status == upload_uncreated){
            // a new block of task will be taken and transferred.
            if (current.upload_status == upload_successful) after_success_upload(current.block_id, current.upload_result_url);
            // take new block and insert into current thread position
            struct data_with_id next_block_data = generate_next_data();
            create_an_upload_process(next_block_data.data, threadInfoList + loop_index);
            // cleanup. the result of current thread is get and current thread is useless.
            // notice that it is thread's job to cleanup useless encoded PNG data.
            pthread_detach(current.thread);
        } else if (current.upload_status == upload_error){
            // When an error occurred, the thread will report the error and lock its upload_failed_notification_mutex.
            // This thread poll manager will check the error code and ensure it can be properly processed.
            // Then the thread manager unlock the mutex to allow the upload worker subprocess to upload again.
            after_failed_upload(current.block_id,current.upload_failed_code);
            pthread_mutex_unlock(current.upload_failed_notification_mutex);
        } else if (current.upload_status == upload_fatal){
            // deadly error or failed many times.
            after_fatal_upload(current.block_id);
            return -1;
        }
    }
}