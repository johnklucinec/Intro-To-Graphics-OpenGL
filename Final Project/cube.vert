#version 330 compatibility

uniform float shift;
out vec4 vertexColor;

void main()
{
    vec4 vert = gl_Vertex;
    float scale = (gl_Color.a <= 0.5) ? 1.02 : 
                  (gl_Color.a < 1.0) ? 1.01 : 1.0;
    
    // Apply scale and translation
    vert.xyz = scale * vert.xyz - vec3(shift, 2.5, 2.5);
    
    // Calculate darkening factor based on position
    float darkening = 1.0 - (vert.z + vert.y + 5.0);
    darkening = clamp(darkening, 0.3, 1.0);
    
    // Only apply darkening to opaque blocks (alpha = 1.0)
    vec3 finalColor = (gl_Color.a == 1.0) ? gl_Color.rgb - darkening : gl_Color.rgb - darkening / 2;
    
    gl_Position = gl_ModelViewProjectionMatrix * vert;
    vertexColor = vec4(finalColor, gl_Color.a);
}
