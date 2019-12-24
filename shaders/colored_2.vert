layout(location=0) in vec3 a_position;
in vec3 a_normal;
in vec3 a_color;

uniform mat4 u_modelMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_camProjection;
uniform mat4 u_lightMVP[3];

out vec2 v_texCoord;
out vec3 v_color;
out vec3 v_vertexPos;
out vec4 v_vertexPosLight[3];
out vec3 v_normal;
out vec3 v_ssaoNormal;

void main()
{
    vec4 tmp = u_modelViewMatrix * vec4(a_position, 1);
    gl_Position = u_camProjection * tmp;
    v_color = a_color;

    v_normal = normalize(mat3(u_modelMatrix) * a_normal);
    v_ssaoNormal = normalize(mat3(u_modelViewMatrix) * a_normal);
    v_vertexPos = tmp.xyz;
    for (int i=0; i<3; ++i)
    v_vertexPosLight[i] = u_lightMVP[i] * vec4(a_position, 1);
}
