uniform sampler2D tex0;
uniform float texelWidth;
uniform float normalStrength;

void main(void)
{  
    float tl = abs(texture2D(tex0, gl_TexCoord[0].st + texelWidth * vec2(-1.0, -1.0)).x);   // top left
    float  l = abs(texture2D(tex0, gl_TexCoord[0].st + texelWidth * vec2(-1.0,  0.0)).x);   // left
    float bl = abs(texture2D(tex0, gl_TexCoord[0].st + texelWidth * vec2(-1.0,  1.0)).x);   // bottom left
    float  t = abs(texture2D(tex0, gl_TexCoord[0].st + texelWidth * vec2( 0.0, -1.0)).x);   // top
    float  b = abs(texture2D(tex0, gl_TexCoord[0].st + texelWidth * vec2( 0.0,  1.0)).x);   // bottom
    float tr = abs(texture2D(tex0, gl_TexCoord[0].st + texelWidth * vec2( 1.0, -1.0)).x);   // top right
    float  r = abs(texture2D(tex0, gl_TexCoord[0].st + texelWidth * vec2( 1.0,  0.0)).x);   // right
    float br = abs(texture2D(tex0, gl_TexCoord[0].st + texelWidth * vec2( 1.0,  1.0)).x);   // bottom right
   
   // Compute dx using Sobel:

    //           -1 0 1 

    //           -2 0 2

    //           -1 0 1

    float dX = tr + 2.0*r + br -tl - 2.0*l - bl;



    // Compute dy using Sobel:

    //           -1 -2 -1 

    //            0  0  0

    //            1  2  1

    float dY = bl + 2.0*b + br -tl - 2.0*t - tr;

    vec4 N = vec4(normalize(vec3(dX, 1.0 / normalStrength, dY)), 1.0);
  
    N *= 0.5;
    N += 0.5;

   gl_FragColor = N;
   //gl_FragData[0] = N;
}