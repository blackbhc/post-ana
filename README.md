This library provide some programs and dynamic library to post analyze the galactic simulation data.
The main concentration of this library is the disk galaxy simulations.

---
# Documentation

## Dependencies
- `python3`:
    - `scons` for building: if you haven't installed `scons`, you can install it by `pip install scons`
    or `conda install scons`.
    - `numpy` for numerical calculation.
    - `h5py` for reading and writing HDF5 files.
- `make`
- a `C++` compiler for compiling: support C++11 or later, such as `g++` or `clang++`.
- `gsl` library for numerical calculation.
- `hdf5` library for reading and writing HDF5 files.

## Features
- [x] Rotation curves: support total and partial rotation curves (rotation curve of particular components).
- [x] CLI based parameters.
- [x] Parallel calculation (threading) to speed up the process.
- [x] Python wrapper for the library.
- [ ] Multiple snapshots.

## Installation
1. Clone the repository into your local directory:
```bash
git clone https://github.com/blackbhc/post-ana.git
```
2. Change the directory to the repository:
```bash
cd post-ana
```
3. Edit the installation path at the third line of `Makefile`, default is `~/.local`.
4. Edit the installation path at the third line of `Makefile`, default is `~/.local`.
5. Build the library: 
```bash
make
```
If you encounter any error, run `make clean` before you run `make` again.
(Edit the first line of `src/python.cpp`, if you encounter compiling error `Python.h not found`. The exact 
position of `Python.h` can be found by command `python3-config --includes`.)
6. Install the library: 
```bash
make install 
```

Uninstall:
```bash
make uninstall
```

## Usage
### Configure the environment 
Add the install path to your environment variable `PATH`: 
```bash
export PATH=$PATH:/path/to/install/bin
```

### As a program
For calculation of rotation velocities, run 
```bash
post [snapshot filename] [analysis result filename]
```

The snapshot file should be in HDF5 format, which follows the convention of the `Gadget` simulation IC file.

### Results
The program will generate a file with the name you specified. The file is in HDF5 format, and contains the 
rotation curve of the galaxy: one dataset for `Radius`, and another group `Rotation Velocity` for the rotation 
velocities of different components (namely `PartTypeX` in the snapshot file).

The unit of the results is in `km/s` for velocity and `kpc` for radius, if the snapshot file use the "standard"
unit system of `Gadget4` ($\rm{kpc}$, $\rm{Gyr}$, $1e10\ M_\odot$). Otherwise, the unit should be transformed
correspondingly.

### As a python library
The library is only a wrapper, which will call the program `post` to do the calculation. The python wrapper 
also provides some function to read the result file. At know this time, the python wrapper is not complete, 
so you need to add the path of the library before you import it. See the example below.

Basic example:
```python
import sys
sys.path.append('/path/to/post-ana') # modify the path to the install path
from post import single_snapshot # at now, this is the only class you can use
import os
os.environ["PATH"]+="path/to/install/bin" # modify the path to the install path
os.environ["LD_LIBRARY_PATH"]="/path/to/gsl/lib:/path/to/hdf5/lib" # modify the path to the gsl and hdf5 library

# create the object
analyzer = single_snapshot("/home/bhchen/FeGradient/Simulation/hr_sigma150/output/snapshot_000.hdf5")

# calculate the rotation curve before you do anything else
analyzer.cal_rc(r_max=30, r_bin_number=25, phi_bin_number=16)
print(analyzer.fdisk) # calculate the disk fraction and print it

# Plot the rotation curve
plt.figure(figsize=(10, 8))
for key in analyzer.rotation_velocities.keys():
    plt.plot(analyzer.radius, analyzer.rotation_velocities[key], label=key)
plt.xlabel(r"$R$ (kpc)")
plt.ylabel(r"$V_c$ (km/s)")
plt.legend()
plt.show()
```
