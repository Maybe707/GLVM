#version 410 core
out vec4 fragColor;
// flat in int spotLightSpaceMatrixArraySize;
// flat in int directionalLightSpaceMatrixArraySize;

#define DIRECTIONAL_LIGHTS_NUMBER                          4
#define POINT_LIGHTS_NUMBER                                32
#define SPOT_LIGHTS_NUMBER                                 8

in VS_OUT {
	vec3 fragmentPosition;
	vec3 normal;
	vec2 textureCoords;
	vec4 fragmentPositionDirectionalLightSpace[DIRECTIONAL_LIGHTS_NUMBER];
	vec4 fragmentPositionSpotLightSpace[SPOT_LIGHTS_NUMBER];
} fs_in;

struct Material {
    vec3      ambient;
	sampler2D diffuse;
	sampler2D specular;
    float     shininess;
}; 

struct DirectionalLight {
	vec3 position;
	vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

	vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	vec3  position;
	vec3  direction;
	float cutOff;
	float outerCutOff;

	vec3  ambient;
	vec3  diffuse;
	vec3  specular;

	float constant;
	float linear;
	float quadratic;
};

uniform bool             shadows;
uniform float            farPlane;
uniform Material         material;
uniform vec3             viewPosition;

uniform DirectionalLight directionalLights[DIRECTIONAL_LIGHTS_NUMBER];
uniform sampler2D        directionalLightFlatShadowMapArray[DIRECTIONAL_LIGHTS_NUMBER];
uniform int              sampledShadowOrdinalNumbers[DIRECTIONAL_LIGHTS_NUMBER];
uniform int              sampledDirectionalShadowOrdinalNumbersArraySize;
uniform int              directionalLightsArraySize;

uniform PointLight       pointLights[POINT_LIGHTS_NUMBER];
uniform samplerCube      pointLightCubeShadowMapArray[POINT_LIGHTS_NUMBER];
uniform int              pointLightCubeShadowMapComponentIndices[POINT_LIGHTS_NUMBER];
uniform int              sampledPointShadowOrdinalNumbersArraySize;
uniform int              pointLightsArraySize;

uniform SpotLight        spotLights[SPOT_LIGHTS_NUMBER];
uniform sampler2D        spotLightFlatShadowMapArray[SPOT_LIGHTS_NUMBER];
uniform int              spotLightFlatShadowMapComponentIndices[SPOT_LIGHTS_NUMBER];
uniform int              sampledSpotShadowOrdinalNumbersArraySize;
uniform int              spotLightArraySize;

//vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);
vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);
float ComputeDirectionalShadow(DirectionalLight light, vec4 fragmentPositionDirectionalLightSpace, sampler2D flatShadowMap);
float ComputePointShadow(PointLight light, vec3 fragmentPosition, samplerCube cubeShadowMap);
float ComputeSpotShadow(SpotLight light, vec4 fragmentPositionSpotLightSpace, sampler2D flatShadowMap);

