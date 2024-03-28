# This part of code is forked and modified from https://github.com/seiing/SoftCon
Sehee Min, Jungdam Won, Seunghwan Lee, Jungnam Park, and Jehee Lee. 2019.
SoftCon: Simulation and Control of Soft-Bodied Animals with Biomimetic Actuators.
ACM Trans. Graph. 38, 6, 208. (SIGGRAPH Asia 2019)

## How to install

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

### Python

#### Python library with pip install
````
pip install numpy
pip install scipy
pip install matplotlib
pip install tensorflow
pip install mpi4py
pip install OpenCV-Python
````

### Interaction

Rotatation with mouse right key, and zoom with mouse middle key.

Keyboard:
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