#version 400 core
//------------------------------------------------------------------------------------------
// fragment shader, shadow map rendering
//------------------------------------------------------------------------------------------
uniform sampler2D objTex;
uniform bool hasObjTex;

//----------------------------------------------------------`--------------------------------
// in variables
in vec2 f_texCoord;
//----------------------------------------------------------`--------------------------------
// out variables
out vec4 fragColor;

//------------------------------------------------------------------------------------------
void main()
{
    /////////////////////////////////////////////////////////////////
    // output
    if(hasObjTex)
    {
        float alpha = texture(objTex, f_texCoord).w;
        if(alpha < 0.5f)
            discard;
    }
    fragColor = gl_FragCoord.z * vec4(1, 1, 1, 1);
}
