#version 100
#define FORMAT rgba32f
#define PRECISION mediump
precision PRECISION float;
#define LOCAL_SIZE_X 1
#define LOCAL_SIZE_Y 1
#define MAX_TEXTURE_SIZE 32768
#define MAX_VAL 255.0
#extension GL_EXT_gpu_shader4:enable
//nchw_to_hwn4c4
// input: (1, (nchw)/4, 4)
uniform sampler2D input_image;

// output: nchw
uniform ivec4 output_shape;
varying vec2 tex_coord;

#define UP_DIV(x, y) (((x)+(y)-1)/(y))

// (h*w* out_4, in_4*in4, out4)
// chw: (out4, h*w* out_4, in_4*in4)
// zyx


void main(){
    // w, h, c
    ivec2 pos = ivec2(tex_coord*MAX_VAL);
    int output_pos_x = pos.x;
    int output_pos_y = pos.y;
    int out_n4_ind = output_pos_y % UP_DIV(output_shape.x, 4);
    output_pos_y =  output_pos_y / UP_DIV(output_shape.x, 4);
    int out_w_ind = output_pos_y%output_shape.w;
    output_pos_y = output_pos_y/output_shape.w;
    int out_h_ind = output_pos_y%output_shape.z;
    int out_c_ind = output_pos_x;


    vec4 res;
    for(int i=0;i<4;++i){
        if(out_n4_ind*4+i>=output_shape.x || out_c_ind>=output_shape.y){
            continue;
        }
        int index = (((out_n4_ind*4+i)*output_shape.y+out_c_ind)*output_shape.z+out_h_ind)*output_shape.w+out_w_ind;
        int offset = index/4;
        int x = offset%MAX_TEXTURE_SIZE;
        int y = offset/MAX_TEXTURE_SIZE;

        res[i] = texture2D(input_image, vec2(x, y))[index%4];
    }

    gl_FragColor = res/MAX_VAL;
    gl_FragColor = vec4(tex_coord.x*MAX_VAL, 0.0, 0.0, 0.0);
    gl_FragColor = texture2D(input_image, gl_FragCoord.xy); 
}
