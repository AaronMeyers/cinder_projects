void main(void) 
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
    vec4 wpos = vec4( gl_Vertex.xyz, 1.0 );
    vec4 epos = gl_ModelViewMatrix * wpos;
    gl_Position = gl_ProjectionMatrix * epos;
}