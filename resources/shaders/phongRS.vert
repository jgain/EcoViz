#version 410
#extension GL_ARB_explicit_attrib_location: enable

// vertex shader: basicShader; simple Phong Model lighting

layout (location=0) in vec3 vertex;
layout (location=1) in vec2 UV;
layout (location=2) in vec3 vertexNormal;
layout (location=3) in vec3 instTransl;
layout (location=4) in vec2 instScale;
layout (location=5) in float csoffset;

// transformations
uniform mat4 MV; // model-view mx
uniform mat4 MVproj; //model-view-projection mx
uniform mat3 normMx; // normal matrix

//colours and material
uniform vec4 matDiffuse;
uniform vec4 matAmbient;
uniform vec4 lightpos; // in camera space
uniform vec4 diffuseCol;
uniform vec4 ambientCol;

// per pixel values to be computed in fragment shader
out vec3 normal; // vertex normal
out vec3 lightDir; // toLight
out vec3 halfVector;
out vec4 diffuse;
out vec4 ambient;

out vec2 texCoord;

void main(void)
{
    vec3 inNormal, v;
    vec4 coloff = vec4(-0.15+csoffset, -0.15+csoffset, -0.15+csoffset, 0.0);
 
    mat4 instanceTransf = mat4(0.0);

    instanceTransf[0][0] = instScale.x;
    instanceTransf[1][1] = instScale.y;
    instanceTransf[2][2] = instScale.x;
    instanceTransf[3] = vec4(instTransl, 1.0);

    texCoord = UV;
    v = vertex;
    inNormal = vertexNormal;

    // map to camera space for lighting etc
    normal = normalize(normMx * inNormal);

    // vertex in camera coords
    vec4 ecPos = MV * vec4(v, 1.0);

    lightDir  = normalize(lightpos.xyz - ecPos.xyz);
    halfVector = normalize(normalize(-ecPos.xyz) + lightDir);

    diffuse = matDiffuse * clamp(diffuseCol+coloff, 0.0, 1.0);
    // diffuse = vec4(1.0, 0.0, 0.0, 1.0);
    ambient = matAmbient * clamp(ambientCol+coloff,0.0, 1.0);

    gl_Position = MVproj * instanceTransf * vec4(v, 1.0); // clip space position
}
