# AutoFrotz Library

This library provides an in-process Infocom [Z-machine](https://en.wikipedia.org/wiki/Z-machine) VM instance (typically for playing modern or Infocom-era text adventures).

It is derived from [Frotz](http://frotz.sourceforge.net/) (written by Stefan Jokisch, Galen Hazelwood et al. and [maintained](http://github.com/DavidGriffith/frotz) by David Griffith), specifically the stdio-only Dumb variant (written by Al Petrofsky).

Interface documentation can be directly found in the library header file, [libraries/autofrotz.hpp](../libraries/autofrotz.hpp), in Javadoc-esque documentation comments.

## Licence

The content of the AutoFrotz repository is free software; you can redistribute it and/or modify it under the terms of the [GNU General Public License](http://www.gnu.org/licenses/gpl-2.0.txt) as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

The content is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

## Quick Start

*   Building requires [SCons 2](http://scons.org/) and [buildtools](https://github.com/gcrossland/buildtools). Ensure that the contents of buildtools is available on PYTHONPATH e.g. `export PYTHONPATH=/path/to/buildtools` (or is in the SCons site_scons dir).
*   The library depends on [Core](https://github.com/gcrossland/Core) and [Bitset](https://github.com/gcrossland/Bitset). Build these first.
*   From the working directory (or archive) root, run SCons to make a release build, specifying the compiler to use and where to find it e.g. `scons CONFIG=release TOOL_GCC=/usr/bin`.
    *   The library files are deployed to the library cache dir, which is (by default) under buildtools.
*   The library also comes with a testing demo, which uses the library to run a specified Infocom story file.
    ```shell
    wget "http://mirror.ifarchive.org/if-archive/games/zcode/Advent.z5"
    ./autofrotz <<EOS
    Advent.z5
    east
    take keys
    west
    south
    south
    south
    unlock grate with key
    open it
    down
    quit
    yes
    EOS
    ```
