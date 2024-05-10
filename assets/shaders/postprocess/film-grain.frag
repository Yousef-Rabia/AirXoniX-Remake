#version 330 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D tex; // The input texture containing the scene

const float grainIntensity = 0.1; // Constant intensity of the grain effect
const float grainSize = 1.0; // Constant size of individual grains

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec4 color = texture(tex, tex_coord);

    // Generate a random value for each pixel
    float randVal = rand(tex_coord * gl_FragCoord.xy);

    // Adjust the random value to control the intensity and size of the grain
    randVal = (randVal - 0.5) * grainIntensity * grainSize;

    // Add the grain to the pixel color
    color.xyz += vec3(randVal);

    frag_color = color;
}
