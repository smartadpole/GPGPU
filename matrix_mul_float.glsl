#version 200 es
precision mediump float;
uniform sampler2D A;
uniform sampler2D B;
uniform int N;
out float color;
void main() {
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    color = 0.0;
    for (int i = 0; i < N; i++) {
        float a = texelFetch(A, ivec2(i, pixel.y), 0).r;
        float b = texelFetch(B, ivec2(pixel.x, i ), 0).r;
        color += a * b;
    }
}
