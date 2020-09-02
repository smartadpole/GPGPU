#version 200 es
precision mediump int;
uniform isampler2D A;
uniform isampler2D B;
uniform int N;
out int color;
void main() {
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    color = 0;
    for (int i = 0; i < N; i++) {
        int a = texelFetch(A, ivec2(i, pixel.y), 0).r;
        int b = texelFetch(B, ivec2(pixel.x, i ), 0).r;
        color += a * b;
    }
}
