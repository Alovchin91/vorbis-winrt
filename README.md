# Ogg Vorbis for Windows Runtime

**vorbis-winrt** is a work in progress for a simple Windows Runtime component for decoding (so far) [Ogg Vorbis](http://xiph.org/vorbis/) files. It is being written completely in C++/CX, and is based on original _libvorbis_ and _libvorbisfile_ C libraries version 1.3.4 source code. It is available for C# and C++/CX developers writing Windows Store apps for Windows (Phone) 8.1 and higher.

Ogg Vorbis for Windows Runtime currently only exposes a decoding functionality found in original _libvorbisfile_ library. If you think encoder and low-level Vorbis API access are necessary, feel free to contribute ;) Still, as far as Windows Runtime matures and becomes a technology that can be used in desktop development as well, Ogg Vorbis for Windows Runtime will also update and add necessary functionality.

## Sample usage

COMING SOON

## How to build

**vorbis-winrt** includes all the necessary source code to build the libraries. Ogg Vorbis for Windows Runtime solution includes original Vorbis libraries and their dependencies, including [libogg](http://downloads.xiph.org/releases/ogg/), and contains libvorbisfile_winrt project that is the main output of the solution.

You will need Visual Studio 2013 or higher to build the library for Windows (Phone) 8.1 or higher. All the projects currently are set up to build for Windows Phone 8.1, but it's easy to retarget them for Windows 8.1 or higher.
