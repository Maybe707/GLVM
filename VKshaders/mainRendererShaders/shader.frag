#version 450

// layout(set = 1, binding = 1) uniform sampler2D texSampler;

// layout(location = 0) in vec3 fragColor;
// layout(location = 1) in vec3 fragmentNormal;
// layout(location = 2) in vec2 fragTexCoord;

// layout(location = 0) out vec4 outColor;

// void main() {
//     outColor = texture(texSampler, fragTexCoord);
// }

#extension GL_EXT_debug_printf : enable

#define DIRECTIONAL_LIGHTS_NUMBER                          2
#define POINT_LIGHTS_NUMBER                                2
#define SPOT_LIGHTS_NUMBER                                 2

layout(location = 0) in vec3 inFragmentPosition;
layout(location = 1) in vec3 inFragmentNormal;
layout(location = 2) in vec2 inFragmentTextureCoordinate;

layout(location = 5) in VS_OUT {
	vec3 fragmentPosition;
	vec3 normal;
	vec2 textureCoords;
	vec4 fragmentPositionDirectionalLightSpace[DIRECTIONAL_LIGHTS_NUMBER];
	vec4 fragmentPositionSpotLightSpace[SPOT_LIGHTS_NUMBER];

	vec3      ambient;
    float     shininess;
} fs_in;

layout(location = 0) out vec4 outColor;

// layout(set = 3, binding = 0) uniform MaterialUBO {
//     vec3      ambient;
//     float     shininess;
// } material; 

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

// #define DIRECTIONAL_LIGHTS_NUMBER                          4
// #define POINT_LIGHTS_NUMBER                                32
// #define SPOT_LIGHTS_NUMBER                                 8

layout(set = 1, binding = 0) uniform LightData {
	vec3 viewPosition;

	PointLight pointLightsArray[POINT_LIGHTS_NUMBER];
	int pointLightsArraySize;
	float farPlane;

	DirectionalLight directionalLightsArray[DIRECTIONAL_LIGHTS_NUMBER];
	int directionalLightsArraySize;
	
	SpotLight spotLightsArray[SPOT_LIGHTS_NUMBER];
	int spotLightArraySize;
} lightData;

// layout(set = 3, binding = 0) uniform DirectionalLightsUBO {

// } directionalLights;

// layout(set = 4, binding = 0) uniform PointLightsUBO {

// } pointLights;

// layout(set = 5, binding = 0) uniform SpotLightsUBO {

// } spotLights;

layout(set = 2, binding = 0) uniform sampler2D specular;

layout(set = 3, binding = 0) uniform sampler2D diffuse;
layout(set = 3, binding = 1) uniform sampler2D directionalLightsShadowMaps[DIRECTIONAL_LIGHTS_NUMBER];
layout(set = 3, binding = 5) uniform samplerCube pointLightsCubeShadowMaps[POINT_LIGHTS_NUMBER];
layout(set = 3, binding = 37) uniform sampler2D spotLightsShadowMaps[SPOT_LIGHTS_NUMBER];

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);
vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);

float ComputeDirectionalShadow(DirectionalLight light, vec4 fragmentPositionDirectionalLightSpace, sampler2D flatShadowMap);
float ComputePointShadow(PointLight light, vec3 fragmentPosition, samplerCube pointLightsCubeShadowMap);
float ComputeSpotShadow(SpotLight light, vec4 fragmentPositionSpotLightSpace, sampler2D flatShadowMap);

vec3 debugCubemapEquirectangular(samplerCube pointLightsCubeShadowMap)
{
    float pi = 3.14159f;
    vec2 uv = gl_FragCoord.xy/vec2(800, 400); // width and hight
 
    float alpha = 2.0f*pi * uv.x;
    float beta = pi*uv.y - pi*0.5f;
    float x = sin(beta) * cos(alpha);
    float y = sin(beta) * sin(alpha);
    float z = cos(beta);
 
    vec3 direction = vec3(x,y,z);
    // Output to screen
    return texture(pointLightsCubeShadowMap, direction).xyz; //iChannel0 your cube map texture
}
 
