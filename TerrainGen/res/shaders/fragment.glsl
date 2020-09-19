#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
flat in vec3 Color;

uniform vec3 lightPos; 
vec3 lightColor = vec3(1.0, 0.8, 0.8);
float ambientStrength = 1.0;

void main()
{
	vec3 ambient = ambientStrength * lightColor;

	//vec3 norm = normalize(Normal);
	//vec3 lightDir = normalize(lightPos - FragPos);

	//float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffuse = diff * lightColor;

	vec3 result = (ambient) * Color;
	//vec3 result = (ambient + diffuse) * Color;
	FragColor = vec4(result, 1.0);
}