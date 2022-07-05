# rtlsdr-demo
A demonstration for making USB drivers for 9Front, using the rtl sdr code as an example.

orginal code from here:

https://github.com/osmocom/rtl-sdr

This is not intended as a working driver.  What it is is an example of reading other drivers and writing drivers to work with 9Front or Plan 9.  So I've commented out some part, and then added equivalent code to work with 9Front.  An example would be in librtlsdr.c lib_control_transfer for the 9Front equivalent usbcmd.

I also moved a lot of stuff from individual header files into dat.h for macros, variables and structs, and into fns.h to hold functions.

the videos covering this can be found here;

https://youtu.be/Xn4u_2wxOdE

If you want to actually compile it and use it;
put it in a directory under /sys/src/cmd/nusb
use the appropriate compiler to compile each file
link it all togather with the standard usb library like so (this is with the 6c and 6l for amd64)
'6l rtlfm.6 librtlsdr.6 tuner_e4k.6 tuner_fc0012.6 tuner_fc0013.6 tuner_fc2580.6 tuner_r82xx.6 ../lib/usb.a6'
if you don't have the ../lib/usb.a6 file, go to /sys/src/cmd/nusb/lib and run mk to build the library.
