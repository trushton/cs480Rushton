#ifndef LIGHTING_ENGINE_H
#define	LIGHTING_ENGINE_H


#include "shader.h"


#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier


using namespace std;

#define INVALID_UNIFORM_LOCATION -1

    static const unsigned int MAX_POINT_LIGHTS = 2;
    static const unsigned int MAX_SPOT_LIGHTS = 2;
    static const float FieldDepth = 20.0f;
    static const float m_scale = 0.0f;

struct BaseLight
{
    glm::vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;

    BaseLight()
    {
        Color = glm::vec3(0.0f, 0.0f, 0.0f);
        AmbientIntensity = 0.0f;
        DiffuseIntensity = 0.0f;
    }
};

struct DirectionalLight : public BaseLight
{        
    glm::vec3 Direction;

    DirectionalLight()
    {
        Direction = glm::vec3(0.0f, 0.0f, 0.0f);
    }
};

struct PointLight : public BaseLight
{
    glm::vec3 Position;

    struct
    {
        float Constant;
        float Linear;
        float Exp;
    } Attenuation;

    PointLight()
    {
        Position = glm::vec3(0.0f, 0.0f, 0.0f);
        Attenuation.Constant = 1.0f;
        Attenuation.Linear = 0.0f;
        Attenuation.Exp = 0.0f;
    }
};

struct SpotLight : public PointLight
{
    glm::vec3 Direction;
    float Cutoff;

    SpotLight()
    {
        Direction = glm::vec3(0.0f, 0.0f, 0.0f);
        Cutoff = 0.0f;
    }
};

struct DirectLightLocations{
    GLuint Color;
    GLuint AmbientIntensity;
    GLuint DiffuseIntensity;
    GLuint Direction;
};

struct PointLightLocations{

    GLuint Color;
    GLuint AmbientIntensity;
    GLuint DiffuseIntensity;
    GLuint Position;

    struct {
        GLuint Constant;
        GLuint Linear;
        GLuint Exp;
    } AttenuationLocation;
};

struct SpotLightLocations{
    GLuint Color;
    GLuint AmbientIntensity;
    GLuint DiffuseIntensity;
    GLuint Position;
    GLuint Direction;
    GLuint Cutoff;
    struct {
        GLuint Constant;
        GLuint Linear;
        GLuint Exp;
    } AttenuationLocation;
};

class LightingEngine {
public:

    LightingEngine();

    virtual bool initialize(string, string);

    void setMVP(const glm::mat4 MVP);
    void setModelMatrix(const glm::mat4 model);
    void setTextureUnit(unsigned int TextureUnit);
    void setDirectionalLight(const DirectionalLight& dLight);
    void setPointLights(unsigned int NumLights, const PointLight* pLights);
    void setSpotLights(unsigned int NumLights, const SpotLight* sLights);
    void setCameraPos(const glm::vec3& cameraPos);
    void setMatSpecularIntensity();
    void setMatSpecularPower(float Power);

    void toggleAmbientLight();
    void toggleDiffuseLight();
    void toggleSpecularLight();
    void togglePointLight();
    void toggleSpotLight();

    GLuint loc_vertexPosition;
    GLuint loc_vertexTexture;
    GLuint loc_vertexNormal;

    GLuint loc_mvpMatrix;
    GLuint loc_modelMatrix;
    GLuint loc_sampler;
    GLuint loc_cameraPos;
    GLuint loc_matSpecularIntensity;
    GLuint loc_matSpecularPower;

    DirectLightLocations loc_directLight;
    PointLightLocations loc_pointLights[MAX_POINT_LIGHTS];
    SpotLightLocations loc_spotLights[MAX_SPOT_LIGHTS];

    GLuint shaderProgram;

    DirectionalLight directLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];

    bool ambientLightOn;
    bool diffuseLightOn;
    bool specularLightOn;
    bool pointLightOn;
    bool spotLightOn;

private:

    float specularIntensity;
    bool setupDirectionalLightShaderData(GLuint);
    bool setupPointLightShaderData(GLuint, int);
    bool setupSpotLightShaderData(GLuint, int);

};


#endif	/* LIGHTING_ENGINE_H */
