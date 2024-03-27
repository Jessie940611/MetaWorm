# MetaWorm

MetaWorm is an integrated datadriven model of *C. elegans*, linking brain, body and environment to faithfully replicate *C. elegans* locomotion behavior. A paper introducing MetaWorm is available at https://www.biorxiv.org/content/10.1101/2024.02.22.581686v2.

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

We recommend users to install and run this framework on Ubuntu. We tested on:    
- OS: Ubuntu 20.04    
- GPU: Nvidia 3090     
- CUDA: 11.4  
- Python 3.8.x  
- Nvidia Optix: 7.0.0

## Installation Guide
### C++ 
#### Basic C++ library
```
sudo apt-get update
sudo apt-get install build-essential cmake-curses-gui git
sudo apt-get install libeigen3-dev freeglut3-dev libtinyxml-dev libpython3-dev python3-numpy libopenmpi-dev
```
#### Boost 1.66 with Python3 : **Boost library should install from the source code!**  
- Download Boost 1.66 source code as zip file in [https://www.boost.org/users/history/version_1_66_0.html](https://www.boost.org/users/history/version_1_66_0.html).
- Unzip the file. ``` tar -xvf boost_1_66_0.tar.gz ```
- Compile and install the source code.
```
cd boost_1_66_0
sudo ./bootstrap.sh --with-python=python3
sudo ./b2 --with-python --with-filesystem --with-system install
```
#### Build
```
cd XXXX
mkdir build
cd build 
cmake ..
make -j8
```
### Python
install python packages
```
pip install -r requirements.txt
```
### create an nmodl mechanism library
```
cd MetaWorm/eworm/components/mechanism
nrnivmodl modfile
```

## Demo
This demo is the open-loop simulation of *C. elegans* movement.
```
./neuronXcore -data ../eworm/ghost_in_mesh_sim/data/tuned/video_offline/video_offline_neuronX
```

The closed-loop simulation of *C. elegans* movement.
```
./neuronXcore -data ../eworm/ghost_in_mesh_sim/data/tuned/video_online/video_online_neuronX
```

## Instructions for Use
### Modify <u>the neural network model</u> of MetaWorm
#### modify any parameters of the model in the files
```
├── components
│   ├── mechanism      # ion channel models
│   ├── model          # cell models, including morphology and locations of cells
│   ├── param
│   │   ├── cell       # biophysical parameters of cells
│   │   ├── connection # adjacency matrix
├── network
│   ├── config.json    # network config
```

#### fitting the neural network data
The eworm_learn file contains code to training the neural network model to fit the target data. The target data can be Preason Correlation Matrix of neurons' membrane potentials, or the Calcium signals of neurons. This training algorithm need GPU to run. And you can use multiple GPUs to run.   
   
(If you add or change a ion channel model X, you need to write the correspoding X_lr.mod file in /MetaWorm/eworm_learn/components/mechanism/modfile/ for training)  

Create an nmodl mechanism library
```
cd /MetaWorm/eworm_learn
nrnivmodl components/mechanism/modfile
```
Train the model
```
./x86_64/special run_eworm_v4.py
```
Before training, you should set the parameters in run_eworm_v4.py
```
TARGET_MODE: 'corr' - fit the correlation matrix of neurons' membrane potentials; 'traces' - fit neurons' membrane potentials
PERCISE: True - more precise, but costs more memory and time; False - less precise, but save memory and time
ADAM: True - used Adam as optimizaer，False - SGD
ngpu: num of GPUs
K_nblock: 

```
### Modify the interaction between <u> the neural network model</u> and <u>the body & environment model</u>

#### train the neural network to control the body

#### run the simulation of *C. elegans* movement

### 3D User Interface Interaction

#### Mouse

Rotatation with mouse right key, and zoom with mouse middle key.

#### Keyboard
space : Play & Pause.   
'r' : reset worm.   
't' : tetrahedron mesh of worm body(FEM).   
'w' : worm body.   
'm' : worm muscles(96 muscles).   
'p' : path of swimming trajectory.   
'a' : arrow of worm swimming direction (yellow arrow)   
'x' : coordinate of world. x-red arrow. y-green arrow. z-blue arrow.   
'c' : coordinate of worm body & head. x-red arrow. y-green arrow. z-blue arrow.   
'q' : exit   

## License
This project is covered under the Apache 2.0 License.   
[BAAI](https://www.baai.ac.cn/)
