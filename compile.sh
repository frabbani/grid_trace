#!/bin/bash

clear
echo "compiling..."
gcc -std=c11 main.c vec.c array.c hash.c gc.c geom.c collide.c grid.c -o a.exe
echo "done!"
