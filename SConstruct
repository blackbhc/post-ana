import os
from SCons.Script import Program

try:
    CPATH = os.environ["CPATH"].split(":")
    LD_LIB_PATH = os.environ["LD_LIBRARY_PATH"].split(":")
    LIB_PATH = os.environ["LIBRARY_PATH"].split(":")
except:
    CPATH = []
    LD_LIB_PATH = []
    LIB_PATH = []

srcs = [
    "src/" + file
    for file in [
        "main.cpp",
        "derivatives.cpp",
        "gravity.cpp",
        "galaxy.cpp",
        "h5io.cpp",
        "aligner.cpp",
    ]
]

flags = [
    "-std=c++11",
    # "-ggdb",
    "-O3",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    # "-Werror",
    "-fPIC",
]

Program(
    target="bin/post",
    source=srcs,
    LIBS=["gsl", "gslcblas", "hdf5"],
    LIBPATH=["./lib"] + LIB_PATH + LD_LIB_PATH,
    CPPPATH=["./src"] + CPATH,
    CXXFLAGS=flags,
)
