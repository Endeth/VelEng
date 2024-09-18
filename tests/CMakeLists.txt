
add_executable (VelTest VelTest.cpp)
set_property(TARGET VelTest PROPERTY CXX_STANDARD 20)
target_link_libraries(VelTest
	VelEng
)

include_directories(${PROJECT_BINARY_DIR}/generated/)