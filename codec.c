#include "codec.h"
#include <math.h>

// input_original_data.content must come from malloc() function. After this operation the input_original_data's content is still valid.
struct binary_data encode_to_png(struct binary_data input_original_data){
    struct binary_data result = {};
    if (input_original_data.size == big_block_size){// validate 3M big size file
        lodepng_encode_memory(&(result.content),&(result.size),input_original_data.content,big_block_line,big_block_line,LCT_RGB,8);
    } else {// is the last small size data
        unsigned char* preprocess_convertable_pixel_image_data;
        int side_length;
        if (input_original_data.size < smallest_block_size){
            preprocess_convertable_pixel_image_data = realloc(input_original_data.content, smallest_block_size);
            side_length = smallest_block_line;
        } else{
            side_length = ceil(sqrt((float )input_original_data.size / 3.0));
            preprocess_convertable_pixel_image_data = realloc(input_original_data.content,side_length*side_length*3);
        }
        lodepng_encode_memory(&(result.content), &(result.size), preprocess_convertable_pixel_image_data, side_length, side_length, LCT_RGB, 8);
        free(preprocess_convertable_pixel_image_data);
    }
    return result;
}