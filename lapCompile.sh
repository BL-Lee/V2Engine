#!/bin/sh -x

../glslc ../src/shaders/triangle.vert -o ../src/shaders/vert.spv
../glslc ../src/shaders/triangle.frag -o ../src/shaders/frag.spv
../glslc ../src/shaders/ray.comp -o ../src/shaders/ray.spv
../glslc ../src/shaders/rayProbe.comp -o ../src/shaders/rayProbe.spv
../glslc ../src/shaders/line.vert -o ../src/shaders/lineVert.spv
../glslc ../src/shaders/line.frag -o ../src/shaders/lineFrag.spv

