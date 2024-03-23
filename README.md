# MetaWorm

MetaWorm is an integrated datadriven model of *C. elegans*, linking brain, body and environment to faithfully replicate *C. elegans* locomotion behavior.

eworm: the neural network model  
MetaWorm: the body & environment model  
neuroXcore: the 3D visualization for the neural network model  

## Table of Contents
* [Background](#background)  
* [System Requirements](#system-requirements)  
* [Installation Guide](#installation-guide)  
* [Demo](#demo)  
* [Instructions for Use](#instructions-for-use)  
* [License](#license)  

## Background
The behavior of an organism is profoundly influenced by the complex interplay between its brain, body, and environment. Existing data-driven models focusing on either the brain or the body-environment separately. A model that integrates these two components is yet to be developed. Here, we present MetaWorm, an integrative data-driven model of a widely studied organism, *C. elegans*. This model consists of two sub-models: the brain model and the body & environment model. The brain model was built by multi-compartment models with realistic morphology, connectome, and neural population dynamics based on experimental data. Simultaneously, the body & environment model employed a lifelike body and a 3D physical environment, facilitating easy behavior quantification. Through the closed-loop interaction between two sub-models, MetaWorm faithfully reproduced the realistic zigzag movement towards attractors observed in *C. elegans*. Notably, MetaWorm is the first model to achieve seamless integration of detailed brain, body, and environment simulations, enabling unprecedented insights into the intricate relationships between neural structures, neural activities, and behaviors. Leveraging this model, we investigated the impact of neural system structure on both neural activities and behaviors. Consequently, MetaWorm can enhance our understanding of how the brain controls the body to interact with its surrounding environment. 

## System Requirements

### OS requirements
Linux: Ubuntu (18.04.1)  
### C++
```
build-essential  
cmake-curses-gui  
libeigen3-dev  
freeglut3-dev  
libtinyxml-dev  
libpython3-dev  
python3-numpy  
libopenmpi-dev  
```
### Python ~= 3.8
```
xlrd~=1.2.0  
numpy~=1.21.5  
matplotlib~=3.4.3  
scipy~=1.6.2  
setuptools~=52.0.0  
tqdm~=4.64.0  
NEURON==8.0  
seaborn  
pymp-pypi  
networkx  
torchvision==0.10.0  
tensorflow  
mpi4py  
OpenCV-Python  
```
## Installation Guide
### C++
Basic C++ library
```
sudo apt-get update
sudo apt-get install build-essential cmake-curses-gui git
sudo apt-get install libeigen3-dev freeglut3-dev libtinyxml-dev libpython3-dev python3-numpy libopenmpi-dev
```
Boost 1.66 with Python3 : Boost library should install from the source code!  
Download Boost 1.66 source code as zip file in https://www.boost.org/users/history/version_1_66_0.html.  
Unzip the file. tar -xvf boost_1_66_0.tar.gz  
Compile and install the source code.  
```
cd boost_1_66_0
sudo ./bootstrap.sh --with-python=python3
sudo ./b2 --with-python --with-filesystem --with-system install
```
### Python
Virtual environment
```
sudo apt-get install python3-pip
sudo pip3 install virtualenv
virtualenv venv
source venv/bin/activate
```
Python library with pip install
```
pip install -r requirements.txt
```
Install this repository
```
git clone https://github.com/Jessie940611/MetaWorm.git
```
Build
```
cd MetaWorm
mkdir build
cd build 
cmake ..
make -j8
```

## Demo
This demo is the open-loop simulation of *C. elegans* movement.
```
./neuronXcore -data ../eworm/ghost_in_mesh_sim/data/tuned/video_offline/video_offline_neuronX
```

## Instructions for Use
The closed-loop simulation of *C. elegans* movement.
```
./neuronXcore -data ../eworm/ghost_in_mesh_sim/data/tuned/video_online/video_online_neuronX
```
The closed-loop simulation of six *C. elegans* movement.
```

```
## License
[BAAI](https://www.baai.ac.cn/)
