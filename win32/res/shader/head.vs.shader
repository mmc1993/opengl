//	vertex
#version 410 core
layout(location = 0) in vec3 pos_;
layout(location = 1) in vec2 uv_;

uniform mat4 mvp_;
uniform mat4 mv_;
uniform sampler2D texture_;
uniform sampler2D normal_;

out vec3 vpos_;
out vec2 vuv_;

