add_library(jsoncpp jsoncpp.cpp)
set_source_files_properties(jsoncpp.cpp PROPERTIES COMPILE_FLAGS -w)
target_link_libraries(jsoncpp)

