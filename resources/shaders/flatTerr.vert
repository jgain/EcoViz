#version 410
#extension GL_ARB_explicit_attrib_location: enable

// vertex shader: flat shade, two-sides (for terrain transect - NOTE: must turn OFF face culling)

layout (location=0) in vec3 vertex;
layout (location=1) in vec2 UV;

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

uniform sampler2D normalMap;
uniform sampler2D htMap;

uniform int drawWalls; // if 0, draw terrain, else drawing walls
uniform vec3 normalWall; // if drawing wall vertices, use this normal

out vec4 colour;

void main(void)
{
    vec3 inNormal, v, normal;
    vec3 lightDir;
    float NdotL;

    v = vertex;

    // lookup normal in texture
    if (drawWalls == 0) // drawing ht field - lookup normals
    {
        inNormal = texture(normalMap, UV).xyz;
        // correct vertex position from heigt map...
        v.y = texture(htMap, UV).r;

    }
    else
    {
        inNormal = normalWall; // use fixed normal for all wall vertices
        if (UV.s < 0.0) // base of wall: leave at z=0.0
            v.y = 0.0;
        else
            v.y = texture(htMap, UV).r;

    }


    // map to camera space for lighting etc
    normal = normalize(normMx * inNormal);


    // vertex in camera coords
    vec4 ecPos = MV * vec4(v, 1.0);

    lightDir  = normalize(lightpos.xyz - ecPos.xyz);

    NdotL = dot(normal, lightDir);
    if (NdotL < 0.0) NdotL = -NdotL;

    vec4 diffuse = 0.5*matDiffuse * diffuseCol * NdotL;
    vec4 ambient = 0.5*matAmbient * ambientCol;

    // colour = clamp(diffuse + ambient, 0.0, 1.0);
    colour  = clamp(0.5*matAmbient*ambientCol + 0.5*matDiffuse*diffuseCol, 0.0, 1.0);

    gl_Position = MVproj * vec4(v, 1.0); // clip space position
}
