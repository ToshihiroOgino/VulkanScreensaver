#!/bin/bash
echo 'Compiling shaders'
cd ./shaders
files="./*"
spv=.spv
rm ./*.spv
for file in $files; do
    echo Compiling $file ...
    glslc $file -o $file$spv
done
echo 'Shaders was compiled.'
