layout(binding=1) uniform sampler2D u_csmVsm[3];
layout(location=10) uniform float u_lightAmbient;

#include "csm_interface.glsl"

struct Light {
    vec3 position;
    float brightness;
    float fadeDistance;
};

readonly layout(std430, binding=3) buffer b_lights {
    Light lights[];
};

float shadow_map_multiplier(in vec3 normal, in float shadow)
{
    int cascadeIdx = 0;
    while (cascadeIdx < u_csmSplits.length()-1 && -gpi.vertexPos.z > -u_csmSplits[cascadeIdx]) {
        ++cascadeIdx;
    }

    vec3 projCoords = gpi.vertexPosLight[cascadeIdx].xyz / gpi.vertexPosLight[cascadeIdx].w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0) {
        return 1.0;
    }

    float cosTheta = abs(dot(normalize(normal), normalize(u_csmLightDir)));
    const float BiasExceedOne = sqrt(0.5);
    float bias = cosTheta > BiasExceedOne ? sqrt(1-cosTheta*cosTheta) / cosTheta : 1;
    currentDepth -= 0.01*bias;

    vec2 moments = texture(u_csmVsm[cascadeIdx], projCoords.xy).xy;
    if (currentDepth < moments.x) {
        return 1.0;
    }

    const float ShadowBias = 0.005;
    float variance = max(moments.y - moments.x * moments.x, ShadowBias);
    float mD = moments.x - currentDepth;
    float pMax = variance / (variance + mD * mD);

    // light bleeding
    const float BleedBias = 0.05;
    pMax = clamp((pMax - BleedBias) / (1.0 - BleedBias), 0.0, 1.0);

    return mix(shadow, 1.0, pMax);
}

float shadow_map_multiplier(in vec3 normal) {
    return shadow_map_multiplier(normal, 0.3);
}

/*
 * @param normal is the surface normal in world space
 * @param pos is the surface position in world space
 */
float calc_positional_lighting(in vec3 normal, in vec3 pos, in float n)
{
    if (lights.length() <= 0 || normal == vec3(0))
    {
        return u_lightAmbient;
    }

    normal = normalize(normal);
    float sum = u_lightAmbient;
    for (int i=0; i<lights.length(); ++i)
    {
        vec3 d = pos - lights[i].position;
        float intensity = lights[i].brightness / (1 + length(d)/lights[i].fadeDistance);
        vec3 light_dir = normalize(d);
        sum += pow(intensity * max(dot(light_dir, normal), 0), n);
    }

    return sum;
}

float calc_positional_lighting(in vec3 normal, in vec3 pos)
{
    return calc_positional_lighting(normal, pos, 1);
}
