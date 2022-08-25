<h1 align="center">
  <br>
  <img src="https://raw.githubusercontent.com/lesharris/dorito/master/doc/dorito.png" alt="Dorito" width="400px">
</h1>

<h4 align="center">A Dev Environment for Chip8, SuperChip, and XO-Chip</h4>

<p align="center">
  <a href="#about">About</a> •
  <a href="#installation">Installation</a> •
  <a href="#features">Features</a> •
  <a href="#motivation">Motivation</a> •
  <a href="#thanks">Thanks</a>
</p>

---

<p align="center">
 <img src="https://raw.githubusercontent.com/lesharris/dorito/master/doc/dorito_app.png" alt="Dorito App Screenshot">
</p>

---

## About

**Dorito** is a modern, cross-platform, desktop-based emulator for **Chip8**, **SuperChip**, and **XO-Chip** games. It
also provides a
complete development environment for making your own retro inspired creations. Maybe for the
next [Octojam](https://itch.io/jam/octojam-8)?!

The primary focuses for **Dorito** are:

* Compatibility
    * There are lots of little quirks to run Chip8 games. **Dorito** lets you enable and disable these quirks at will -
      or use
      the built-in compatibility profiles for the major Chip8 systems.

* Easy Development
    * **Dorito** provides a comprehensive, all-in-one development experience. Code using the built-in editor, create
      sound effects and sprites, monitor your games as they run, and more.

* Community
    * There is a large community of *Chip developers. **Dorito** wants to be a good citizen by providing an
      alternative *Chip development environment that works with the community and not replace any part of it.

## Installation

**Dorito** currently provides [binary builds](https://github.com/lesharris/dorito/releases) for Windows and MacOS (x86
and Apple Silicon)  Linux support should be
there
from a 'compiling the source' standpoint but work remains on packaging Dorito for Linux. Is Snap the way to go these
days? Linux folks, would love some feedback here.

Dorito uses Cmake's FetchContent along with vcpkg for dependency management. Ninja is also the preferred build system.
To build from
source, the following recipe
should be all that is needed:

```
$ git clone https://github.com/lesharris/dorito.git
$ cd dorito
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake -G Ninja ..
$ ninja
```

And then if you wanted to build a release for some reason you would then execute:

```
$ ninja package
```

That will create either a Zip release file, or a MacOS DMG file.

## Features

**Dorito** can do a lot!  Here are some highlights:

<p align="center">
  <img src="https://raw.githubusercontent.com/lesharris/dorito/master/doc/dorito_xo.png" alt="Dorito XO Chip Support">
</p>

Full compatibility with the pre-eminent and original modern *Chip
environment, [Octo](http://johnearnest.github.io/Octo/). It implements _full_ XO-Chip support including the
newer `audio` and `pitch` opcodes.

<p align="center">
  <img src="https://raw.githubusercontent.com/lesharris/dorito/master/doc/dorito_code.png" alt="Dorito Code Editor">
</p>

The dev environment integrates John Earnest's Octo Assembly Language compiler (gratefully taken from the
official [c-octo](https://github.com/JohnEarnest/c-octo) project) with the built-in editor. Compiler errors show exactly
where the problem is in the editor, `:monitor`s and `:breakpoint`s are fully supported.

<p align="center">
  <img src="https://raw.githubusercontent.com/lesharris/dorito/master/doc/dorito_sound.png" alt="Dorito Sound Editor">
</p>

Includes a complete implementation of the standard Octo sound tool for creating sound effects for your games.

<p align="center">
  <img src="https://raw.githubusercontent.com/lesharris/dorito/master/doc/dorito_sprite.png" alt="Dorito Sprite Editor">
</p>

Also includes a full featured sprite editor with support for 8x16 and 16x16 sprites. In both 1-bit or _glorious_ 2-bit
color!

## Dorito vs Octo Compatibility

|                                            | Dorito | Octo |
|--------------------------------------------|:------:|:----:|
| Full Chip8, SuperChip, and XO-Chip Support |   ✔    |  ✔   |
| `audio` support                            |   ✔    |  ✔   |
| `pitch` support                            |   ✔    |  ✔   |
| Pre-defined palettes and custom palettes   |   ✔    |  ✔   |
| Speed settings (cycles per frame)          |   ✔    |  ✔   |
| Automatic Per-ROM configuration saving     |   ✔    |      |
| Run-time tracing disassembler              |   ✔    |  ✔   |
| Official Octo Assembly Language compler    |   ✔    |  ✔   | 
| Individual Quirk setting                   |   ✔    |  ✔   |
| Compatibility Profiles                     |   ✔    |  ✔   |
| Sprite Editor                              |   ✔    |  ✔   |
| Sound Editor                               |   ✔    |  ✔   |
| Visual Internal CPU State                  |   ✔    |  ✔   |
| Audio Waveform and Buffer visualization    |   ✔    |      |
| RAM Visualization and Editing              |   ✔    |      |
| CPU Profiling                              |        |  ✔   |
| Octo Cartridges                            |        |  ✔   |
| Standalone Static Analysis disassembler    |        |  ✔   |
| Multiple Font choices                      |        |  ✔   |
| Chip flavored Name                         |   ✔    |      |
| Sea creature flavored Name                 |        |  ✔   |

One of the goals of **Dorito** is full Octo compatibility and missing features are targets for future versions. Pull
requests welcome! Hop on in!

## Motivation

I've written numerous emulators over the years and after getting my Gameboy emulator to a place I was happy with I
thought to myself, "Self, how about you write a _good_ Chip8 emulator instead of one of your many _bad_ ones for once?"
And so Dorito was born.

Dorito really wants to be part of the discussion when it comes to the future of Chip8 emulation and development. Rather
than all new features being introduced into Octo, Dorito wants to provide a 'second-source' for experimentation and
implementation. Also John Earnest's work on Octo clearly speaks for itself, but personally I wanted a cross platform
option that was not web-based.

## Thanks

Like Newton (but also very much not like Newton lol) I've stood on the shoulders of giants when creating Dorito. I would
like to personally thank [John Earnest](https://github.com/JohnEarnest) for not only keeping the Chip8 light on but
ensuring it remains brightly lit. I also want to thank Timendus for his work on improving the general state of Chip8
emulation with the release of his [CHIP-8 test suite](https://github.com/Timendus/chip8-test-suite) (which Dorito
passes, thank you Timendus!) as well as his own excellent emulator [Silicon8](https://github.com/Timendus/silicon8)
which was a tremendous help when trying to figure out what the heck was going on with XO-Chip and audio. Lastly, a big
thank you to all the folks on [/r/EmuDev](https://old.reddit.com/r/EmuDev) for all the support, community, and
entertainment provided over the years. Maybe I'll stop lurking and post. Then again, maybe not.

