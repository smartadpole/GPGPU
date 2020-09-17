precision mediump float;
uniform sampler2D img;
varying vec2 tex_coord;
uniform float a;

void main() {
  vec4 color = texture2D(img, tex_coord);
  gl_FragColor = color * a;
};