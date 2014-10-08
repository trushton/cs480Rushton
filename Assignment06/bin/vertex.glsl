attribute vec3 v_position;
attribute vec2 v_tex;
varying vec2 tex_coords;

uniform mat4 mvpMatrix;
void main(void){
   gl_Position = mvpMatrix * vec4(v_position, 1.0f);
   tex_coords = v_tex;
}
