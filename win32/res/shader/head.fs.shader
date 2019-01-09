//	frag
#version 330 core
uniform mat4 mvp_;
uniform mat4 mv_;
uniform sampler2D texture_;
uniform sampler2D normal_;

in vec3 vpos_;
in vec2 vuv_;

out vec4 color_;

