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

vec4 Dequantize(vec4 a)
{
    return scaleA*(a-zeroPointA);;
}

vec4 Quantize(vec4 a)
{
    return a/scaleA + zeroPointA;
}


void main() {
    vec4 a = texture2D(A, tex_coord);
    vec4 b = texture2D(B, tex_coord);
    a = INPUT(a);
    b = INPUT(b);
    a = Dequantize(a);
    b = Dequantize(b);

    vec4 res = a*b;

    res = Quantize(res);
    res = OUTPUT(res);
    // gl_FragColor =  a*b*maxVal; 
    gl_FragColor = a;
    // gl_FragColor = mod(a, b); // a%b
}
