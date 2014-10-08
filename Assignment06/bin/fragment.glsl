varying vec2 tex_coords;
uniform sampler2D gSampler;

void main(void){
   gl_FragColor = texture2D(gSampler, tex_coords.xy);
}
