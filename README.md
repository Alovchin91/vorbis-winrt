# Ogg Vorbis for Windows Runtime

**vorbis-winrt** is a work in progress for a simple Windows Runtime component for decoding (so far) [Ogg Vorbis](http://xiph.org/vorbis/) files. It is being written completely in C++/CX, and is based on original _libvorbis_ and _libvorbisfile_ C libraries version 1.3.4 source code. It is available for C# and C++/CX developers writing Windows Store apps for Windows (Phone) 8.1 and higher.

Ogg Vorbis for Windows Runtime currently only exposes a decoding functionality found in original _libvorbisfile_ library. If you think encoder and low-level Vorbis API access are necessary, feel free to contribute ;) Still, as far as Windows Runtime matures and becomes a technology that can be used in desktop development as well, Ogg Vorbis for Windows Runtime will also update and add necessary functionality.

## Sample usage

**vorbis-winrt**'s API is similar to original [Vorbisfile API](http://xiph.org/vorbis/doc/vorbisfile/index.html), except that it's object-oriented. The main class that you will use to decode an Ogg Vorbis file stream is `libvorbisfile.WindowsRuntime.OggVorbisFile` class. Instantiate the class and then call `OggVorbisFile.Open` to initialize the stream decoder. To get samples from the decoder, simply use `OggVorbisFile.Read` method.

**Coming soon**: Example solution containing sample Windows Phone 8.1 background audio project that uses Ogg Vorbis for Windows Runtime.

Here's the sample usage. Assume you have a managed helper class with `OggVorbisFile _vorbisFile` field that is supposed to help you wrap all the decoder logic. Here's how you initialize the decoder:

```cs
public void Initialize(IRandomAccessStream fileStream)
{
    this._vorbisFile.Open(fileStream);
    
    if (!this._vorbisFile.IsValid)
        throw new InvalidOperationException();
}
```

Note that if something goes wrong, the `OggVorbisFile.Open()` method will throw an exception with HResult equal to one of libvorbis' [return codes](http://xiph.org/vorbis/doc/libvorbis/return.html), but still it's recommended to check for `OggVorbisFile.IsValid` each time you call any `OggVorbisFile` API.

Now, to get a sample from the decoder, you can write a simple method like this:

```cs
public IBuffer GetSample(int length)
{
    if (!this._vorbisFile.IsValid)
        throw new InvalidOperationException();
    
    return this._vorbisFile.Read(length);
}
```

**Seeking is not yet implemented.**

## How to build

**vorbis-winrt** includes all the necessary source code to build the libraries. Ogg Vorbis for Windows Runtime solution includes original Vorbis libraries and their dependencies, including [libogg](http://downloads.xiph.org/releases/ogg/), and contains libvorbisfile_winrt project that is the main output of the solution.

You will need Visual Studio 2013 or higher to build the library for Windows (Phone) 8.1 or higher. All the projects currently are set up to build for Windows Phone 8.1, but it's easy to retarget them for Windows 8.1 or higher.

## How to create a media stream source

Use the link bellow or check the wiki.

https://github.com/Alovchin91/vorbis-winrt/wiki/MediaStream-Source
