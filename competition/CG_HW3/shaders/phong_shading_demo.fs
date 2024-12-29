#version 330 core

// Data from vertex shader.
in vec3 FragPos;
in vec3 NormalOut;
in vec2 TexCoordOut;

// Maximum number of lights
const int MAX_DIR_LIGHTS = 4;
const int MAX_POINT_LIGHTS = 8;
const int MAX_SPOT_LIGHTS = 8;
const int MAX_AREA_LIGHTS = 4; 

// Structures for different light types
struct DirectionalLight {
    vec3 direction;
    vec3 radiance;
};

struct PointLight {
    vec3 position;
    vec3 intensity;
    float constant;
    float linear;
    float quadratic;
    float decayStart;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 intensity;
    float cosCutoffStart;
    float cosCutoffEnd;
    float constant;
    float linear;
    float quadratic;
    float decayStart;
};

// AreaLight structure
struct AreaLight {
    vec3 position;
    vec3 intensity;
    vec3 direction;
    float width;
    float height;
    int samples;
    float constant;
    float linear;
    float quadratic;
    float decayStart;
};

// Uniform arrays for lights
uniform int numDirLights;
uniform DirectionalLight dirLights[MAX_DIR_LIGHTS];

uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform int numSpotLights;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

// AreaLight uniforms
uniform int numAreaLights;
uniform AreaLight areaLights[MAX_AREA_LIGHTS];

// Uniform variables.
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform vec3 cameraPos; // 在相機空間中，cameraPos 可設定為 vec3(0.0, 0.0, 0.0)
uniform vec3 ambientLight;

// Material properties
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;

// Texture
uniform sampler2D mapKd;
uniform sampler2D mapKs;

// bool for shading and lighting
uniform bool isBlingPhong;
uniform bool onAmbientLight;
uniform bool onDiffuseLight;
uniform bool onSpecularLight;

// Output data.
out vec4 FragColor;

// Ambient light.
vec3 Ambient(vec3 Ka)
{
    return ambientLight * Ka;
}

// Diffuse light.
vec3 Diffuse(vec3 normal, vec3 lightDir, vec3 lightRadiance, vec3 Kd)
{
    float diff = max(dot(normal, lightDir), 0.0);
    return Kd * lightRadiance * diff;
}

// Specular light.
vec3 Specular(vec3 normal, vec3 lightDir, vec3 viewDir, vec3 lightRadiance, vec3 Ks, float Ns)
{
    float spec;
    if(isBlingPhong){
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), Ns);
    } else {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), Ns);
    }
    return spec * lightRadiance * Ks;
}