vec3 debugCubemapUnflatted(vec2 quadSize, samplerCube pointLightsCubeShadowMap)
{
 
    //   Y+
    //Z- X+ Z+ X-
    //   Y-
 
    float pi = 3.14159f;
 
    vec2 uv = mod(gl_FragCoord.xy,quadSize)/quadSize;
    ivec2 grid = ivec2(gl_FragCoord.xy/quadSize);
 
    uv.y *= -1.0;
    uv.y += 1.0;
 
 
    vec2 uv2 = (uv - vec2(0.5))*2.0f;
    float a = length(uv2);
    float rc = (1.0 - a)* sqrt(2.0f) + a;
 
    vec2 st = uv2* abs(rc);
    vec3 direction = vec3(0.0f); 
 
    if(grid.x == 1 && grid.y == 1)
    {
        direction = vec3(rc, -st.t, -st.s);
    }
 
    if(grid.x == 0 && grid.y == 1)
    {
        direction = vec3(-st.s, -st.t, -rc);
    }
    if(grid.x == 2 && grid.y == 1)
    {
        direction = vec3(st.s, -st.t, rc);
    }
    if(grid.x == 3 && grid.y == 1)
    {
        direction = vec3(-rc, -st.t, st.s);
    }
    if(grid.x == 1 && grid.y == 2)
    {
        direction = vec3(st.s, rc, st.t);
    }
    if(grid.x == 1 && grid.y == 0)
    {
        direction = vec3(st.s, -rc, -st.t);
    }
 
    return texture(pointLightsCubeShadowMap, direction).xyz;
}

float linearize_depth(float depth,float zNear,float zFar) {
    return zNear * zFar / (zFar + depth * (zNear - zFar));
}

float delinearize_depth(float depth, float near, float far) {
	return ((near * far) / depth - far) / (near - far);
}

void main()
{
	vec3 fragmentNormal = normalize(fs_in.normal);
	vec3 viewDirection  = normalize(lightData.viewPosition - fs_in.fragmentPosition);

	vec3 result = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < lightData.directionalLightsArraySize; ++i ) {
		vec3 light = ComputeDirectionalLight(lightData.directionalLightsArray[i], fragmentNormal, viewDirection);
		float shadow = ComputeDirectionalShadow(lightData.directionalLightsArray[i], fs_in.fragmentPositionDirectionalLightSpace[i], directionalLightsShadowMaps[i]);
		result += (1.0 - shadow) * light;
		if(shadow > 0.0)
			shadow = 0.0;
	}

	for(int i = 0; i < lightData.pointLightsArraySize; ++i) {
//		debugPrintfEXT("Quadratic value: %f", lightData.pointLightsArray[3].quadratic);
		
		vec3 light = ComputePointLight(lightData.pointLightsArray[i], fragmentNormal, inFragmentPosition, viewDirection);
		float shadow = ComputePointShadow(lightData.pointLightsArray[i],
										  inFragmentPosition, pointLightsCubeShadowMaps[i]);
		result += (1.0 - shadow) * light;

		// if (shadow == 1.0)
		// 	result = vec3(0.0, 0.0, 1.0);
		// else
		// 	result = vec3(1.0, 0.0, 0.0);
		
//		result += shadow;
		if(shadow > 0.0)
			shadow = 0.0;
	}

	for(int i = 0; i < lightData.spotLightArraySize; ++i) {
		vec3 light = ComputeSpotLight(lightData.spotLightsArray[i], fragmentNormal,
									  inFragmentPosition, viewDirection);
		float shadow = ComputeSpotShadow(lightData.spotLightsArray[i],
										 fs_in.fragmentPositionSpotLightSpace[i], spotLightsShadowMaps[i]);

		result += (1.0 - shadow) * light;
		if(shadow > 0.0)
			shadow = 0.0;
	}

//    float depthValue = texture(directionalLightsShadowMaps, inFragmentTextureCoordinate).r;
//x	float depthValue = texture(pointLightsCubeShadowMaps, vec3(inFragmentTextureCoordinate, 0)).r;
	
