varying vec4 ecPosition; 
varying vec3 ecPosition3; 
varying vec3 normal;
varying vec4 ambientGlobal;
varying vec4 vertColor;
varying float depthValue;

uniform sampler2D displacementMap;
uniform bool shaderDisplacement;
uniform sampler2D normalMap;
uniform float depthScale;
uniform float depthOffset;

vec3 calcNormal(sampler2D depth_map, vec2 coords)
{
    vec4 p1 = texture2D(depth_map, coords.xy);
    vec4 p2 = texture2D(depth_map, vec2(coords.x, coords.y + (1.0/480.0) ) );
    vec4 p3 = texture2D(depth_map, vec2(coords.x + (1.0/640.0), coords.y));
    
    vec3 dx = vec3(1, 0, p1 - p2);
    vec3 dy = vec3(0, 1, p1 - p3);
    
    vec3 normal = cross(dx, dy);
    
    return normal;
}

void main()
{
	
	vec4 position = gl_Vertex;
	
	vec2 depthUV = vec2( gl_MultiTexCoord0.x, 1.0 - gl_MultiTexCoord0.y );
	vec4 dv = texture2D( displacementMap, depthUV );
	depthValue = 1.0 - dv.r;
	position.z += dv.r * depthScale - depthOffset;
	
	ecPosition		= gl_ModelViewMatrix * position; 
	ecPosition3		= vec3( ecPosition ) / ecPosition.w;
	
	if (shaderDisplacement) {
//		vec3 normalLookup = texture2D( normalMap, gl_MultiTexCoord0.xy ).xzy;
		vec3 normalLookup = calcNormal( normalMap, gl_MultiTexCoord0.xy );
		normalLookup -= 0.5;
		normalLookup *= 2.0;
//		normalLookup = normalize(normalLookup);
		normal = gl_NormalMatrix * normalLookup;
		
	} else {
		normal = gl_NormalMatrix * gl_Normal;
	}
//	normal			= gl_NormalMatrix * (shaderDisplacement ? texture2D( normalMap, gl_MultiTexCoord0.xy ).xyz : gl_Normal);
//	normal			= vec3(0.0, 0.0, 1.0);
	gl_Position		= gl_ModelViewProjectionMatrix * position;
	
	ambientGlobal	= gl_LightModel.ambient * gl_FrontMaterial.ambient;
	
	vertColor		= gl_Color;
	
	gl_TexCoord[0] = vec4(gl_MultiTexCoord0.xy, 0.0, 1.0);
}