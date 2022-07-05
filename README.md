# rtlsdr-demo
A demonstration for making USB drivers for 9Front, using the rtl sdr code as an example.

orginal code from here:

https://github.com/osmocom/rtl-sdr

This is not intended as a working driver.  What it is is an example of reading other drivers and writing drivers to work with 9Front or Plan 9.  So I've commented out some part, and then added equivalent code to work with 9Front.  An example would be in librtlsdr.c lib_control_transfer for the 9Front equivalent usbcmd.

I also moved a lot of stuff from individual header files into dat.h for macros, variables and structs, and into fns.h to hold functions.

the videos covering this can be found here;

https://youtu.be/Xn4u_2wxOdE