void main()
{
    // 採樣漫反射貼圖
    vec3 texColor = texture(mapKd, TexCoordOut).rgb;
    vec3 effectiveKd = (Kd == vec3(0.0)) ? texColor : Kd;

    // 採樣鏡面反射貼圖
    vec3 specMap = texture(mapKs, TexCoordOut).rgb;
    // 計算有效的 Ks 值
    vec3 effectiveKs = (Ks == vec3(0.0)) ? specMap : Ks * specMap;
    

    vec3 norm = normalize(NormalOut);
    // 在相機空間中，相機位置為原點
    vec3 viewDir = normalize(-FragPos);

    // Ambient light.
    vec3 ambient = Ambient(Ka);

    // Directional lights
    vec3 dirLightResult = vec3(0.0);
    for(int i = 0; i < numDirLights; i++) {
        vec3 lightDir = normalize( -dirLights[i].direction); // 確保方向正確
        vec3 diffuse = Diffuse(norm, lightDir, dirLights[i].radiance, effectiveKd);
        vec3 specular = Specular(norm, lightDir, viewDir, dirLights[i].radiance, effectiveKs, Ns);
        
        if(!onDiffuseLight) diffuse = vec3(0.0);
        if(!onSpecularLight) specular = vec3(0.0);
        
        dirLightResult += diffuse + specular;
    }

    // Point lights
    vec3 pointLightResult = vec3(0.0);
    for(int i = 0; i < numPointLights; i++) {
        vec3 lightPosCamSpace = (viewMatrix * worldMatrix * vec4(pointLights[i].position, 1.0)).xyz;
        vec3 lightDir = normalize(lightPosCamSpace - FragPos);
        float distance = length(lightPosCamSpace - FragPos);
        float attenuation;
        if(distance <= pointLights[i].decayStart) {
            attenuation = 1.0;
        } else {
            float adjustedDistance = distance - pointLights[i].decayStart;
            attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * adjustedDistance + pointLights[i].quadratic * (adjustedDistance * adjustedDistance));
        }
        vec3 radiance = pointLights[i].intensity * attenuation;
        
        vec3 diffuse = Diffuse(norm, lightDir, radiance, effectiveKd);
        vec3 specular = Specular(norm, lightDir, viewDir, radiance, effectiveKs, Ns);
        
        if(!onDiffuseLight) diffuse = vec3(0.0);
        if(!onSpecularLight) specular = vec3(0.0);
        
        pointLightResult += diffuse + specular;
    }

    // Spot lights
    vec3 spotLightResult = vec3(0.0);
    for(int i = 0; i < numSpotLights; i++) {
        vec3 lightPosCamSpace = (viewMatrix * worldMatrix * vec4(spotLights[i].position, 1.0)).xyz;
        vec3 lightDir = normalize(lightPosCamSpace - FragPos);
        vec3 spotDirCamSpace = normalize(viewMatrix * worldMatrix * vec4(spotLights[i].direction, 0.0)).xyz;

        float cosTheta = dot(lightDir, spotDirCamSpace);
        float cosEpsilon = spotLights[i].cosCutoffStart - spotLights[i].cosCutoffEnd;
        float spotLightDistance = length(lightPosCamSpace - FragPos);

        float attenuation;
        if(spotLightDistance <= spotLights[i].decayStart) {
            attenuation = 1.0;
        } else {
            float adjustedDistance = spotLightDistance - spotLights[i].decayStart;
            attenuation = 1.0 / (spotLights[i].constant + spotLights[i].linear * adjustedDistance + spotLights[i].quadratic * (adjustedDistance * adjustedDistance));
        }

        float intensityFactor = clamp((cosTheta - spotLights[i].cosCutoffEnd) / cosEpsilon, 0.0, 1.0);
        vec3 radiance = intensityFactor * spotLights[i].intensity * attenuation;

        vec3 diffuse = Diffuse(norm, lightDir, radiance, effectiveKd);
        vec3 specular = Specular(norm, lightDir, viewDir, radiance, effectiveKs, Ns);

        if(!onDiffuseLight) diffuse = vec3(0.0);
        if(!onSpecularLight) specular = vec3(0.0);

        spotLightResult += diffuse + specular;
    }

    // Area lights
    vec3 areaLightResult = vec3(0.0);
    for(int i = 0; i < numAreaLights; i++) {
        // 定義區域光源的方向和正交向量
        vec3 lightDir = normalize(areaLights[i].direction);
        vec3 right = normalize(cross(lightDir, vec3(0.0, 1.0, 0.0)));
        vec3 up = normalize(cross(right, lightDir));

        // 計算每個樣本點的光照
        for(int s = 0; s < areaLights[i].samples; s++) {
            // 生成隨機偏移（或使用規則分布）
            float randU = fract(sin(float(s) * 12.9898) * 43758.5453);
            float randV = fract(sin(float(s) * 78.233) * 43758.5453);

            // 計算樣本點在面光源上的位置
            vec3 samplePos = areaLights[i].position 
                            + (randU - 0.5) * areaLights[i].width * right 
                            + (randV - 0.5) * areaLights[i].height * up;

            // 轉換到相機空間
            vec3 lightPosCamSpace = (viewMatrix * worldMatrix * vec4(samplePos, 1.0)).xyz;
            vec3 lightDirection = normalize(lightPosCamSpace - FragPos);
            float distance = length(lightPosCamSpace - FragPos);
            float attenuation;
            if(distance <= areaLights[i].decayStart) {
                attenuation = 1.0;
            } else {
                float adjustedDistance = distance - areaLights[i].decayStart;
                attenuation = 1.0 / (areaLights[i].constant + areaLights[i].linear * adjustedDistance + areaLights[i].quadratic * (adjustedDistance * adjustedDistance));
            }
            vec3 radiance = areaLights[i].intensity * attenuation;

            // 計算漫反射和鏡面反射
            vec3 diffuse = Diffuse(norm, lightDirection, radiance, effectiveKd);
            vec3 specular = Specular(norm, lightDirection, viewDir, radiance, effectiveKs, Ns);

            if(!onDiffuseLight) diffuse = vec3(0.0);
            if(!onSpecularLight) specular = vec3(0.0);

            areaLightResult += diffuse + specular;
    }

    // 平均樣本點的光照效果
    areaLightResult /= float(areaLights[i].samples);
}

    // Final color.
    vec3 result = ambient + dirLightResult + pointLightResult + spotLightResult + areaLightResult;
    if(!onAmbientLight) result -= ambient;
    FragColor = vec4(result, 1.0);
}