//	outColor = vec4(debugCubemapUnflatted(vec2(100, 100)), 1.0);
	outColor = vec4(result, 1.0);
//	outColor = vec4(vec3(depthValue), 1.0);
	
//	vec3 result = material.ambient * pointLights.pointLightsArray[0].diffuse;

	// vec3 lightDirection = normalize(pointLights.pointLightsArray[0].position - inFragmentPosition);
	// float differece = max(dot(fragmentNormal, lightDirection), 0.0);

	// vec3 diffuse = differece * pointLights.pointLightsArray[0].ambient;
	
	// outColor = vec4(diffuse, 1.0);
}

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection) {
	vec3 lightDirection = normalize(-light.direction);
	// diffuse shading
	float difference    = max(dot(normal, lightDirection), 0.0f);
	// specular shading
	vec3 reflectDirection   = reflect(-lightDirection, normal);
	float specularComponent = pow(max(dot(viewDirection, reflectDirection), 0.0f), fs_in.shininess);
	// combine results
	vec3 ambient  = light.ambient * fs_in.ambient;
	vec3 diffuse  = light.diffuse * difference * vec3(texture(diffuse, fs_in.textureCoords));
	vec3 specular = light.specular * specularComponent * vec3(texture(specular, fs_in.textureCoords));

	return (ambient + diffuse + specular);
}

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection) {
	vec3 lightDirection = normalize(light.position - fragmentPosition);
	// Diffuse shading
	float difference    = max(dot(normal, lightDirection), 0.0f);
	// specular shading
	vec3 reflectDirection   = reflect(-lightDirection, normal);
	float specularComponent = pow(max(dot(viewDirection, reflectDirection), 0.0f), fs_in.shininess);
	// attenuation
	float distance    = length(light.position - fragmentPosition);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// combine results
	vec3 ambient  = light.ambient * fs_in.ambient;
	vec3 diffuse  = light.diffuse * difference * vec3(texture(diffuse, inFragmentTextureCoordinate));
	vec3 specular = light.specular * specularComponent * vec3(texture(specular, inFragmentTextureCoordinate));

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
	float specularComponent = pow(max(dot(viewDirection, reflectDirection), 0.0f), fs_in.shininess);
	// attenuation
	float distance    = length(light.position - fragmentPosition);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
	// spotlight intensity
    float theta     = dot(lightDirection, normalize(-light.direction));
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	// combine results
	vec3 ambient  = light.ambient * fs_in.ambient;
	vec3 diffuse  = light.diffuse * difference * vec3(texture(diffuse, inFragmentTextureCoordinate));
	vec3 specular = light.specular * specularComponent * vec3(texture(specular, inFragmentTextureCoordinate));
	ambient  *= attenuation * intensity;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;
	
    return vec3(ambient + diffuse + specular);
}

float ComputeDirectionalShadow(DirectionalLight light, vec4 fragmentPositionDirectionalLightSpace, sampler2D flatShadowMap) {
	// Perform perspective devide
	vec3 projectiveCoordinates = fragmentPositionDirectionalLightSpace.xyz / fragmentPositionDirectionalLightSpace.w;
	// Transform to [0.1] range
	vec3 projectiveCoordinatesZO      = projectiveCoordinates * 0.5 + 0.5;
	// Get closest depth value from light's perspective (using [0,1] range fragmentPositionLight as coordinates)
//	float closestDepth         = texture(flatShadowMap, projectiveCoordinatesZO.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth         = projectiveCoordinates.z;
	// Check whether current fragment position is in shadow
	vec3 normal = normalize(fs_in.normal);
	vec3 lightDir = normalize(light.position - fragmentPositionDirectionalLightSpace.xyz);
//	vec3 lightDir = normalize(light.position - vec3(fs_in.fragmentPosition));
//	vec3 lightDir = normalize(vec3(fs_in.fragmentPosition) - light.position);
	float bias                 = max(0.01 * (1.0 - dot(normal, lightDir)), 0.005);
//	float shadow               = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(flatShadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(flatShadowMap, projectiveCoordinatesZO.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	
	if (projectiveCoordinatesZO.z > 1.0)
		shadow = 0.0;

	// if (projectiveCoordinates.x > 1.0 || projectiveCoordinates.x < -1.0)
	// 	shadow = 0.0;

	// if (projectiveCoordinates.y > 1.0 || projectiveCoordinates.y < -1.0)
	// 	shadow = 0.0;
		
	return shadow;
}

