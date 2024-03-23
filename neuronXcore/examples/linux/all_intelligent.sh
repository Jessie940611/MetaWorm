#!/bin/bash

Viewer=../data/linux/bin/neuronXcore

Data_Dir=../data

Material=../data/materials/neuron1.txt

LD_Libraries=../data/linux/lib



export LD_LIBRARY_PATH=$LD_Libraries:$LD_LIBRARY_PATH

$Viewer -win 1280 720 -bg 0.1 -data $Data_Dir -mat $Material 

