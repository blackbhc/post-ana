import os

CPATH = os.environ["CPATH"].split(":")
LD_LIB_PATH = os.environ["LD_LIBRARY_PATH"].split(":")
LIB_PATH = os.environ["LIBRARY_PATH"].split(":")

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
    "-std=c++20",
    # "-ggdb",
    "-O3",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Werror",
    "-fPIC",
]

print("CPATH:", os.environ["CPATH"])

Program(
    target="bin/rc",
    source=srcs,
    LIBS=["gsl", "gslcblas", "postana", "hdf5"],
    LIBPATH=["./lib"] + LIB_PATH + LD_LIB_PATH,
    CPPPATH=["./src"] + CPATH,
    CXXFLAGS=flags,
)

SharedLibrary(
    target="lib/libpostana.so",
    source=srcs[1:],
    LIBS=["gsl", "gslcblas", "hdf5"],
    LIBPATH=["./lib"] + LIB_PATH + LD_LIB_PATH,
    CPPPATH=["./src"] + CPATH,
    CXXFLAGS=flags,
)
