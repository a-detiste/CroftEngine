#include "time_uniform.glsl"

float rand1(in vec2 seed)
{
    const vec2 K1 = vec2(
    23.14069263277926, // e^pi (Gelfond's constant)
    2.665144142690225// 2^sqrt(2) (Gelfond–Schneider constant)
    );
    return fract(cos(dot(seed*u_time/40, K1)) * 12345.6789);
}

// get a random 2D-vector, each component within -1..1
vec2 rand2(in vec2 uv)
{
    float noise_x = clamp(fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    float noise_y = clamp(fract(sin(dot(uv, vec2(12.9898, 78.233)*2.0)) * 43758.5453), 0.0, 1.0)*2.0-1.0;
    return vec2(noise_x, noise_y);
}

vec3 shade_texel(in vec3 rgb, in float depth)
{
    return rgb * (1.0 - depth);
}

vec3 shaded_texel(in sampler2D tex, in vec2 uv, in float depth)
{
    return shade_texel(texture(tex, uv).rgb, depth);
}

float luminance(in vec3 color)
{
    return dot(color, vec3(0.212656, 0.715158, 0.072186));
}

float luminance(in vec4 color)
{
    return dot(vec3(color), vec3(0.212656, 0.715158, 0.072186));
}
