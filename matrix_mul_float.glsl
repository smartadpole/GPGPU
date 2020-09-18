precision mediump float;
uniform sampler2D A;
varying vec2 tex_coord;
uniform float a;
const float maxVal = 255.0;

void main() {
  vec4 color = texture2D(A, tex_coord);
  gl_FragColor = color * a;
};