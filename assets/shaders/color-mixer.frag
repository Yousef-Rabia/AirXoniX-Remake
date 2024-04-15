#version 330 core

// This shader is designed to work with "triangle.vert" and it receives an
// interpolated varying which represents the vertex color.

in Varyings {
    vec3 color;
} fs_in;

out vec4 frag_color;
uniform vec4 red, green, blue;
// red(r,g,b,a), green(r,g,b,a), blue(r,g,b,a)
// fs_in.color=(r,g,b,a)


// currently the shader just returns the interpalated color varying.
// However, we want to mix the color channels around. We can do this using a 
// color matrix which we will send to the shader as 3 uniforms: red, green, blue.
// Each of these 3 variables will be a vec4. To apply the channel mixing for a
// certain channel (e.g. red), we apply this linear transformation:
// frag_color.r = red.r * fs_in.color.r + red.g + fs_in.color.g + red.b + fs_in.color.b + red.a; // probably there is a mistake
// However, this line is too long to write, so we can simplify it using a dot product
// (which is defined in the "dot" function).

//TODO: (Req 1) Finish this shader and apply the channel mixing using the "dot" function.

void main(){
    frag_color = vec4(
        dot(vec4(fs_in.color,1.0), red),
        dot(vec4(fs_in.color,1.0), green),
        dot(vec4(fs_in.color,1.0), blue),
        1.0
    );
}