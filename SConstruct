import os

srcs = ["src/" + file for file in ["main.cpp", "derivatives.cpp", "gravity.cpp"]]

flags = ["-std=c++20", "-O3", "-Wall", "-Wextra", "-Wpedantic", "-Werror"]

Program("bin/rc", "src/main.cpp", CPPPATH="./src", CXXFLAGS=flags)

SharedLibrary(
    "lib/libpostana.so",
    ["src/gravity.cpp", "src/derivatives.cpp"],
    LIBPATH="./lib",
    CPPPATH="./src",
    CXXFLAGS=flags,
)
