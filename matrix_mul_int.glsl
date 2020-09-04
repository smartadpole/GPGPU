precision mediump float;
uniform sampler2D A;
uniform sampler2D B;
uniform float N;
varying vec2 tex_coord;

void main() {
    vec4 a = texture2D(A, tex_coord);
    vec4 b = texture2D(B, tex_coord);
    // float a = texelFetch(A, ivec2(i, pixel.y), 0).r;
    // float b = texelFetch(B, ivec2(pixel.x, i ), 0).r;
    gl_FragColor = a+b;
}
