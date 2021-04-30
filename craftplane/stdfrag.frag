#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Norm;

uniform sampler2D Tex;

void main()
{
	FragColor = texture(Tex, TexCoord) * vec4(Norm.xyz / 2.0 + vec3(0.5f, 0.5f, 0.5f), 1.0);
} 