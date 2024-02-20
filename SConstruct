import os

srcs = ["src/" + file for file in ["main.cpp", "derivatives.cpp", "gravity.cpp"]]

flags = ["-std=c++20", "-O3", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-fPIC"]

Program(
    target="bin/rc",
    source="src/main.cpp",
    LIBS=["gsl", "gslcblas", "postana"],
    LIBPATH="./lib",
    CPPPATH="./src",
    CXXFLAGS=flags,
)

SharedLibrary(
    target="lib/libpostana.so",
    source=["src/gravity.cpp", "src/derivatives.cpp", "src/aligner.cpp"],
    LIBS=["gsl", "gslcblas"],
    CPPPATH="./src",
    CXXFLAGS=flags,
)
