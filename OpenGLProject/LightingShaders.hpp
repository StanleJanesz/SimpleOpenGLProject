#ifndef LightingShaders_hpp
#define LightingShaders_hpp
// Vertex shader for the lighting pass
const char* lightingVS = R"(
#version 330 core
layout(location = 0) in vec2 aPos;

out vec2 TexCoords;

void main()
{
    TexCoords = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Fragment shader for the lighting pass
const char* lightingFS = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

struct Light {
    vec3 position;
    vec3 color;
    vec3 direction;
    int type;
    // type 0 = point light
    // type 1 = directional light
    // type 2 = spot light
};


uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform Light lights[6];

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float specPower;
uniform bool isFog;
uniform bool isDayLight;
uniform float fogDensity;
uniform mat4 view;
uniform bool isBlinn;
float CalculateDistance(vec3 lightPos, vec3 FragPos)
{
	return length(lightPos - FragPos);
}
float attenuation(float distance)
{
        float constant = 1.0;
		float linear = 0.9;
		float quadratic = 0.62;
		return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

vec3 CalculateDiffuse(vec3 Albedo,vec3 Normal, Light light, vec3 lightDir)
{

    float diff = max(dot(Normal, lightDir), 0.0);
    return diff * light.color * Albedo;
}
vec3 CalculateSpecular(vec3 FragPos, vec3 Normal, Light light, vec3 lightDir)
{
    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
    if (isBlinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(Normal, halfwayDir), 0.0), specPower);
		return spec * light.color;
	}
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specPower);
    return spec  * light.color;
}
vec3 CalculateAmbient(vec3 Albedo)
{
    if (isDayLight)
	    return 0.4f * Albedo;
    else
	    return 0.08f * Albedo;
}

 vec3 calculateLight(Light light, vec3 Albedo, vec3 Normal, vec3 FragPos)
 {
        vec3 lightDir = normalize(light.position - FragPos);

        float dist = length(light.position - FragPos);
		float att = attenuation(dist);
        
        vec3 diffuse = CalculateDiffuse(Albedo, Normal, light, lightDir) * att;
   
        vec3 specular = CalculateSpecular(FragPos, Normal, light, lightDir) * att;

        return (diffuse + specular) ;
 }
 vec3 calculateDirectionalLight(Light light, vec3 Albedo, vec3 Normal, vec3 FragPos) {

		vec3 diffuse = CalculateDiffuse(Albedo, Normal, light, light.direction);
   
        vec3 specular = CalculateSpecular(FragPos, Normal, light, light.direction);

        return (diffuse + specular) ;
       
	}

vec3 calculateSpotLight(Light light, vec3 Albedo, vec3 Normal, vec3 FragPos) {
        vec3 lightDir = normalize(light.position - FragPos);
        float dist = length(light.position - FragPos);
		float att = attenuation(dist);
		float theta = dot(lightDir, normalize(-light.direction));
        float cutOff = 0.91;
		float outerCutOff = 0.82;
		float epsilon   = cutOff - outerCutOff;
        float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
        
        vec3 diffuse = CalculateDiffuse(Albedo, Normal, light, lightDir) * intensity * att;
   
        vec3 specular = CalculateSpecular(FragPos, Normal, light, lightDir) * intensity * att;

        vec3 result = vec3(0.0);
		if (theta > cutOff)
			result = result + diffuse + specular;

        return result;
	}
vec3 calculateFog(vec3 objectColor,vec3 FragPos)
{
    float fogDistance = length(FragPos);
	float fogFactor = exp(- fogDensity  * fogDistance);
    vec3 fogColor = vec3(0.6, 0.6, 0.6);
    return fogFactor * objectColor + (1 - fogFactor) * fogColor;
}
void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    FragPos = vec3(view * vec4(FragPos, 1.0));
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    Normal = mat3(transpose(inverse(view))) * Normal;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    // Ambient
    
    vec3 ambient = CalculateAmbient(Albedo);

    vec3 lightsColors; 
    for (int i = 0; i < 6; i++)
	{
		if (lights[i].type == 0)
			lightsColors = lightsColors + calculateLight(lights[i], Albedo, Normal, FragPos);
		else if (lights[i].type == 1 && isDayLight)
			lightsColors = lightsColors + calculateDirectionalLight(lights[i], Albedo, Normal, FragPos);
		else if (lights[i].type == 2)
			lightsColors = lightsColors + calculateSpotLight(lights[i], Albedo, Normal, FragPos);
	}
    

    FragColor = vec4(ambient + lightsColors, 1.0);
	if (isFog)
		FragColor = vec4(calculateFog(FragColor.rgb, FragPos), 1.0);
}
)";
#endif