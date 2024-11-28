#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 NormalIn;

// Transformation matrix.
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;
uniform mat4 MVP;
uniform vec3 cameraPos;

// data pass to fragment shader
out vec3 FragPos;
out vec3 NormalOut; // Renamed output variable

void main()
{
    // Calculate normal in world space.
    vec3 normal = (normalMatrix * vec4(NormalIn, 0.0)).xyz;

    // Calculate position in world space.
    vec4 positionTmp = viewMatrix * worldMatrix * vec4(Position, 1.0);

    // Calculate position in clip space.
    gl_Position = MVP * vec4(Position, 1.0);

    // Pass data to fragment shader.
    FragPos = positionTmp.xyz / positionTmp.w;
    NormalOut = normal;
}
