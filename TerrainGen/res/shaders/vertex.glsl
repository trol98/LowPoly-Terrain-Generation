#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

uniform mat4 view;
uniform mat4 projection;
//uniform mat4 model isn't needed

out vec3 FragPos;
out vec3 Normal;

flat out vec3 Color;


void main()
{
	Normal = aNormal;
	Color = aColor;
	FragPos = vec3(vec4(aPos, 1.0));
	gl_Position = projection * view  * vec4(aPos, 1.0);
}