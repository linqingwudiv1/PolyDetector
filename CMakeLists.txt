cmake_minimum_required(VERSION 3.10)

# set the project name
project(PolyDetector)

# add the executable
add_executable(${PROJECT_NAME} PolyDetector.h PolyDetector.cpp main.cpp geom.h)