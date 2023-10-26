#version 330

#extension GL_ARB_explicit_attrib_location: enable

uniform vec4 specularCol;
uniform float shiny;
uniform vec4 matSpec;

uniform sampler2D decalTexture;
uniform int useTexturing;

uniform float manipAlpha;

layout (location = 0) out vec4 color;

in vec3 normal;
in vec3 halfVector;
in vec3 lightDir;
in vec4 diffuse;
in vec4 ambient;

in vec2 texCoord;

// NOTE: this shader does not compute a distance attentuation term for lighting.
// some more variables need to be passed in for that.

void main(void)
{

    vec3 n, halfV,viewV,ldir;
    float NdotL,NdotHV;
    vec3 c = ambient.rgb;

    n = normalize(normal);

    //compute the dot product between normal and normalized lightdir

    NdotL = max(dot(n, normalize(lightDir)), 0.0);

    if (NdotL > 0.0) {

        c += diffuse.rgb * NdotL; // + ambient;

        halfV = normalize(halfVector);
        NdotHV = max(dot(n,halfV),0.0);
        c += matSpec.rgb * specularCol.rgb * pow(NdotHV, shiny);

     }
     // decal texturing:
     if (useTexturing == 1)
     {
        vec4 texel = texture(decalTexture, texCoord);

        c = mix(c.rgb, texel.rgb, texel.a); // GL_DECAL
     }

     color = vec4(c, 1.0); 
     // color.a = manipAlpha; // override this for all produced fragments
}
