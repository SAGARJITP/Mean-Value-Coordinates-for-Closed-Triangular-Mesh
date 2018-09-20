add_library(objloader objloader.cpp objloader.h)
target_link_libraries(objloader ysclass geblkernel)	
target_include_directories(objloader PUBLIC .)

