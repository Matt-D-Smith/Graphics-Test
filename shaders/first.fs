#version 330

in vec3 fragPosition;

// Output fragment color
out vec4 finalColor;

void main()
{

    // Calculate final fragment color
    finalColor = vec4(fragPosition.xyz, 1.0);
}