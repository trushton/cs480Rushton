const int MAX_POINT_LIGHTS = 2;

attribute vec3 v_position;
attribute vec2 v_texture;
attribute vec3 v_normal;

varying vec4 color;
varying vec2 texture;
vec3 viewPos;


struct BaseLight                                                                    
{                                                                                   
    vec3 Color;                                                                     
    float AmbientIntensity;                                                         
    float DiffuseIntensity;                                                         
};                                                                                  
                                                                                    
struct DirectionalLight                                                             
{                                                                                   
    BaseLight Base;                                                                 
    vec3 Direction;                                                                 
};                                                                                  
                                                                                    
struct Attenuation                                                                  
{                                                                                   
    float Constant;                                                                 
    float Linear;                                                                   
    float Exp;                                                                      
};                                                                                  
                                                                                    
struct PointLight                                                                           
{                                                                                           
    BaseLight Base;                                                                         
    vec3 Position;                                                                          
    Attenuation Atten;                                                                      
};  

struct SpotLight                                                                            
{                                                                                           
    PointLight Base;                                                                        
    vec3 Direction;                                                                         
    float Cutoff;                                                                           
};


uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;                                               
uniform DirectionalLight gDirectionalLight;                                                 
uniform PointLight gPointLights; 
uniform SpotLight gSpotLights;                                         
uniform vec3 cameraPos;                                                                  
uniform float gMatSpecularIntensity;                                                        
uniform float gSpecularPower;                                                            
          

vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)                   
{                                                                                           
    vec4 AmbientColor = vec4(Light.Color, 1.0) * Light.AmbientIntensity;                   
    float DiffuseFactor = dot(Normal, -LightDirection);                                     
                                                                                            
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
    if (DiffuseFactor > 0.0) {                                                                
        DiffuseColor = vec4(Light.Color, 1.0) * Light.DiffuseIntensity * DiffuseFactor;    
                                                                                            
        vec3 VertexToEye = normalize(cameraPos - viewPos);                             
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));                     
        float SpecularFactor = dot(VertexToEye, LightReflect);                              
        SpecularFactor = pow(SpecularFactor, gSpecularPower);                               
        if (SpecularFactor > 0.0) {                                                           
            SpecularColor = vec4(Light.Color, 1.0) * gMatSpecularIntensity * SpecularFactor;                         
        }                                                                                   
    }                                                                                       
                                                                                            
    return (AmbientColor + DiffuseColor + SpecularColor);                                   
}                                                                                           
                                                                                            
vec4 CalcDirectionalLight(vec3 Normal)                                                      
{                                                                                           
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal); 
}                                                                                              

vec4 CalcPointLight(PointLight l, vec3 Normal)                                              
{                                                                                           
    vec3 LightDirection = viewPos - l.Position;                                           
    float Distance = length(LightDirection);                                                
    LightDirection = normalize(LightDirection);                                             
                                                                                            
    vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal);                         
    float Attenuation2 =  l.Atten.Constant + l.Atten.Linear * Distance + l.Atten.Exp * Distance * Distance;                                 
                                                                                            
    return Color / Attenuation2;                                                             
}

vec4 CalcSpotLight(SpotLight l, vec3 Normal)                                                
{                                                                                           
    vec3 LightToPixel = normalize(viewPos - l.Base.Position);                             
    float SpotFactor = dot(LightToPixel, l.Direction);                                      
                                                                                            
    if (SpotFactor > l.Cutoff) {                                                            
        vec4 Color = CalcPointLight(l.Base, Normal);                                        
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - l.Cutoff));                   
    }                                                                                       
    else {                                                                                  
        return vec4(0,0,0,0);                                                               
    }                                                                                       
}              


void main(void)
{
	gl_Position = mvpMatrix * vec4(v_position, 1.0);
	texture = v_texture;

	vec3 normal = (modelMatrix * vec4(v_normal, 0.0)).xyz;
	viewPos = (modelMatrix * vec4(v_position, 1.0)).xyz;


	vec3 Normal = normalize(normal);                                                       
    vec4 TotalLight = CalcDirectionalLight(Normal);                                         
                                                                                            
    TotalLight += CalcPointLight(gPointLights, Normal);                                            
              
    TotalLight += CalcSpotLight(gSpotLights, Normal);                                                                                    
                                                                                            
    color = TotalLight;  
}
