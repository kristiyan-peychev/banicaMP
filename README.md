##banica Music Player
###Purposes and goals
We started this as something to do in our spare time, and it still is. TODO: Write something emotional here.

###Dependencies
The player is dependent on the following, open-source libraries. URLs to each library's respective website will be provided eventually.
*   libmad
*   libFLAC, libFLAC++
*   libtag
*   portaudio

It is also worth noting that to build the player you will *need* a compiler with support for C++14. As of the time of this edit Micosoft's Visual C++ compiler is highly likely _not_ to be able to compile the entire project.

###Compilation and Installation
The player uses CMake as its main build script _for now_.
To build the project just execute:

`mkdir build`

`cd build`

`cmake ..`

`make`

###TODOs
*   Write a proper README;
*   Look at TODOs in subdirs;
*   Get more decoders;
*   Make a proper GUI;
*   Make a proper CLI;
*   Add a logger;
*   Add a persist/config system;
*   Add networking support;
*   Add audio filters;
*   Document each module properly(Doxygen?);
*   Test this under windows and OS X;

[![Join the chat at https://gitter.im/kristiyan-peychev/banicaMP](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/kristiyan-peychev/banicaMP?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
