// Nearest neighbour interpolation algo
void nearest_neighbour_interpolation(__constant unsigned char* read,unsigned char* write,
                           __constant unsigned int old_width, __constant unsigned int old_height,
                           __constant unsigned int new_width,unsigned int new_height
){
    if((old_width == new_width) && (new_height == old_height)){
        for(int i = 0;i<old_width;i++){
            for(int j = 0; j< old_height;j++){
                write[i][j]=read[i][j];
            }
        }
        return;
    }
    float scalar_width =  new_width/old_width;
    float scalar_height = new_height/old_height;
    for(int i=0;i<new_width;i++){
        for(int j=0;j<new_height;j++){
            write[i][j] = read[round(i/scalar_width)][round(j/scalar_height)];
        }
    }
    return;
}


// Kernel to resize a image
__kernel void resize_image(__constant unsigned char* read,unsigned char* write,
                           __constant unsigned int old_width, __constant unsigned int old_height,
                           __constant unsigned int new_width,unsigned int new_height,
                           __constant unsigned int method
){
    if(method == 0){
        nearest_neighbour_interpolation(read,write,old_width,old_height,new_width,new_height);
        return;
    // TODO Other resize algorithms
    }else{
        return;
    }
}