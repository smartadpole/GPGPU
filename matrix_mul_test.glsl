#version 100
#define PRECISION mediump
precision PRECISION float;
#define MAX_VAL 255.0
uniform sampler2D input_image;

// output: nchw
uniform ivec4 output_shape;
varying vec2 tex_coord;
const float maxVal = 255.0;
const float W = 3200.0, H = 2400.0;

#define NEGATIVE(x) (256-(x))
#define NEGATIVEf(x) (256.0-(x))
#define OUTPUT(x) ((x)/maxVal)
#define KEEP_SIGNAL(x) (sign(x) >= 0 ? (x) : NEGATIVE(abs(x)))
#define KEEP_SIGNALf(x) (sign(x) >= 0.0 ? (x) : NEGATIVEf(abs(x)))
#define KEEP_SIGNALf_texture(x) KEEP_SIGNALf((x)*maxVal)

vec2 MoveRight(const vec2 pos)
{
    return vec2(min(1.0, pos.x + 1.0/W), pos.y);
}

void main(){
    // w, h, c
    ivec2 pos = ivec2(tex_coord.x*W, tex_coord.y*H);
    vec4 pixel = vec4(texture2D(input_image, tex_coord).r, 0.0, 0.0, 0.0);
    vec4 pixel2 = vec4(texture2D(input_image, MoveRight(tex_coord)).r, 0.0, 0.0, 0.0);
    // pixel /= float(2);
    // gl_FragColor = OUTPUT(vec4(pos.y, 0.0, 0.0, 128.0));
    gl_FragColor = vec4(KEEP_SIGNALf(OUTPUT(-2.0)*3.0), 0.0, 0.0, 0.0);
}