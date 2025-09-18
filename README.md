This is an OpenGL renderer for the C UI layout library clay.h  
An intended use case for this renderer is game UIs, both debugging and in game. Because it is entirely c and c++ no multi-language programming is required (if the game is written in c++), allowing for easy modification of game variables through the UI.

Example UI generated using example.cpp:
<p align="center">
  <img src="images/example.png" alt="Clay OpenGL Demo" width="600" style="border:2px solid gray;">
</p>

To run the example first replace the CMAKE_TOOLCHAIN_FILE variable with the appropriate value for your setup.  
Then run the following commands:  
cmake --preset debug  
cmake --build build\debug -j  
cmake --build build\debug -  
build\debug\clay_example.exe  
Currently only been tested on Windows