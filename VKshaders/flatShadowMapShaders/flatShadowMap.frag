#version 450

void main()
{             
	gl_FragDepth = gl_FragCoord.z;
}

// layout(set = 1, binding = 1) uniform sampler2D texSampler;

// layout(location = 0) in vec3 fragColor;
// layout(location = 1) in vec3 fragmentNormal;
// layout(location = 2) in vec2 fragTexCoord;

// layout(location = 0) out vec4 outColor;

// void main() {
//     outColor = texture(texSampler, fragTexCoord);
// }

// layout(location = 0) in vec3 inFragmentPosition;
// layout(location = 1) in vec3 inFragmentNormal;
// layout(location = 2) in vec2 inFragmentTextureCoordinate;

// layout(location = 0) out vec4 outColor;

// layout(set = 1, binding = 1) uniform ViewPositionUBO {
// 	vec3 viewPosition;
// };

// layout(set = 2, binding = 2) uniform MaterialUBO {
//     vec3      ambient;
//     float     shininess;
// } material; 

// struct DirectionalLight {
// 	vec3 position;
// 	vec3 direction;
  
//     vec3 ambient;
//     vec3 diffuse;
//     vec3 specular;
// };

// struct PointLight {
//     vec3 position;

// 	vec3 ambient;
//     vec3 diffuse;
//     vec3 specular;

// 	float constant;
// 	float linear;
// 	float quadratic;
// };

// struct SpotLight {
// 	vec3  position;
// 	vec3  direction;
// 	float cutOff;
// 	float outerCutOff;

// 	vec3  ambient;
// 	vec3  diffuse;
// 	vec3  specular;

// 	float constant;
// 	float linear;
// 	float quadratic;
// };

// #define DIRECTIONAL_LIGHTS_NUMBER                          4
// #define POINT_LIGHTS_NUMBER                                32
// #define SPOT_LIGHTS_NUMBER                                 8

// layout(set = 3, binding = 3) uniform DirectionalLightsUBO {
// 	DirectionalLight directionalLightsArray[DIRECTIONAL_LIGHTS_NUMBER];
// 	int directionalLightsArraySize;
// } directionalLights;

// layout(set = 4, binding = 4) uniform PointLightsUBO {
// 	PointLight pointLightsArray[POINT_LIGHTS_NUMBER];
// 	int pointLightsArraySize;
// } pointLights;

// layout(set = 5, binding = 5) uniform SpotLightsUBO {
// 	SpotLight spotLightsArray[SPOT_LIGHTS_NUMBER];
// 	int spotLightArraySize;
// } spotLights;

// layout(set = 6, binding = 6) uniform	sampler2D diffuse;
// layout(set = 7, binding = 7) uniform	sampler2D specular;

// vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
// vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);
// vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);

// void main()
// {
// 	vec3 fragmentNormal = normalize(inFragmentNormal);
// 	vec3 viewDirection  = normalize(viewPosition - inFragmentPosition);

// 	vec3 result = vec3(0.0, 0.0, 0.0);
// 	for(int i = 0; i < directionalLights.directionalLightsArraySize; ++i )
// 		result += ComputeDirectionalLight(directionalLights.directionalLightsArray[i], inFragmentNormal, viewDirection);

// 	for(int i = 0; i < pointLights.pointLightsArraySize; ++i)
// 		result += ComputePointLight(pointLights.pointLightsArray[i], fragmentNormal, inFragmentPosition, viewDirection);

// 	for(int i = 0; i < spotLights.spotLightArraySize; ++i)
// 		result += ComputeSpotLight(spotLights.spotLightsArray[i], fragmentNormal, inFragmentPosition, viewDirection);

// 	outColor = vec4(result, 1.0);
// //	vec3 result = material.ambient * pointLights.pointLightsArray[0].diffuse;

// 	// vec3 lightDirection = normalize(pointLights.pointLightsArray[0].position - inFragmentPosition);
// 	// float differece = max(dot(fragmentNormal, lightDirection), 0.0);

// 	// vec3 diffuse = differece * pointLights.pointLightsArray[0].ambient;
	
// 	// outColor = vec4(diffuse, 1.0);
// }

// vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection) {
// 	vec3 lightDirection = normalize(light.direction);
// 	// diffuse shading
// 	float difference    = max(dot(normal, lightDirection), 0.0f);
// 	// specular shading
// 	vec3 reflectDirection   = reflect(-lightDirection, normal);
// 	float specularComponent = pow(max(dot(viewDirection, reflectDirection), 0.0f), material.shininess);
// 	// combine results
// 	vec3 ambient  = light.ambient * material.ambient;
// 	vec3 diffuse  = light.diffuse * difference * vec3(texture(diffuse, inFragmentTextureCoordinate));
// 	vec3 specular = light.specular * specularComponent * vec3(texture(specular, inFragmentTextureCoordinate));

// 	return (ambient + diffuse + specular);
// }

// vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection) {
// 	vec3 lightDirection = normalize(light.position - fragmentPosition);
// 	// Diffuse shading
// 	float difference    = max(dot(normal, lightDirection), 0.0f);
// 	// specular shading
// 	vec3 reflectDirection   = reflect(-lightDirection, normal);
// 	float specularComponent = pow(max(dot(viewDirection, reflectDirection), 0.0f), material.shininess);
// 	// attenuation
// 	float distance    = length(light.position - fragmentPosition);
// 	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

// 	// combine results
// 	vec3 ambient  = light.ambient * material.ambient;
// 	vec3 diffuse  = light.diffuse * difference * vec3(texture(diffuse, inFragmentTextureCoordinate));
// 	vec3 specular = light.specular * specularComponent * vec3(texture(specular, inFragmentTextureCoordinate));

// 	// ambient  *= attenuation;
// 	// diffuse  *= attenuation;
// 	// specular *= attenuation;

// 	return (ambient + diffuse + specular);
// }

// vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection) {
// 	vec3 lightDirection = normalize(light.position - fragmentPosition);
// 	// diffuse shading
// 	float difference    = max(dot(normal, lightDirection), 0.0f);
// 	// specular shading
// 	vec3 reflectDirection   = reflect(-lightDirection, normal);
// 	float specularComponent = pow(max(dot(viewDirection, reflectDirection), 0.0f), material.shininess);
// 	// attenuation
// 	float distance    = length(light.position - fragmentPosition);
// 	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
// 	// spotlight intensity
//     float theta     = dot(lightDirection, normalize(-light.direction));
// 	float epsilon   = light.cutOff - light.outerCutOff;
// 	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
// 	// combine results
// 	vec3 ambient  = light.ambient * material.ambient;
// 	vec3 diffuse  = light.diffuse * difference * vec3(texture(diffuse, inFragmentTextureCoordinate));
// 	vec3 specular = light.specular * specularComponent * vec3(texture(specular, inFragmentTextureCoordinate));
// //	ambient  *= attenuation * intensity;
//     diffuse  *= attenuation * intensity;
//     specular *= attenuation * intensity;
	
//     return (diffuse);
// }
