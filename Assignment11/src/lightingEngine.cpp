#include <limits.h>
#include <string.h>

#include "lightingEngine.h"


LightingEngine::LightingEngine()
{   
}

bool LightingEngine::initialize(string vs, string fs)
{
    // initialize all lighting types to on
    ambientLightOn = true;
    diffuseLightOn = true;
    specularLightOn = true;
    pointLightOn = true;
    spotLightOn = true;

    // initialize shaders
    shader vertexShader(GL_VERTEX_SHADER);
    shader fragmentShader(GL_FRAGMENT_SHADER);

    if (!vertexShader.initialize(vs))
        return false;

    if (!fragmentShader.initialize(fs))
        return false;


    // link the 2 shader objects into a shaderProgram
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader.getShader());
    glAttachShader(shaderProgram, fragmentShader.getShader());
    glLinkProgram(shaderProgram);
    
    GLint shader_status;

    // check if everything linked ok
    char fragmentBuffer[512];

    glGetProgramInfoLog(shaderProgram, 512, NULL, fragmentBuffer);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER shaderProgram FAILED TO LINK" << std::endl;

        cout << fragmentBuffer;
        cout << endl;
        return false;
    }

    loc_vertexPosition = glGetAttribLocation(shaderProgram, const_cast<const char*>("v_position"));
    if((signed)loc_vertexPosition == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_vertexTexture = glGetAttribLocation(shaderProgram, const_cast<const char*>("v_texture"));
    if((signed)loc_vertexTexture == -1)
    {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }

    loc_vertexNormal = glGetAttribLocation(shaderProgram, const_cast<const char*>("v_normal"));
    if((signed)loc_vertexNormal == -1)
    {
        std::cerr << "[F] V_NORMAL NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpMatrix = glGetUniformLocation(shaderProgram, const_cast<const char*>("mvpMatrix"));
    if((signed)loc_mvpMatrix == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }
    loc_modelMatrix = glGetUniformLocation(shaderProgram, const_cast<const char*>("modelMatrix"));
    if((signed)loc_modelMatrix == -1)
    {
        std::cerr << "[F] MODELMATRIX NOT FOUND" << std::endl;
        return false;    GLuint vertexShadingProgram;

    }

    if(!setupDirectionalLightShaderData(shaderProgram))
    {
        return false;
    }


        // point light
    if(!setupPointLightShaderData(shaderProgram, 0))
    {
        return false;
    }

        // spot light
    if(!setupSpotLightShaderData(shaderProgram, 0))
    {
        return false;
    }



    loc_matSpecularIntensity = glGetUniformLocation(shaderProgram, const_cast<const char*>("gMatSpecularIntensity"));
    if((signed)loc_matSpecularIntensity == -1)
    {
        std::cerr << "[F] loc_matSpecularIntensity NOT FOUND" << std::endl;
        return false;
    }
    loc_matSpecularPower = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpecularPower"));
    if((signed)loc_matSpecularPower == -1)
    {
        std::cerr << "[F] loc_matSpecularPower NOT FOUND" << std::endl;
        return false;
    }
    loc_cameraPos = glGetUniformLocation(shaderProgram, const_cast<const char*>("cameraPos"));
    if((signed)loc_cameraPos == -1)
    {
        std::cerr << "[F] loc_cameraPos NOT FOUND" << std::endl;
        return false;
    }   

    directLight.Color = glm::vec3(0.5f, 0.5f, 0.5f);
    directLight.Direction = glm::vec3(0.0f, -16.0, 0.0);

    return true;
}

void LightingEngine::setMVP(const glm::mat4 mvp)
{
    glUniformMatrix4fv(loc_mvpMatrix, 1, GL_FALSE, glm::value_ptr(mvp));
}


void LightingEngine::setModelMatrix(const glm::mat4 model)
{
    glUniformMatrix4fv(loc_modelMatrix, 1, GL_FALSE, glm::value_ptr(model));
}


void LightingEngine::setTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(loc_sampler, TextureUnit);
}

void LightingEngine::setCameraPos(const glm::vec3& cameraPos)
{
    glUniform3f(loc_cameraPos, cameraPos.x, cameraPos.y, cameraPos.z);
}

void LightingEngine::setDirectionalLight(const DirectionalLight& dLight)
{
    glUniform3f(loc_directLight.Color, dLight.Color.x, dLight.Color.y, dLight.Color.z);
    glUniform1f(loc_directLight.AmbientIntensity, dLight.AmbientIntensity);
    glm::vec3 Direction = dLight.Direction;
    glm::normalize(Direction);
    glUniform3f(loc_directLight.Direction, Direction.x, Direction.y, Direction.z);
    glUniform1f(loc_directLight.DiffuseIntensity, dLight.DiffuseIntensity);
}

void LightingEngine::setMatSpecularIntensity()
{
    glUniform1f(loc_matSpecularIntensity, specularIntensity);
}

void LightingEngine::setMatSpecularPower(float Power)
{
    glUniform1f(loc_matSpecularPower, Power);
}

