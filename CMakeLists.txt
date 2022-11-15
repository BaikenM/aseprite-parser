cmake_minimum_required (VERSION 3.8)
project ("aseprite_parser")

# Add source to this project's executable.
add_executable (aseprite_parser "main.cpp"
	 "Aseprite_parser.cpp" "Aseprite_parser.h"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET aseprite_parser PROPERTY CXX_STANDARD 20)
endif()
