#include <lodepng.h>
#include <stdio.h>
#include <math.h>
#include "nbtp.h"
#include "upload.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>

// before the last block call this
struct binary_data get_next_good_png_block(FILE* file){
    unsigned char* this_block = malloc(big_block_size);
    struct binary_data result = {};
    fread(this_block,big_block_size,1,file);
    lodepng_encode_memory(&(result.content),&(result.size),this_block,big_block_line,big_block_line,LCT_RGB,8);
    free(this_block);
    return result;
}

struct binary_data get_last_png_block(FILE* file, size_t last_data_size){
    unsigned char* this_block;
    int side_length;
    if (last_data_size < smallest_block_size){
        this_block = malloc(smallest_block_size);
        side_length = smallest_block_line;
        fread(this_block,last_data_size,1,file);
    } else{
        side_length = ceil(sqrt((float )last_data_size / 3.0));
        this_block = malloc(side_length * side_length * 3);
        fread(this_block,last_data_size,1,file);
    }
    struct binary_data result = {};
    lodepng_encode_memory(&(result.content),&(result.size),this_block,side_length,side_length,LCT_RGB,8);
    free(this_block);
    return result;
}

struct file_block_info {
    int total_block_count;
    size_t final_block_size;
    size_t file_size;
};

struct file_block_info get_file_block_info(FILE* file){
    struct file_block_info result ;
    fseek(file, 0 , SEEK_END);
    result.file_size = ftell(file);
    fseek(file, 0 , SEEK_SET);
    result.total_block_count = (int) ceilf((float) result.file_size/(float) big_block_size);
    result.final_block_size = result.file_size - (result.total_block_count - 1) * big_block_size;
    return result;
}

void write_headers_to_file(char* filename, struct file_block_info file_info, FILE* file){
    cJSON * headers_json = cJSON_CreateObject();
    cJSON_AddItemToObject(headers_json,"file_name",cJSON_CreateString(filename));
    cJSON_AddItemToObject(headers_json,"file_length",cJSON_CreateNumber(file_info.file_size));
    cJSON_AddItemToObject(headers_json,"blocks_count",cJSON_CreateNumber(file_info.total_block_count));
    cJSON_AddItemToObject(headers_json,"last_block_size",cJSON_CreateNumber(file_info.final_block_size));
    fputs(cJSON_Print(headers_json),file);
    fflush(file);
}

int main(int argc, char** argv){
    curl_global_init(CURL_GLOBAL_ALL);
    struct file_block_info file_info;
    if (strcmp(argv[1],"upload") == 0){
        char* filename = argv[2];
        FILE* upload_file = fopen(filename,"rb");
        char record_file_name[strlen(filename) + 5];
        strcpy(record_file_name,filename);
        strcat(record_file_name,".nbtp");
        FILE* record_file = fopen(record_file_name,"w");
        file_info = get_file_block_info(upload_file);
        write_headers_to_file(filename,file_info,record_file);
        fputc('\n',record_file);
        for (int current_block = 1; current_block <= file_info.total_block_count - 1; ++current_block) {
            struct binary_data looping_png_data = get_next_good_png_block(upload_file);
            char* url = upload_png_to_server(looping_png_data);
            fputs(url,record_file);
            fputc('\n',record_file);
        }
        struct binary_data last_png_data = get_last_png_block(upload_file,file_info.final_block_size);
        char* url = upload_png_to_server(last_png_data);
        fputs(url,record_file);
        fputc('\n',record_file);
    }
}