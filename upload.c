#include "upload.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define initial_chunk_memory 230
#include <cjson/cJSON.h>

struct memory_binary_data {
    char* content;
    size_t data_size;
    size_t memory_size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
    size_t realsize = size * nmemb;
    struct memory_binary_data *mem = (struct memory_binary_data *)userp;

    if (mem->data_size + realsize + 1 > mem->memory_size){
        char *ptr = realloc(mem->content, mem->data_size + realsize + 1);
        if(ptr == NULL) {
            /* out of memory! */
            printf("not enough memory (realloc returned NULL)\n");
            return 0;
        }
        mem->content = ptr;
    }

    memcpy(&(mem->content[mem->data_size]), contents, realsize);
    mem->data_size += realsize;
    mem->content[mem->data_size] = 0;

    return realsize;
}

char* get_url_from_response(char* response_string){
    cJSON *json = cJSON_ParseWithLength(response_string,initial_chunk_memory);
    cJSON* url_json_str = cJSON_GetObjectItemCaseSensitive(json,"url");
    return cJSON_GetStringValue(url_json_str);
}

char* upload_png_to_server(struct binary_data png_file_need_to_upload) {
    CURL *curl = curl_easy_init();
    curl_mime *mime;
    struct memory_binary_data chunk;
    chunk.content = malloc(initial_chunk_memory);
    chunk.data_size = 0;
    chunk.memory_size = initial_chunk_memory;

    struct curl_mimepart *file_part;
    struct curl_mimepart *scene_text_part;
    struct curl_mimepart *name_text_part;
    /* create a mime handle */
    mime = curl_mime_init(curl);

    scene_text_part = curl_mime_addpart(mime);
    curl_mime_name(scene_text_part,"scene");
    curl_mime_data(scene_text_part, "aeMessageCenterImageRule", CURL_ZERO_TERMINATED);

    name_text_part = curl_mime_addpart(mime);
    curl_mime_name(name_text_part,"name");
    curl_mime_data(name_text_part,"filename.png",CURL_ZERO_TERMINATED);

    /* add a part */
    file_part = curl_mime_addpart(mime);
    /* add data to the part  */
    curl_mime_data(file_part, (char *) png_file_need_to_upload.content, png_file_need_to_upload.size);
    curl_mime_name(file_part, "file");
    curl_mime_filename(file_part, "filename.png");
    curl_mime_type(file_part, "image/png");

    curl_easy_setopt(curl, CURLOPT_URL, "http://kfupload.alibaba.com/mupload");
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36");
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_mime_free(mime);
    free(png_file_need_to_upload.content);

    return get_url_from_response(chunk.content);
}