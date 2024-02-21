This library provide some programs and dynamic library to post analyze the galactic simulation data.
The main concentration of this library is the disk galaxy simulations.

---
# Documentation

## Dependencies
- `python` and `scons` for building: if you haven't installed `scons`, you can install it by `pip install scons`
  or `conda install scons`.
- `make`
- a `C++` compiler for compiling: support C++11 or later, such as `g++` or `clang++`.
- `gsl` library for numerical calculation.
- `hdf5` library for reading and writing HDF5 files.

## Features
- [x] Rotation curves: support total and partial rotation curves (rotation curve of particular components).
- [x] CLI based parameters.
- [x] Parallel calculation (threading) to speed up the process.
- [ ] Dynamic library for python.
- [ ] Multiple snapshots.

## Installation
1. Clone the repository into your local directory: `git clone https://github.com/blackbhc/post-ana.git`
2. Change the directory to the repository: `cd post-ana`
3. Edit the installation path at the third line of `Makefile`, default is `~/.local`.
4. Build the library: `make`. If you encounter any error, run `make clean` before you run `make` again.
5. Install the library: `make install`

Uninstall: `make uninstall`

## Usage
### As a program
For calculation of rotation velocities, run `post [snapshot filename] [analysis result filename]`.

The snapshot file should be in HDF5 format, which follows the convention of the `Gadget` simulation IC file. 

### Results
The program will generate a file with the name you specified. The file is in HDF5 format, and contains the 
rotation curve of the galaxy: one dataset for `Radius`, and another group `Rotation Velocity` for the rotation 
velocities of different components (namely `PartTypeX` in the snapshot file).

The unit of the results is in `km/s` for velocity and `kpc` for radius, if the snapshot file use the "standard"
unit system of `Gadget4` ($\rm{kpc}$, $\rm{Gyr}$, $1e10\ M_\odot$). Otherwise, the unit should be transformed
correspondingly.

### As a dynamic library (future)

