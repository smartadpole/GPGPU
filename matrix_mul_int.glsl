#version 100
precision highp float;
precision mediump int;
uniform sampler2D A;
uniform sampler2D B;
uniform float zeroPointA;
uniform float scaleA;
uniform float zeroPointB;
uniform float scaleB;
varying vec2 tex_coord;
const float maxVal = 255.0;

#define INPUT(x) ((x) * maxVal)
#define OUTPUT(x) ((x) / maxVal)
#define NEGATIVE(x) (256-(x))
#define NEGATIVEf(x) (256.0-(x))
#define KEEP_SIGNAL(x) (sign(x) >= 0 ? (x) : NEGATIVE(abs(x)))
#define KEEP_SIGNALf(x) (sign(x) >= 0.0 ? (x) : NEGATIVEf(abs(x)))
#define KEEP_SIGNALf_texture(x) KEEP_SIGNALf((x)*maxVal)


vec4 Dequantize(const float zeroPoint, const float scale, vec4 a)
{
    return scale*(INPUT(a)-zeroPoint);
}

vec4 Quantize(const float zeroPoint, const float scale, vec4 a)
{
    return OUTPUT(a/scale + zeroPoint);
}


void main() {
    vec4 a = texture2D(A, tex_coord);
    vec4 b = texture2D(B, tex_coord);
    

    b = Dequantize(zeroPointB, scaleB, b);
    a = Dequantize(zeroPointA, scaleA, a);
    

    vec4 res = a*b;

    res = Quantize(zeroPointA, scaleA, res);
    // gl_FragColor =  a*b*maxVal; 
    gl_FragColor = res;
    // gl_FragColor = mod(a, b); // a%b
}
