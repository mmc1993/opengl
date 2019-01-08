//	vertex
#version 330 core
uniform mat4 mvp_;
uniform mat4 mv_;
uniform mat4 m_;
uniform sampler2D normal_;
uniform sampler2D texture_;

layout (localtion = 0) in vec3 pos_;
layout (localtion = 1) in vec2 uv_;

out vec3 vpos_;
out vec2 vuv_;

