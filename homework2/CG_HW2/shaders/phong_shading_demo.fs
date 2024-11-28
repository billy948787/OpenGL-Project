#version 330 core

// Data from vertex shader.
in vec3 FragPos;
in vec3 NormalOut;

// Uniform variables.
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform vec3 cameraPos;
uniform vec3 ambientLight;
uniform vec3 dirLightDir;
uniform vec3 dirLightRadiance;
uniform vec3 pointLightPos;
uniform vec3 pointLightIntensity;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightIntensity;
uniform float spotLightCutOffStart;
uniform float spotLightTotalWidth;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
// bool for shading and lighting
uniform bool isBlingPhong;
uniform bool onPointLight;
uniform bool onSpotLight;
uniform bool onDirLight;
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
    // Transform light dir and light pos to view space
    vec3 viewDirLightDir = normalize(vec3(viewMatrix * vec4(-dirLightDir, 0.0)));
    vec3 viewSpotLightDir = normalize(vec3(viewMatrix * vec4(spotLightDir, 0.0)));

    vec4 tempViewPointLightPos = viewMatrix * vec4(pointLightPos, 1.0);
    vec4 tempViewSpotLightPos = viewMatrix * vec4(spotLightPos, 1.0);

    vec3 viewPointLightPos = tempViewPointLightPos.xyz / tempViewPointLightPos.w;
    vec3 viewSpotLightPos = tempViewSpotLightPos.xyz / tempViewSpotLightPos.w;

    vec3 norm = normalize(NormalOut);
    vec3 viewDir = normalize(-FragPos); // cameraPos in view space is (0, 0, 0)

    // Ambient light.
    vec3 ambient = Ambient(Ka);

    // Directional light.
    vec3 lightDir = normalize(viewDirLightDir);
    vec3 dirDiffuse = Diffuse(norm, lightDir, dirLightRadiance, Kd);
    vec3 dirSpecular = Specular(norm, lightDir, viewDir, dirLightRadiance, Ks, Ns);

    // Final directional light.
    if(!onDiffuseLight) dirDiffuse = vec3(0.0);
    if(!onSpecularLight) dirSpecular = vec3(0.0);
    vec3 dirLight = dirDiffuse + dirSpecular;

    // Point light.
    vec3 pointLightDir = normalize(viewPointLightPos - FragPos);
    float pointLightDistance = length(viewPointLightPos - FragPos);
    float attenuation;
    
    
    attenuation = 1.0f / (pointLightDistance * pointLightDistance);
    
    vec3 radiance = pointLightIntensity * attenuation;
    vec3 pointDiffuse = Diffuse(norm, pointLightDir, radiance, Kd);
    vec3 pointSpecular = Specular(norm, pointLightDir, viewDir, radiance, Ks, Ns);

    // Final point light.
    if(!onDiffuseLight) pointDiffuse = vec3(0.0);
    if(!onSpecularLight) pointSpecular = vec3(0.0);
    vec3 pointLight = pointDiffuse + pointSpecular;

    // Spot light.
    vec3 spotLightToFragDir = normalize(viewSpotLightPos - FragPos);
    float cosCutOffStart = cos(spotLightCutOffStart);
    float cosTotalWidth = cos(spotLightTotalWidth);
    float cosTheta = dot(viewSpotLightDir, -spotLightToFragDir);
    float cosEpsilon = cosCutOffStart - cosTotalWidth;
    float spotLightDistance = length(viewSpotLightPos - FragPos);
    
    attenuation = 1.0f / (spotLightDistance * spotLightDistance);
    
    if(cosTheta > cosCutOffStart) {
        radiance = spotLightIntensity * attenuation;
    } else if(cosTheta < cosTotalWidth) {
        radiance = vec3(0.0);
    } else {
        float intensityFactor = clamp((cosTheta - cosTotalWidth) / cosEpsilon, 0.0, 1.0);
        radiance = intensityFactor * spotLightIntensity * attenuation;
    }
    vec3 spotDiffuse = Diffuse(norm, spotLightToFragDir, radiance, Kd);
    vec3 spotSpecular = Specular(norm, spotLightToFragDir, viewDir, radiance, Ks, Ns);

    // Final spot light.
    if(!onDiffuseLight) spotDiffuse = vec3(0.0);
    if(!onSpecularLight) spotSpecular = vec3(0.0);
    vec3 spotLight = spotDiffuse + spotSpecular;

    // Final color.
    if(!onAmbientLight) ambient = vec3(0.0);
    if(!onDirLight) dirLight = vec3(0.0);
    if(!onPointLight) pointLight = vec3(0.0);
    if(!onSpotLight) spotLight = vec3(0.0);

    vec3 result = ambient + dirLight + pointLight + spotLight;

    FragColor = vec4(result, 1.0);
}