attribute vec3 v_position;
attribute vec2 v_texture;
attribute vec3 v_normal;

varying vec2 texture;
varying vec3 normal;
varying vec3 viewPos;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;


void main(void)
{
	gl_Position = mvpMatrix * vec4(v_position, 1.0);
	texture = v_texture;
	normal = (modelMatrix * vec4(v_normal, 0.0)).xyz;
	viewPos = (modelMatrix * vec4(v_position, 1.0)).xyz;
}
