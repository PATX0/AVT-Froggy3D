#version 330 core

#define NUMBER_OF_CANDLES 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 colorOut;

struct Materials {
	sampler2D diffuse;
	vec4 ambient;
	sampler2D specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;


struct PointLight
{
	vec3 position;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	
	float constant; 
	float  linear; 
	float quadratic;
};

uniform PointLight pointLight;
uniform PointLight candles[NUMBER_OF_CANDLES];

/*struct DirLight
{
	vec3 dir;
	
 	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
 
};

uniform DirLight dirLight;

struct SpotLight
{
	vec3 dir;
	vec3 position;
	float cutOff;
	float outCutOff;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant; 
	float  linear; 
	float quadratic;
};

uniform SpotLight spotLight;
*/
uniform vec3 viewPos;

//vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

//vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {

	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize( viewPos - FragPos);
	vec3 result;
	
	//vec3 result = CalcDirLight(dirLight, norm, viewDir);
	
	for(int i=0; i < NUMBER_OF_CANDLES; i++){
		result += CalcPointLight(candles[i], norm, FragPos, viewDir);
	}
	
	//result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
	
	colorOut = vec4(result, 1.0);
	
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance*distance));
	
	vec3  ambient = light.ambient * vec3(texture(mat.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(mat.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(mat.specular, TexCoords));
	
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	
	return (ambient + diffuse + specular);
}

/*vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	
	vec3 lightDir = normalize(-light.dir);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
	
	vec3 ambient = light.ambient * vec3(texture(mat.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(mat.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(mat.specular, TexCoords));
	
	return (ambient + diffuse + specular);
}


vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance*distance));
	
	float theta = dot(lightDir, normalize(-light.dir));
	float eps = light.cutOff - light.outCutOff;
	float intensity = clamp((theta - light.outCutOff)/eps , 0.0, 1.0);
	
	vec3  ambient = light.ambient * vec3(texture(mat.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(mat.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(mat.specular, TexCoords));
	
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	
	return (ambient + diffuse + specular);

}*/
	