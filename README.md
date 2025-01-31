DankPlayer
-------

**DankPlayer** is an open-source, command-line video player made exclusively with [Raylib](https://www.raylib.com/) and [FFmpeg](https://www.ffmpeg.org/). It is currently non-functional for daily use, but we are going to get to usability pretty soon! Currently, this project is only supported on Linux.

Build Instructions
------------------

First, clone the repository:

```bash
git clone https://github.com/ProgrammerFahim/dank-player.git
cd dank-player
```

To compile this, you need Raylib and FFmpeg's headers and libraries available on your system. Beyond the standard header and library locations, the `Makefile` currently looks for additional headers and libraries in local `include` and `lib` directories, respectively.

Download the `linux_amd64` Raylib library from [Raylib releases](https://github.com/raysan5/raylib/releases/tag/5.5), unzip them, and copy over the `include` and `lib` directories to the `dank-player` directory.

To get FFmpeg headers and libraries, the straight-forward way of getting them on Debian-based distros is by doing:

```bash
sudo apt install libavformat-dev libavcodec-dev libavutil-dev libavdevice-dev libavfilter-dev libswscale-dev libswresample-dev 
```

And then simply call `make` within the `dank-player` directory:

```bash
make
```

If the Raylib and/or FFmpeg header and library files are **not** in standard locations on your filesystem **and** not in local `include` and `lib` directories (in case you got them some other way or compiled them yourself to a local directory of your choice), you can supply those directory paths to the Makefile using `EXTRA_INC` and `EXTRA_LIB` variables that specify additional header and library directories:

```bash
make EXTRA_INC="-I/path/to/raylib -I/path/to/ffmpeg" EXTRA_LIB="-L/path/to/raylib -L/path/to/ffmpeg"
```
Afterwards, run your video file with:

```bash
./player [path-to-your-file]
```

Things to add
-------------

Here are the list of things to add to make this functional:

- [ ] Add audio track to player (yep, that's how incomplete this is)
- [ ] Add window resizing
- [ ] Add seeking functionality
- [ ] **LOW-LEVEL** Add buffer for decoded frames and separate-thread decoding
