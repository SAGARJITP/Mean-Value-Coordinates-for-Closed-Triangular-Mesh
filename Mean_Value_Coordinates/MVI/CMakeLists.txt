add_library(meanValueInterpolation meanValueInterpolation.cpp meanValueInterpolation.h)
target_link_libraries(meanValueInterpolation ysclass geblkernel)
target_include_directories(meanValueInterpolation PUBLIC .)