float ComputePointShadow(PointLight light, vec3 fragmentPosition, samplerCube pointLightsCubeShadowMap) {
	// Get vector between fragment position and light position
	vec3 fragmentToLight       = fragmentPosition - light.position;
	// // Get the fragment to light vector to sample from the shadow map
//	float closestDepth         = texture(pointLightsCubeShadowMaps, fragmentToLight).r;
//	closestDepth = linearize_depth(closestDepth, 1.0, 1000.0);
//	closestDepth               /= 100.0;
	// // It is currently in linear range between [0,1], let's re-transform it back to original depth value
	// closestDepth              *= farPlane;
	// Now get current linear depth as the length between the fragment and light position
	float currentDepth         = length(fragmentToLight);
	// currentDepth               -= 1.0;
	// currentDepth = delinearize_depth(currentDepth, 1.0, 100.0);
//	currentDepth               = currentDepth / 100.0;
	// Test for shadows simple
	// float bias                 = 10.0;
	// float shadow               = currentDepth - bias > closestDepth ? 1.0 : 0.0;

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
	// 						float closestDepth = texture(pointLightsCubeShadowMaps, fragmentToLight + vec3(x, y, z)).r;
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
//	float bias    = -0.12514;
	float samples = 20;
	float viewDistance = length(lightData.viewPosition - fragmentPosition);
	float diskRadius   = (1.0 + (viewDistance / lightData.farPlane)) / 25.0;
	for(int i = 0; i < samples; ++i)
		{
			float closestDepth = texture(pointLightsCubeShadowMap, fragmentToLight + sampleOffsetDirections[i] *
										 diskRadius).r;
//			closestDepth = linearize_depth(closestDepth, 1.0, 100.0);
			closestDepth *= lightData.farPlane;
			// bias = linearize_depth(bias, 1.0, 100.0);
			// closestDepth += 1.0;
			
//			closestDepth *= pointLights.farPlane; // undo mapping [0;1]
//			closestDepth *= 100.0; // undo mapping [0;1]
			if(currentDepth - bias > closestDepth)
				shadow += 1.0;
		}
	shadow /= float(samples);
//	shadow /= float(100);

//	return closestDepth;
	return shadow;
//	return currentDepth;
//	return abs(currentDepth - closestDepth);
}

float ComputeSpotShadow(SpotLight light, vec4 fragmentPositionSpotLightSpace, sampler2D flatShadowMap) {
	// Perform perspective devide
	vec3 projectiveCoordinates = fragmentPositionSpotLightSpace.xyz / fragmentPositionSpotLightSpace.w;
	// Transform to [0.1] range
	vec3 projectiveCoordinatesZO      = projectiveCoordinates * 0.5 + 0.5;
	// Get closest depth value from light's perspective (using [0,1] range fragmentPositionLight as coordinates)
//	float closestDepth         = texture(flatShadowMap, projectiveCoordinatesZO.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth         = projectiveCoordinates.z;
	// Check whether current fragment position is in shadow
	vec3 normal = normalize(fs_in.normal);
	vec3 lightDir = normalize(light.position - fragmentPositionSpotLightSpace.xyz);
//	vec3 lightDir = normalize(light.position - fs_in.fragmentPosition);
	float bias                 = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
//	float shadow               = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(flatShadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(flatShadowMap, projectiveCoordinatesZO.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	
	if (projectiveCoordinatesZO.z > 1.0)
		shadow = 0.0;

	// if (projectiveCoordinates.x > 1.0 || projectiveCoordinates.x < -1.0)
	// 	shadow = 0.0;

	// if (projectiveCoordinates.y > 1.0 || projectiveCoordinates.y < -1.0)
	// 	shadow = 0.0;
		
	return shadow;
}
