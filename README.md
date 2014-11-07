Modular C++ Sirlin Chess 2
==========================

Chess 2 is a variant of classic Chess (the board game) designed by David Sirlin. Details can be found at http://ludemegames.com/chess2/. Some nice features are having six armies to choose from each with significant differences in playstyle and a double-blind betting system that keeps each game different.

This is currently in a very very rough state with countless bugs and a lackluster SDL frontend, but the core gameplay is mostly correct at this point. I just need to identify and squash a bunch of bugs, and maybe improve the UI a bit, and then it'll at least be completely playable. Submit your issues!

Installation
------------

The only dependencies are:

* A C++ compiler with support for C++11
* SDL 2.0
* SDL_image 2.0

CMake is the build system, and the CMakeLists should be general enough to work on any platform with the libraries installed in the expected locations. Try the following:

    cmake CMakeLists.txt && make

If this failed, make sure that CMake found SDL and that your compiler supports C++11. If it succeeded, you should be able to run the resulting executable with:

    ./chess2-sdl

Running it without any arguments will explain what you need to specify on the command line.
