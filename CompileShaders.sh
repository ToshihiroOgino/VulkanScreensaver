#!/bin/bash
echo 'Compiling shaders'
glslc ./shaders/shader.vert -o ./shaders/vert.spv
glslc ./shaders/shader.frag -o ./shaders/frag.spv
echo 'Shaders was compiled.'
