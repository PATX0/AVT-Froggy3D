#version 330 core

#define NUMBER_OF_CANDLES 6

out vec4 colorOut;
out vec4 color;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;

} DataIn;

struct PointLight
{
	vec3 pos;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	
	float constant; 
	float  linear; 
	float quadratic;
};

struct DirLight
{
	vec3 dir;
	
 	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
 
};

struct SpotLight
{
	vec3 dir;
	vec3 pos;
	float cutOff;
	float outCutOff;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant; 
	float  linear; 
	float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform PointLight pointLight;
uniform PointLight candles[NUMBER_OF_CANDLES];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
	
  	vec3 spec = vec3(0.0);
	
	vec3 n = normalize(DataIn.normal);
	vec3 l = normalize(DataIn.lightDir);
	vec3 e = normalize(DataIn.eye);

	float intensity = max(dot(n,l), 0.0);
	
		if (intensity > 0.0) {

		vec3 h = normalize(l + e);
		float intSpec = max(dot(h,n), 0.0);
		spec = mat.specular.rgb * pow(intSpec, mat.shininess);
	}
	
	colorOut = vec4(max(intensity * mat.diffuse.rgb + spec, mat.ambient.rgb), 1.0);
	
	//3 LIGHTS
	
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize( viewPos - FragPos);
	
	vec3 result = CalcDirLight(dirLight, norm, viewDir);
	
	for(int i=0; i < NUMBER_OF_CANDLES; i++){
		result += CalcPointLight(candles[i], norm, FragPos, viewDir);
	}
	
	result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
	
	color = vec4(result, 1.0);
	

	
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	
	vec3 lightDir = normalize(-light.dir);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
	vec3  ambient = light.ambient * vec3(texture(mat.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(mat.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(mat.specular, TexCoords));
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

	vec3 lightDir = normalize(light.pos - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
	float distance = length(light.pos - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance*distance));
	
	vec3  ambient = light.ambient * vec3(texture(mat.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(mat.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(mat.specular, TexCoords));
	
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	
	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

	vec3 lightDir = normalize(light.pos - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
	float distance = length(light.pos - fragPos);
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

}
	