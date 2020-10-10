#version 100
#define FORMAT rgba32f
#define PRECISION mediump
precision PRECISION float;
#define LOCAL_SIZE_X 1
#define LOCAL_SIZE_Y 1
#define MAX_TEXTURE_SIZE 32768
//any4_to_any
// (1, nhwc/4, 4)
uniform sampler2D input_image;

// from any to any4
// output shape is equal to input shape
uniform ivec4 output_shape;

#define UP_DIV(x, y) (((x)+(y)-1)/(y))

// (1, (nhwc)/4, 4)


void main(){
    ivec2 pos = ivec2(gl_FragCoord.xy);

    int output_num_elements = output_shape.x * output_shape.y
                    * output_shape.z * output_shape.w;
    output_num_elements = UP_DIV(output_num_elements, 4);
    if(pos.x+pos.y*MAX_TEXTURE_SIZE>=output_num_elements){
        return;
    }

    vec4 res;
    for(int i=0;i<4;++i){
        int output_index = (pos.x+pos.y*MAX_TEXTURE_SIZE)*4+i;
        int index = output_index%output_shape.w;
        int offset = output_index/output_shape.w*UP_DIV(output_shape.w, 4)+index/4;
        int x = offset%MAX_TEXTURE_SIZE;
        int y = offset/MAX_TEXTURE_SIZE;

        res[i] = float(x)/255.0; //texture2D(input_image, vec2(x, y))[index%4];
    }

    gl_FragColor = res;
}
