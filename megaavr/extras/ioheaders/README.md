# I/O header files
These are the I/O header files from the latest ATpack available at the time of writing, the Atmel AVR Dx Series Device Support v1.10.11 package, supporting all AVR DA and DB parts, as well as the 64k DD-series (likely they will release largest, smallest, and middle size in that order like they did with the DA/DB. Files specific to parts not supported by DxCore are not included. These files are included in the documentation largely because *all post 2016 AVR devices basically require you to have both the datasheet and IO header open* when writing any sort of meaningful code that goes beyond the core and Arduino API functions. (In contrast with older parts, where you needed only the datasheet - the longer names for everything on the modern AVRs are far more readable, and I agree with their decision to pursue the new naming scheme, and these parts are all an incredible improvement; so I don't mean to complain.... but would it have killed them to put the actual word-for-word names for registers/bitfields/etc to copy/paste, so we don't need to keep the header file open too? If you're going to extend the length of the names of bitfields to make for more readable code, that also makes them difficult to type with perfect spelling, capitalization, and underscores in the right places, such that basically everyone will copy-paste them...

I am redistributing these solely as a convenience to users; particularly for less experienced users in the Arduino world, it turns out that being told to look at the header file is a bit daunting ("I found a bunch of iosomething.h files, but not the one for the chip I'm using" "In the Arduino folder? Oh no that's the version that the IDE came with, the core installed a new one..." "well where is it installed to?" "Okay so its in c:/users/(yourusername)/AppData/Arduino15/packages (so on and so forth), but AppData is hidden..." "Uhhhh.... okay can't I just go download it" "Oh of course, you can download it from Microchip - download the file from this page. Then change the file extension to .zip - the download has the .atpack extension, even though it's just a zip file")

The ATpack files are all available from [http://packs.download.atmel.com/](http://packs.download.atmel.com/) They are licensed by Microchip under the Apache license.

The files in this location are NOT used by the core in any way, nor are they included in the board manager releases (the copy of the headers that are used are in `(toolchain root)/avr/include/avr`, where the root of the toolchain installation depends on your OS, how Arduino was installed, and the version of the core). The copy of these files that *is* when compiling for boards defined by this core is the one that is included with the toolchain package that board manager installs. The extras folder is not included in board manager releases at all - which is also why the main README.md has full-length URLs to the reference pages. Nobody wants to have to dig up files installed via board manager, whether it's to find a header used by the compiler, a pinout chart, or the core documentation (the .md files are best read on github, anyway)

## Header file license
**These files are not part of the core and I had no hand in their development**
They have been modified only by running codespell over them in order to make the repo pass CI spellcheck.

This is the license notice displayed when downloading these files. It (and only it) applies to all files in this directory with the exception of README.md itself.

```text
Copyright (c) 2021 Microchip Technology Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the Licence at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
## Note on the DD-series
The DD-series headers are consistent with neither themselves nor the product brief in several places. This is typical (they clearly copy+pasted the content of the DB-series parts), and by the time silicon is shipping, we'll probably have a better set of headers. The important stuff all seems to be present and as expected.

## The most useful references
The highest pincount, highest flash headers are the most useful ones to refer to when you need to know what some option is called or how it's spelled.
* [AVR128DB64 - ioavr128db64.h](ioavr128db64.h)
* [AVR128DA64 - ioavr128da64.h](ioavr128da64.h)
* [AVR64DD32 - ioavr64dd32.h](ioavr64dd32.h)

## Smaller pincount devices
Some functionality depends on the pincount (mostly the number of instances of a given peripheral).
* [AVR128DA28 - ioavr128da28.h](ioavr128da28.h)
* [AVR128DA32 - ioavr128da32.h](ioavr128da32.h)
* [AVR128DA48 - ioavr128da48.h](ioavr128da48.h)
* [AVR128DB28 - ioavr128db28.h](ioavr128db28.h)
* [AVR128DB32 - ioavr128db32.h](ioavr128db32.h)
* [AVR128DB48 - ioavr128db48.h](ioavr128db48.h)
* [AVR64DD14 - ioavr64dd14.h](ioavr64dd14.h)
* [AVR64DD20 - ioavr64dd20.h](ioavr64dd20.h)
* [AVR64DD28 - ioavr64dd28.h](ioavr64dd28.h)

## Smaller-flash-size device headers
These are literally identical except for things directly related to flash or ram size (for example, only 128k parts mention RAMPZ). There is nothing illuminating in these files.
* [AVR64DA28 - ioavr64da28.h](ioavr64da28.h)
* [AVR64DA32 - ioavr64da32.h](ioavr64da32.h)
* [AVR64DA48 - ioavr64da48.h](ioavr64da48.h)
* [AVR64DA64 - ioavr64da64.h](ioavr64da64.h)
* [AVR64DB28 - ioavr64db28.h](ioavr64db28.h)
* [AVR64DB32 - ioavr64db32.h](ioavr64db32.h)
* [AVR64DB48 - ioavr64db48.h](ioavr64db48.h)
* [AVR64DB64 - ioavr64db64.h](ioavr64db64.h)
* [AVR32DA28 - ioavr32da28.h](ioavr32da28.h)
* [AVR32DA32 - ioavr32da32.h](ioavr32da32.h)
* [AVR32DA48 - ioavr32da48.h](ioavr32da48.h)
* [AVR32DB28 - ioavr32db28.h](ioavr32db28.h)
* [AVR32DB32 - ioavr32db32.h](ioavr32db32.h)
* [AVR32DB48 - ioavr32db48.h](ioavr32db48.h)

## The original v1.0.16 headers
These are not used, and have problems that have since been fixed. They also mention a couple of features that were removed from the product with the backspace key (as in, they were removed only from the documentation).
* [AVR128DA28 - old_ioavr128da28.h](old_ioavr128da28.h)
* [AVR128DA32 - old_ioavr128da32.h](old_ioavr128da32.h)
* [AVR128DA48 - old_ioavr128da48.h](old_ioavr128da48.h)
* [AVR128DA64 - old_ioavr128da64.h](old_ioavr128da64.h)
