#Linux WAVE player
##Dependecies
For this module to compile successfully you will need the aftermentioned packages installed. I _will_ provide URLs to all of their respective websites here and in a special file at a later date.
*   libasound
*   libfftw3(only fftw for some packages)
*   libdl

##Compilation

Create a `build` directory. Execute

`cmake ..`

`make`
##Usage
Consult `play.h.in`
##TODO
*   Sound volume;
*   Equalizer;
*   Actually implement it;
*   Fix FIFOs, they leave a file.
