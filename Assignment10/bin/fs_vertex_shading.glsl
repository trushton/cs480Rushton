
varying vec2 texture;
varying vec4 color;


uniform sampler2D gSampler;                                                                 
                                                                   
                                                                                         
void main()                                                                                 
{                                                                                                                                                                           
    gl_FragColor = texture2D(gSampler, texture.xy) * color ;                             
}