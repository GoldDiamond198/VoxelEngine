#version 450
layout(location = 0) in vec3 inPos;
void main() {
    // Simply pass the position through
    gl_Position = vec4(inPos, 1.0);
}
