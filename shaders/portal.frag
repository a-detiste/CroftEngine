#include "portal_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

layout(location=0) out vec2 out_perturb;

mat2 rotate2d(in float a){
    float ca = cos(a);
    float sa = sin(a);
    return mat2(ca, -sa, sa, ca);
}

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
    (c - a) * u.y * (1.0 - u.x) +
    (d - b) * u.x * u.y;
}

float fbm(in vec2 st) {
    float value = 0.0;
    float amplitude = .5;
    for (int i = 0; i < 6; i++) {
        value += amplitude * noise(st);
        st *= 1.4;
        amplitude *= .5;
    }
    return value;
}
const float TimeMult = 0.0005;
const float TexScale = 2048;

float bumpTex(in vec2 uv) {
    const float PI = 3.14159265359;
    vec2 coords1 = rotate2d(.9*PI) * uv + u_time*vec2(0.1, -0.3)*TimeMult;
    vec2 coords2 = rotate2d(.06*PI) * uv - u_time*vec2(0.1, 0.2)*TimeMult;

    float wave1 = fbm(coords1*vec2(30.0, 20.0));
    float wave2 = fbm(coords2*vec2(30.0, 20.0));

    return pow((wave1 + wave2) / 2.0, 2.0);
}


float bumpFunc(in vec2 st){
    return bumpTex(st + vec2(bumpTex(st)*0.11, 0));
}

vec2 bumpMap(in vec2 st){
    const float eps = 2./TexScale;
    vec2 ff = vec2(
    bumpFunc(st-vec2(eps, 0)),
    bumpFunc(st-vec2(0, eps))
    );

    return (ff-vec2(bumpFunc(st)))/eps*0.004;
}

void main()
{
    vec2 uv = ppi.vertexPosWorld.xz / TexScale;
    vec2 bm = bumpMap(uv);
    vec3 sn = normalize(vec3(bm.x, 1, bm.y)); // normal in XZ plane (model space)

    const float IOR = 1.3;
    vec4 orig = u_viewProjection * vec4(ppi.vertexPosWorld, 1);
    orig /= orig.w;
    vec4 surface = u_viewProjection * vec4(vec3(sn.x, 0, sn.z) + ppi.vertexPosWorld, 1);
    surface /= surface.w;
    out_perturb = (surface-orig).xy;
}
