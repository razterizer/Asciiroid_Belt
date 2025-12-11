# Asciiroids

![GitHub License](https://img.shields.io/github/license/razterizer/Asciiroids?color=blue)
![Static Badge](https://img.shields.io/badge/C%2B%2B-20-yellow)

[![build ubuntu](https://github.com/razterizer/Asciiroids/actions/workflows/build-ubuntu.yml/badge.svg)](https://github.com/razterizer/Asciiroids/actions/workflows/build-ubuntu.yml)
[![build macos](https://github.com/razterizer/Asciiroids/actions/workflows/build-macos.yml/badge.svg)](https://github.com/razterizer/Asciiroids/actions/workflows/build-macos.yml)
[![build windows](https://github.com/razterizer/Asciiroids/actions/workflows/build-windows.yml/badge.svg)](https://github.com/razterizer/Asciiroids/actions/workflows/build-windows.yml)
![Valgrind Status](https://raw.githubusercontent.com/razterizer/Asciiroids/badges/valgrind-badge.svg)

<!-- [![release linux](https://github.com/razterizer/Asciiroids/actions/workflows/release-linux.yml/badge.svg)](https://github.com/razterizer/Asciiroids/actions/workflows/release-linux.yml)
[![release macos](https://github.com/razterizer/Asciiroids/actions/workflows/release-macos.yml/badge.svg)](https://github.com/razterizer/Asciiroids/actions/workflows/release-macos.yml)
[![release windows](https://github.com/razterizer/Asciiroids/actions/workflows/release-windows.yml/badge.svg)](https://github.com/razterizer/Asciiroids/actions/workflows/release-windows.yml) -->

![Top Languages](https://img.shields.io/github/languages/top/razterizer/Asciiroids)
![GitHub repo size](https://img.shields.io/github/repo-size/razterizer/Asciiroids)
![C++ LOC](https://raw.githubusercontent.com/razterizer/Asciiroids/badges/loc-badge.svg)
![Commit Activity](https://img.shields.io/github/commit-activity/t/razterizer/Asciiroids)
![Last Commit](https://img.shields.io/github/last-commit/razterizer/Asciiroids?color=blue)
![Contributors](https://img.shields.io/github/contributors/razterizer/Asciiroids?color=blue)

![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/razterizer/Asciiroids/total)


<img alt="Asciiroids 25fps 2" src="https://github.com/user-attachments/assets/d5af6a5b-1230-4545-9dcf-e8b84f119b44" width="696" height="596" />

<img width="703" height="594" alt="image" src="https://github.com/user-attachments/assets/369e49c5-8242-4997-8fa4-d46e4f40138d" />

## About the Game

A cross-platform (Lin/Mac/Win) terminal-based shooter game.

Relive the old Atari Asteroids arcade game from 1979 with this clone. 
It gets even better, because this game allow you to play it directly on your favourite terminal (emulator) or windows console.

This [`Termin8or`](https://github.com/razterizer/Termin8or)-based game uses a vector based sprite for the spaceship and bitmap based sprites for asteroids, UFOs and explosion animations.
It uses the Termin8or `CollisionHandler` for collisions between asteroids <-> spaceship and UFOs <-> spaceship.
[`8Beat`](https://github.com/razterizer/8Beat) is used for "music" playback and SFX generation and playback.

I hope this game will give you much joy and fun.

### Keys

  * `←`, `a`           : Turn Left.
  * `→`, `d`           : Turn Right.
  * `↑`, `w`           : Thrust.
  * `[space]`          : Fire.
  * `[backspace]`, `h` : Hyperspace. 

## Build & Run Instructions

There are two options on dealing with repo dependencies:

### Repo Dependencies Option 1

This method will ensure that you are running the latest stable versions of the dependencies that work with `Asciiroids`.

The script `fetch-dependencies.py` used for this was created by [Thibaut Buchert](https://github.com/thibautbuchert).
`fetch-dependencies.py` is used in the following scripts below:

After a successful build, the scripts will then prompt you with the question if you want to run the program.

When the script has been successfully run for the first time, you can then go to sub-folder `Asciiroids` and use the `build.sh` / `build.bat` script instead, and after you have built, just run the `run.sh` or `run.bat` script.

#### Windows

Run the following script:
```sh
setup_and_build.bat
```

#### MacOS

Run the following script:
```sh
setup_and_build_macos.sh
```

#### Linux (Debian-based, e.g. Ubuntu)

Run the following script:
```sh
setup_and_build_debian.sh
```

### Repo Dependencies Option 2

In this method we basically outline the things done in the `setup_and_build`-scripts in Option 1.

This method is more suitable for development as we're not necessarily working with "locked" dependencies.

You need the following header-only libraries:
* https://github.com/razterizer/Core
* https://github.com/razterizer/Termin8or
* https://github.com/razterizer/8Beat
* https://github.com/razterizer/AudioLibSwitcher_applaudio
* https://github.com/razterizer/TrainOfThought
* https://github.com/razterizer/applaudio

Make sure the folder structure looks like this:
```
<my_source_code_dir>/lib/Core/                      ; Core repo workspace/checkout goes here.
<my_source_code_dir>/lib/Termin8or/                 ; Termin8or repo workspace/checkout goes here.
<my_source_code_dir>/lib/8Beat/                     ; 8Beat repo workspace/checkout goes here.
<my_source_code_dir>/lib/AudioLibSwitcher_applaudio ; AudioLibSwitcher_applaudio repo workspace/checkout goes here.
<my_source_code_dir>/lib/TrainOfThought             ; TrainOfThought repo workspace/checkout goes here.
<my_source_code_dir>/lib/applaudio                  ; applaudio repo workspace/checkout goes here.
<my_source_code_dir>Asciiroids/                     ; Asciiroids repo workspace/checkout goes here.
```

These repos are not guaranteed to all the time work with the latest version of `Asciiroids`. If you want the more stable aproach then look at Option 1 instead.

### Windows

Then just open `<my_source_code_dir>/Asciiroids/Asciiroids/Asciiroids.sln` and build and run. That's it!

You can also build it by going to `<my_source_code_dir>/Asciiroids/Asciiroids/` and build with `build.bat`.
Then you run the game by typing `run.bat`.

### MacOS

Go to `<my_source_code_dir>/Asciiroids/Asciiroids/` and build with `./build.sh`.

Then run by typing `./bin/asciiroids`.

### Linux

Go to `<my_source_code_dir>/Asciiroids/Asciiroids/` and build with `./build.sh`.

Then run by typing `./bin/asciiroids`.

## Make New Release

To make a new release, you first have to update the `RELEASE_NOTES.md` document. E.g.:

<img width="759" height="406" alt="image" src="https://github.com/user-attachments/assets/f5fbd6e3-cf10-473b-8e7d-908189c82d20" />


Then to make a new release you should use the `tag_release.sh` script:
```sh
./tag_release -f RELEASE_NOTES.md
```

or if you want to re-upload an existing release, e.g.:
```sh
./tag_release 1.1.5.7. -f RELEASE_NOTES.md
```

you can also use these commands:
```sh
./tag_release bump patch "Release notes."
```
```sh
./tag_release bump minor "Release notes."
```
```sh
./tag_release bump major "Release notes."
```
```sh
./tag_release 1.1.5.7 "Release notes."
```

If you want to remove a release for some reason, you can do so by running the following git commands:
```sh
# Delete local tag
git tag -d release-1.0.0.0

# Delete remote tag
git push --delete origin release-1.0.0.0
```

## Running from a Release

When you download a MacOS release then you need to tell the gatekeeper unblock the executable (here: `asciiroids`), but only if you trust the program of course (check source code + release workflow if you're unsure).
When you feel ready, you can allow the binary to be run by going to the release folder and type the following:

```sh
xattr -dr com.apple.quarantine asciiroids
```

On Windows, you might have to unblock the exe by right-clicking the exe-file and check the `Unblock` checkbox.
