# ps2quake

Nicolas Plourde has ported the classic game Quake to the Playstation 2 console.
The game is now fully playable but without sound.
You will need a usb keyboard and mouse supported by the ps2.
You will need either the shareware or full retail of Quake for the Data files to play this port of Quake.

Hi, I just tried to upload to ps2dev.org the first draft of my quake 1 port for the ps2. I was not able to upload the file for some reason (maybe the script dont like Mac OS X + firefox), anyway can I mail it to someone who can upload it to ps2dev.org.

About the port. I only support the video output module yet, no sound, no control and no network. Quake look for id1 directory in host: and is not able to read file from .pak it seem so you have to uncompress it before (maybe its just me so try first). I also have to find a way to load it from hdd or cdrom since network transfer is really slow. Next on my todo list is also to optimise quake math function to run on the vu and port the software drawing engine to use harware accel, right now its running in software at about 10fps, not bad at all. All ps2 code is based on Dreamtime Tutorial.

# Original ReadMe:

```
This is preview release of quake for the Playstation 2.
The game is now fully playable but without sound. You will
need a usb keyboard and mouse supported by the ps2 (logitech
work well).

There is two elf included in this package one for NTSC and
the other for VGA (sorry no pal but You can add it in 
vid_ps2.c). This release was built with stable toolchain
from http://www.oopo.net/consoledev and ps2sdk (CVS) from
http://www.ps2dev.org. I have tested it only on Mac OS X
using ps2client and ps2link 1.23.

To run this thing you only have to execute the proper elf, 
have quake id1 directory has host:id1 (Same dir that you run 
quake-XXX.elf from) and also have the included irx dir in 
your host also (You need the usbd.irx from napalm or sony). 
You can use Quake shareware data or the registred version data. 

That is it enjoy.

Nicolas Plourde
nicolasplourde@hotmail.com

```