void LightingEngine::setPointLights(unsigned int NumLights, const PointLight* pLights)
{
    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(loc_pointLights[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(loc_pointLights[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(loc_pointLights[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform3f(loc_pointLights[i].Position, pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
        glUniform1f(loc_pointLights[i].AttenuationLocation.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(loc_pointLights[i].AttenuationLocation.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(loc_pointLights[i].AttenuationLocation.Exp, pLights[i].Attenuation.Exp);
    }
}

void LightingEngine::setSpotLights(unsigned int NumLights, const SpotLight* sLights)
{
    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(loc_spotLights[i].Color, sLights[i].Color.x, sLights[i].Color.y, sLights[i].Color.z);
        glUniform1f(loc_spotLights[i].AmbientIntensity, sLights[i].AmbientIntensity);
        glUniform1f(loc_spotLights[i].DiffuseIntensity, sLights[i].DiffuseIntensity);
        glUniform3f(loc_spotLights[i].Position, sLights[i].Position.x, sLights[i].Position.y, sLights[i].Position.z);
        glm::vec3 Direction = sLights[i].Direction;
        glm::normalize(Direction);
        glUniform3f(loc_spotLights[i].Direction, Direction.x, Direction.y, Direction.z);
        glUniform1f(loc_spotLights[i].Cutoff, glm::cos(glm::radians(sLights[i].Cutoff)));
        glUniform1f(loc_spotLights[i].AttenuationLocation.Constant, sLights[i].Attenuation.Constant);
        glUniform1f(loc_spotLights[i].AttenuationLocation.Linear, sLights[i].Attenuation.Linear);
        glUniform1f(loc_spotLights[i].AttenuationLocation.Exp, sLights[i].Attenuation.Exp);
    }
}

void LightingEngine::toggleAmbientLight()
    {
        if(ambientLightOn)
            directLight.AmbientIntensity = 0.5f;
        else
            directLight.AmbientIntensity = 0.0f;

    }


void LightingEngine::toggleDiffuseLight()
    {
        if(diffuseLightOn)
            directLight.DiffuseIntensity = 0.05f;
        else
            directLight.DiffuseIntensity = 0.0f;
    }


void LightingEngine::toggleSpecularLight()
    {
        if(specularLightOn)
            specularIntensity = 1.0f;
        else
            specularIntensity = 0.0f;
    }


void LightingEngine::togglePointLight()
    {
        pointLights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
        pointLights[0].Position = glm::vec3(0.0f, 16.0f, 0.0);
        pointLights[0].Attenuation.Linear = 0.1f;

        if(pointLightOn)
        {
            pointLights[0].DiffuseIntensity = 0.5f;
            pointLights[0].AmbientIntensity = 0.5f;  
        }
        else
        {
            pointLights[0].DiffuseIntensity = 0.0f;
            pointLights[0].AmbientIntensity = 0.0f;              
        }      
    }


void LightingEngine::toggleSpotLight()
    {
        spotLights[0].Color = glm::vec3(0.0f, 1.0f, 1.0f);
        spotLights[0].Direction = glm::vec3(0.0f, -1.0f, 0.0f);
        spotLights[0].Attenuation.Linear = 0.5f;
        spotLights[0].Cutoff = 75.0f;

        if(spotLightOn)
        {
            spotLights[0].DiffuseIntensity = 0.9f;
            spotLights[0].AmbientIntensity = 0.0f;
        }
        else
        {
            spotLights[0].DiffuseIntensity = 0.0f;
            spotLights[0].AmbientIntensity = 0.0f;
        }
    }


bool LightingEngine::setupDirectionalLightShaderData(GLuint shaderProgram)
    {
        loc_directLight.Color = glGetUniformLocation(shaderProgram, const_cast<const char*>("gDirectionalLight.Base.Color"));
        if((signed)loc_directLight.Color == -1)
            {
                std::cerr << "[F] Color NOT FOUND" << std::endl;
                return false;
            }
        loc_directLight.AmbientIntensity = glGetUniformLocation(shaderProgram, const_cast<const char*>("gDirectionalLight.Base.AmbientIntensity"));
        if((signed)loc_directLight.AmbientIntensity == -1)
            {
                std::cerr << "[F] AmbientIntensity NOT FOUND" << std::endl;
                return false;
            }
        loc_directLight.DiffuseIntensity = glGetUniformLocation(shaderProgram, const_cast<const char*>("gDirectionalLight.Base.DiffuseIntensity"));
        if((signed)loc_directLight.DiffuseIntensity == -1)
            {
                std::cerr << "[F] DiffuseIntensity NOT FOUND" << std::endl;
                return false;
            }
        loc_directLight.Direction = glGetUniformLocation(shaderProgram, const_cast<const char*>("gDirectionalLight.Direction"));
        if((signed)loc_directLight.Direction == -1)
            {
                std::cerr << "[F] Direction NOT FOUND" << std::endl;
                return false;
            }

        return true;
    }


bool LightingEngine::setupPointLightShaderData(GLuint shaderProgram, int index)
    {
        loc_pointLights[index].Color = glGetUniformLocation(shaderProgram, const_cast<const char*>("gPointLights.Base.Color"));
        if((signed)loc_pointLights[index].Color == -1)
            {
                std::cerr << "[F] Color NOT FOUND" << std::endl;
                return false;
            }
        loc_pointLights[index].AmbientIntensity = glGetUniformLocation(shaderProgram, const_cast<const char*>("gPointLights.Base.AmbientIntensity"));
        if((signed)loc_pointLights[index].AmbientIntensity == -1)
            {
                std::cerr << "[F] AmbientIntensity NOT FOUND" << std::endl;
                return false;
            }
        loc_pointLights[index].DiffuseIntensity = glGetUniformLocation(shaderProgram, const_cast<const char*>("gPointLights.Base.DiffuseIntensity"));
        if((signed)loc_pointLights[index].DiffuseIntensity == -1)
            {
                std::cerr << "[F] DiffuseIntensity NOT FOUND" << std::endl;
                return false;
            }
        loc_pointLights[index].Position = glGetUniformLocation(shaderProgram, const_cast<const char*>("gPointLights.Position"));
        if((signed)loc_pointLights[index].Position == -1)
            {
                std::cerr << "[F] Direction NOT FOUND" << std::endl;
                return false;
            }

        loc_pointLights[index].AttenuationLocation.Constant = glGetUniformLocation(shaderProgram, const_cast<const char*>("gPointLights.Atten.Constant"));
        if((signed)loc_pointLights[index].AttenuationLocation.Constant == -1)
            {
                std::cerr << "[F] AmbientIntensity NOT FOUND" << std::endl;
                return false;
            }
        loc_pointLights[index].AttenuationLocation.Linear = glGetUniformLocation(shaderProgram, const_cast<const char*>("gPointLights.Atten.Linear"));
        if((signed)loc_pointLights[index].AttenuationLocation.Linear == -1)
            {
                std::cerr << "[F] DiffuseIntensity NOT FOUND" << std::endl;
                return false;
            }
        loc_pointLights[index].AttenuationLocation.Exp = glGetUniformLocation(shaderProgram, const_cast<const char*>("gPointLights.Atten.Exp"));
        if((signed)loc_pointLights[index].AttenuationLocation.Exp == -1)
            {
                std::cerr << "[F] Direction NOT FOUND" << std::endl;
                return false;
            }

        return true;
    }


bool LightingEngine::setupSpotLightShaderData(GLuint shaderProgram, int index)
    {
        loc_spotLights[index].Color = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpotLights.Base.Base.Color"));
        if((signed)loc_spotLights[index].Color == -1)
            {
                std::cerr << "[F] Color NOT FOUND" << std::endl;
                return false;
            }
        loc_spotLights[index].AmbientIntensity = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpotLights.Base.Base.AmbientIntensity"));
        if((signed)loc_spotLights[index].AmbientIntensity == -1)
            {
                std::cerr << "[F] AmbientIntensity NOT FOUND" << std::endl;
                return false;
            }
        loc_spotLights[index].DiffuseIntensity = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpotLights.Base.Base.DiffuseIntensity"));
        if((signed)loc_spotLights[index].DiffuseIntensity == -1)
            {
                std::cerr << "[F] DiffuseIntensity NOT FOUND" << std::endl;
                return false;
            }
        loc_spotLights[index].Position = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpotLights.Base.Position"));
        if((signed)loc_spotLights[index].Position == -1)
            {
                std::cerr << "[F] Position NOT FOUND" << std::endl;
                return false;
            }
        loc_spotLights[index].Direction = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpotLights.Direction"));
        if((signed)loc_spotLights[index].Direction == -1)
            {
                std::cerr << "[F] Direction NOT FOUND" << std::endl;
                return false;
            }
        loc_spotLights[index].Cutoff = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpotLights.Cutoff"));
        if((signed)loc_spotLights[index].Cutoff == -1)
            {
                std::cerr << "[F] Cutoff NOT FOUND" << std::endl;
                return false;
            }
        loc_spotLights[index].AttenuationLocation.Constant = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpotLights.Base.Atten.Constant"));
        if((signed)loc_spotLights[index].AttenuationLocation.Constant == -1)
            {
                std::cerr << "[F] Attenuation constant NOT FOUND" << std::endl;
                return false;
            }
        loc_spotLights[index].AttenuationLocation.Linear = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpotLights.Base.Atten.Linear"));
        if((signed)loc_spotLights[index].AttenuationLocation.Linear == -1)
            {
                std::cerr << "[F] Attenuation linear NOT FOUND" << std::endl;
                return false;
            }
        loc_spotLights[index].AttenuationLocation.Exp = glGetUniformLocation(shaderProgram, const_cast<const char*>("gSpotLights.Base.Atten.Exp"));
        if((signed)loc_spotLights[index].AttenuationLocation.Exp == -1)
            {
                std::cerr << "[F] Attenuation exp NOT FOUND" << std::endl;
                return false;
            }
        return true;
    }