
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Mean_Value_Coordinates/src)

add_subdirectory(Mean_Value_Coordinates/utility)
add_subdirectory(Mean_Value_Coordinates/ObjLoader)
add_subdirectory(Mean_Value_Coordinates/Exec)
add_subdirectory(Mean_Value_Coordinates/MVI)


