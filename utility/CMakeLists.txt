add_library(utility glutil.cpp glutil.h)	
target_include_directories(utility PUBLIC .)
target_link_libraries(utility ysclass geblkernel)