void main()
{
	vec3 normal        = normalize(fs_in.normal);
	vec3 viewDirection = normalize(viewPosition - fs_in.fragmentPosition);
	
	vec3 result = vec3(0.0, 0.0, 0.0);
	// Compute directional lighting
	int directionalLightIndicesCounter    = 0;
	int directionalLightIndexAccumulator  = -1;
	if(sampledDirectionalShadowOrdinalNumbersArraySize > 0)
		directionalLightIndexAccumulator  = sampledShadowOrdinalNumbers[directionalLightIndicesCounter];

	float shadow = 0.0;
	for (int f = 0; f < directionalLightsArraySize; ++f) {
		if(f == directionalLightIndexAccumulator && directionalLightIndicesCounter != sampledDirectionalShadowOrdinalNumbersArraySize) {
			shadow = ComputeDirectionalShadow(directionalLights[f], fs_in.fragmentPositionDirectionalLightSpace[f],
			                                    directionalLightFlatShadowMapArray[f]);
			++directionalLightIndicesCounter;
			directionalLightIndexAccumulator = sampledShadowOrdinalNumbers[directionalLightIndicesCounter];
		}

		vec3 light  = ComputeDirectionalLight(directionalLights[f], normal, viewDirection);
		result += (1.0 - shadow) * light;
		if(shadow > 0.0)
			shadow = 0.0;
	}
	// Compute point lights
	int pointLightIndicesCounter    = 0;
	int pointLightIndexAccumulator  = -1;
	if(sampledPointShadowOrdinalNumbersArraySize > 0)
		pointLightIndexAccumulator  = pointLightCubeShadowMapComponentIndices[pointLightIndicesCounter];

	shadow = 0.0;
	for (int i = 0; i < pointLightsArraySize; ++i) {
		if(i == pointLightIndexAccumulator && pointLightIndicesCounter != sampledPointShadowOrdinalNumbersArraySize) {
			shadow = ComputePointShadow(pointLights[i], fs_in.fragmentPosition,
										pointLightCubeShadowMapArray[pointLightIndicesCounter]);
			++pointLightIndicesCounter;
			pointLightIndexAccumulator = pointLightCubeShadowMapComponentIndices[pointLightIndicesCounter];
		}
		vec3 light = ComputePointLight(pointLights[i], normal, fs_in.fragmentPosition, viewDirection);
		result += (1.0 - shadow) * light;
		if(shadow > 0.0)
			shadow = 0.0;
	}
	// Compute spot light
	int spotLightIndicesCounter    = 0;
	int spotLightIndexAccumulator  = -1;
//	int spotLightArraySize = spotLightSpaceMatrixArraySize;
//	int spotLightArraySize = 1;
	if(sampledSpotShadowOrdinalNumbersArraySize > 0)
		spotLightIndexAccumulator = spotLightFlatShadowMapComponentIndices[spotLightIndicesCounter];

	shadow = 0.0;
	for (int j = 0; j < spotLightArraySize; ++j) {
		vec3 lightDirection = normalize(spotLights[j].position - fs_in.fragmentPosition);
		float theta         = dot(lightDirection, normalize(-spotLights[j].direction));
		if(j == spotLightIndexAccumulator && spotLightIndicesCounter != sampledSpotShadowOrdinalNumbersArraySize) {    ///< DELETE GOVNO!!!!!!!
			shadow = ComputeSpotShadow(spotLights[j], fs_in.fragmentPositionSpotLightSpace[j],
										spotLightFlatShadowMapArray[spotLightIndicesCounter]);
			
			++spotLightIndicesCounter;
			spotLightIndexAccumulator = spotLightFlatShadowMapComponentIndices[spotLightIndicesCounter];
		}
		vec3 light = ComputeSpotLight(spotLights[j], normal, fs_in.fragmentPosition, viewDirection);
		result += (1.0 - shadow) * light;
		if(shadow > 0.0)
			shadow = 0.0;

//		result += ComputeSpotLight(spotLights[j], normal, fs_in.fragmentPosition, viewDirection);
	}

	// Compute shadow
	// float shadow = ComputeShadow(fs_in.fragmentPositionPointLightSpace);
	// result = result * (1.0 - shadow);


	// float depthValue = texture(spotLightFlatShadowMapArray[0], fs_in.textureCoords).r;
	// fragColor = vec4(vec3(depthValue), 1.0);
	fragColor = vec4(result, 1.0);
//	fragColor = vec4(spotLightArraySize, spotLightArraySize ,spotLightArraySize, 1.0);
}

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection) {
	vec3 lightDirection = normalize(light.direction);
	// diffuse shading
	float difference    = max(dot(normal, lightDirection), 0.0f);
	// specular shading
	vec3 reflectDirection   = reflect(-lightDirection, normal);
	float specularComponent = pow(max(dot(viewDirection, reflectDirection), 0.0f), material.shininess);
	// combine results
	vec3 ambient  = light.ambient * material.ambient;
	vec3 diffuse  = light.diffuse * difference * vec3(texture(material.diffuse, fs_in.textureCoords));
	vec3 specular = light.specular * specularComponent * vec3(texture(material.specular, fs_in.textureCoords));

	return (ambient + diffuse + specular);
}

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection) {
	vec3 lightDirection = normalize(light.position - fragmentPosition);
	// diffuse shading
	float difference    = max(dot(normal, lightDirection), 0.0f);
	// specular shading
	vec3 reflectDirection   = reflect(-lightDirection, normal);
	float specularComponent = pow(max(dot(viewDirection, reflectDirection), 0.0f), material.shininess);
	// attenuation
	float distance    = length(light.position - fragmentPosition);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// combine results
	vec3 ambient  = light.ambient * material.ambient;
	vec3 diffuse  = light.diffuse * difference * vec3(texture(material.diffuse, fs_in.textureCoords));
	vec3 specular = light.specular * specularComponent * vec3(texture(material.specular, fs_in.textureCoords));

	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection) {
	vec3 lightDirection = normalize(light.position - fragmentPosition);
	// diffuse shading
	float difference    = max(dot(normal, lightDirection), 0.0f);
	// specular shading
	vec3 reflectDirection   = reflect(-lightDirection, normal);
	float specularComponent = pow(max(dot(viewDirection, reflectDirection), 0.0f), material.shininess);
	// attenuation
	float distance    = length(light.position - fragmentPosition);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
	// spotlight intensity
    float theta     = dot(lightDirection, normalize(-light.direction));
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	// combine results
	vec3 ambient  = light.ambient * material.ambient;
	vec3 diffuse  = light.diffuse * difference * vec3(texture(material.diffuse, fs_in.textureCoords));
	vec3 specular = light.specular * specularComponent * vec3(texture(material.specular, fs_in.textureCoords));
//	ambient  *= attenuation * intensity;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;
	
    return (ambient + diffuse + specular);
}

