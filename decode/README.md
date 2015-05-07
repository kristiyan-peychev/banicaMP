#Decoders
##Function
The decoders _must_ all decode their respective file types to 16-bit signed integer WAVE format, which is supposedly the CD format, to be later recreated as sound by the player itself
##Usage
To use the decoders you have to include both the `decoder.h` and `get.h` headers in this directory and use `get_decoder` to get a pointer to a decoder for your specified format. Consult `get.h` for a clarification.
##Supported formats
We support only FLAC and MP3 for now. We might consider doing more formats in the future, but we are fine for now.

##TODOs
Make and make an ogg decoder
