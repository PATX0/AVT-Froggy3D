#version 330

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;

uniform int texMode;
out vec4 colorOut;

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
	vec2 tex_coord;
} DataIn;

void main() {

	vec4 spec = vec4(0.0);
	vec4 texel, texel1; 

	vec3 n = normalize(DataIn.normal);
	vec3 l = normalize(DataIn.lightDir);
	vec3 e = normalize(DataIn.eye); 

	float intensity = max(dot(n,l), 0.0);
	if (intensity > 0.0) {
		vec3 h = normalize(l + e);
		float intSpec = max(dot(h,n), 0.0);
		spec = mat.specular * pow(intSpec, mat.shininess);
	}


	if(texMode == 0) // modulate diffuse color with texel color
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from lighwood.tga
		colorOut = max(intensity * mat.diffuse * texel + spec,mat.ambient * texel);
	}
	else if (texMode == 2) // diffuse color is replaced by texel color, with specular area or ambient (0.1*texel)
	{
		texel = texture(texmap, DataIn.tex_coord);  // texel from stone.tga
		colorOut = max(intensity*texel + spec, 0.1*texel);
	}
	else // multitexturing
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from lighwood.tga
		texel1 = texture(texmap1, DataIn.tex_coord);  // texel from checker.tga
		colorOut = max(intensity*texel*texel1 + spec, 0.1*texel*texel1);
		//colorOut = texel * texel1;
	}

//colorOut = max(intensity * mat.diffuse + spec, mat.ambient)* //texture(texmap, DataIn.tex_coord) * texture(texmap1, DataIn.tex_coord);

}