float ComputeDirectionalShadow(DirectionalLight light, vec4 fragmentPositionDirectionalLightSpace, sampler2D flatShadowMap) {
	// Perform perspective devide
	vec3 projectiveCoordinates = fragmentPositionDirectionalLightSpace.xyz / fragmentPositionDirectionalLightSpace.w;
	// Transform to [0.1] range
	projectiveCoordinates      = projectiveCoordinates * 0.5 + 0.5;
	// Get closest depth value from light's perspective (using [0,1] range fragmentPositionLight as coordinates)
	float closestDepth         = texture(flatShadowMap, projectiveCoordinates.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth         = projectiveCoordinates.z;
	// Check whether current fragment position is in shadow
	vec3 normal = normalize(fs_in.normal);
//	vec3 lightDir = normalize(lightPos - fs_in.fragmentPositionPointLightSpace.xyz);
	vec3 lightDir = normalize(light.position - fs_in.fragmentPosition);
	float bias                 = max(0.01 * (1.0 - dot(normal, lightDir)), 0.005);
//	float shadow               = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(flatShadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(flatShadowMap, projectiveCoordinates.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 0.5 : 0.0;
		}
	}
	shadow /= 9.0;
	
	if (projectiveCoordinates.z > 1.0)
		shadow = 0.0;

	// if (projectiveCoordinates.x > 1.0 || projectiveCoordinates.x < -1.0)
	// 	shadow = 0.0;

	// if (projectiveCoordinates.y > 1.0 || projectiveCoordinates.y < -1.0)
	// 	shadow = 0.0;
		
	return shadow;
}

float ComputePointShadow(PointLight light, vec3 fragmentPosition, samplerCube cubeShadowMap) {
	// Get vector between fragment position and light position
	vec3 fragmentToLight       = fragmentPosition - light.position;
	// // Get the fragment to light vector to sample from the shadow map
	// float closestDepth         = texture(cubeShadowMap, fragmentToLight).r;
	// // It is currently in linear range between [0,1], let's re-transform it back to original depth value
	// closestDepth              *= farPlane;
	// Now get current linear depth as the length between the fragment and light position
	float currentDepth         = length(fragmentToLight);
	// Test for shadows simple
	// float bias                 = 0.05;
	// float shadow               = currentDepth - bias > closestDepth ? 0.5 : 0.0;

	// Test for shadows full solid cube of offsets
	// float bias    = 0.05;
	// float shadow  = 0.0;
	// float samples = 4.0;
	// float offset  = 0.1;
	// for(float x = -offset; x < offset; x += offset / (samples * 0.5))
	// 	{
	// 		for(float y = -offset; y < offset; y += offset / (samples * 0.5))
	// 			{
	// 				for(float z = -offset; z < offset; z += offset / (samples * 0.5))
	// 					{
	// 						float closestDepth = texture(cubeShadowMap, fragmentToLight + vec3(x, y, z)).r;
	// 						closestDepth *= farPlane; ///< undo mapping [0;1]
	// 						if(currentDepth - bias > closestDepth)
	// 							shadow += 1.0;
	// 					}
	// 			}
	// 	}
	// shadow /= (samples * samples * samples);

	// Test for shadows ranged cube of offsets
	vec3 sampleOffsetDirections[20] = vec3[]
		(
			vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
			vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
			vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
			vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
			vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
		);  
	
	float shadow  = 0.0;
	float bias    = 0.15;
	float samples = 20;
	float viewDistance = length(viewPosition - fragmentPosition);
	float diskRadius   = (1.0 + (viewDistance / farPlane)) / 25.0;
	for(int i = 0; i < samples; ++i)
		{
			float closestDepth = texture(cubeShadowMap, fragmentToLight + sampleOffsetDirections[i] *
										 diskRadius).r;
			closestDepth *= farPlane; // undo mapping [0;1]
			if(currentDepth - bias > closestDepth)
				shadow += 1.0;
		}
	shadow /= float(samples);

//	return closestDepth;
	return shadow;
}

float ComputeSpotShadow(SpotLight light, vec4 fragmentPositionSpotLightSpace, sampler2D flatShadowMap) {
	// Perform perspective devide
	vec3 projectiveCoordinates = fragmentPositionSpotLightSpace.xyz / fragmentPositionSpotLightSpace.w;
	// Transform to [0.1] range
	projectiveCoordinates      = projectiveCoordinates * 0.5 + 0.5;
	// Get closest depth value from light's perspective (using [0,1] range fragmentPositionLight as coordinates)
	float closestDepth         = texture(flatShadowMap, projectiveCoordinates.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth         = projectiveCoordinates.z;
	// Check whether current fragment position is in shadow
	vec3 normal = normalize(fs_in.normal);
//	vec3 lightDir = normalize(lightPos - fs_in.fragmentPositionPointLightSpace.xyz);
	vec3 lightDir = normalize(light.position - fs_in.fragmentPosition);
	float bias                 = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
//	float shadow               = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(flatShadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(flatShadowMap, projectiveCoordinates.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 0.5 : 0.0;
		}
	}
	shadow /= 9.0;
	
	if (projectiveCoordinates.z > 1.0)
		shadow = 0.0;

	// if (projectiveCoordinates.x > 1.0 || projectiveCoordinates.x < -1.0)
	// 	shadow = 0.0;

	// if (projectiveCoordinates.y > 1.0 || projectiveCoordinates.y < -1.0)
	// 	shadow = 0.0;
		
	return shadow;
}
