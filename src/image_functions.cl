__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

// Nearest neighbour interpolation
__kernel void nn_resize(__read_only image2d_t read,__write_only image2d_t write,
                           unsigned int old_width, unsigned int old_height,
                           unsigned int new_width, unsigned int new_height
){
    const int x_ = get_global_id(0);
    const int y_ = get_global_id(1);

    if (x_ >= new_width || y_ >= new_height) {
        return;
    }

    float scalar_width  = (float)new_width / old_width;
    float scalar_height = (float)new_height / old_height;

    float2 nn_xy = (float2)(x_ / scalar_width, y_ / scalar_height);
    int2 inn_xy = convert_int2(nn_xy);

    // Read the pixel value from the source image
    float4 color = read_imagef(read, sampler, inn_xy);

    // Write the pixel value to the destination image
    write_imagef(write, (int2)(x_, y_), color);
}