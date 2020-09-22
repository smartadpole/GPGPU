#version 100
#extension GL_EXT_gpu_shader4 : enable
precision highp float;
precision mediump int;
uniform sampler2D A;
uniform sampler2D B;
uniform float N;
varying vec2 tex_coord;
const float maxVal = 255.f;

void main() {
    vec4 a = texture2D(A, tex_coord);
    vec4 b = texture2D(B, tex_coord);
    // gl_FragColor =  a*b*maxVal; 
    // gl_FragColor = vec4(ivec4(a*maxVal)%ivec4(b*maxVal))/maxVal; // a%b
    gl_FragColor = mod(a, b); // a%b
}
