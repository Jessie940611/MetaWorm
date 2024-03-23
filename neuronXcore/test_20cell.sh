#!/bin/bash

Data_Dir=/home/malei/workspace/github/neuronX/data/striatum_20cell/

Viewer=/home/malei/workspace/github/neuronX/neuronXcore/build/neuronXcore_multi_gpu

$Viewer -win 1280 720 -bg 0.161 -circle 140  -circleDist 0.83 -user $Data_Dir   --camera-focal-distance 0.85  --camera-lens-radius 0.0

