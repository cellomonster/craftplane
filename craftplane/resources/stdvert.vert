#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNorm;

out vec2 TexCoord;
out vec3 Norm;

uniform mat4 trans;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * trans * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    Norm = aNorm;
}