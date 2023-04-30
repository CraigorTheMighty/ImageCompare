ImageCompare
============

A small command-line program to compare sets of images against a reference image.

Outputs traditional PSNR information, and optionally SSIM images.

Usage
=====

Run "compare.exe" without any arguments in the command-line to get program usage information.

Dependences
===========

Uses the DevIL library (https://openil.sourceforge.net/download.php) to load source images. DevIL is licensed under LGPL 2.1. The relevant headers, .libs, .dlls are included in this repository.

Uses libKTX (https://github.com/KhronosGroup/KTX-Software/releases/) to load .ktx images. libKTX is licensed under the Apache 2.0 license. The relevant headers, .libs, .dlls are included in this repository.

Compiling
=========

I personally use Microsoft Visual Studio - Community Edition 2022 to build. I've not tested building using other tools or on other OS's, so your mileage may vary.

You _SHOULD_ use a toolchain with OpenMP support that recognises the relevant #pragma statements in the source. The SSIM computations will be _extremely_ slow if you do not have OpenMP support.

You will need to include the following .libs (included in this repository where possible):

- DevIL.lib
- ILU.lib
- ILUT.lib
- ktx.lib

You must include the following .dlls in the same directory as the final executable (or where Windows can otherwise find them):

- DevIL.dll
- ILU.dll
- ILUT.dll
- ktx.dll

License
=======

MIT license. You can pretty much do what you like with this code, but please consider attributing me as the source.

Copyright (c) 2023 Craig Sutherland

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
