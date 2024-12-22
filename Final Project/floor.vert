#version 330 compatibility

uniform int shift;
uniform int width;
out vec4 vertexColor;

void main()
{
    vec4 vert = gl_Vertex;
    
    // Apply shift
    vert.x += float(shift);
    vert.y -= 3;
    vert.z -= 2;
    
    // Calculate which platform and position within platform
    int platformNum = gl_InstanceID / (width * width);
    int instanceInPlatform = gl_InstanceID % (width * width);
    
    // Calculate grid position within platform
    int row = instanceInPlatform / width;
    int col = instanceInPlatform % width;
    
    // Offset vertex position
    vert.x += float(col);
    vert.z += float(row);
    
    gl_Position = gl_ModelViewProjectionMatrix * vert;

    // Alternate between passed color and grey
    if ((row + col) % 2 == 0) {
        vertexColor = gl_Color;
    } else {
        vertexColor = vec4(0.5, 0.5, 0.5, 1.0);
    }
}
