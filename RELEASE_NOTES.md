# Release Notes

## 1.3.3.11

- Adopted Forge with applaudio cboxes as the default dependency style and reproducible release locks.
- Added an opt-in OpenAL build through `--style=git-source --profile=openal` without publishing OpenAL cboxes.
- Updated hosted applaudio builds to 8Beat 1.0.4.7 with asynchronous chiptune playback and shutdown fixes.
- Pinned OpenAL source builds to adapter release 1.0.1.14 through 8Beat.
- Documented Forge build modes for applaudio, OpenAL, Release, and local development.
- Made the 3D audio transforms backend-neutral and selected the backend-specific channel layout at the AudioLibSwitcher boundary.

## 1.3.2.10
- Getting rid of now obsolete dependency to TrainOfThought.

## 1.3.1.9
- Temporarily disabled the title screen while the splash artwork is redesigned.
- Removed the title splash texture from local builds and Linux, macOS, and Windows release packages.

## 1.3.0.8
- Bumped Termin8or dependency to 3.0.0.6.
- Updated compatibility with Termin8or's Unicode/glyph API changes.

## 1.2.1.7
- Fixed tag_release.sh script so that it doesn't output a version.h header (copy-paste mistake).

## 1.2.0.6
- Using Termin8or 2.0.0.2 with 8-bit color support. But we still render in black and white though.
- Improved volume levels.

## 1.1.3.5
- Fixes the --help argument so that it doesn't display audio settings on top of the help text, which seems a little out of place.
- Replacing SFX sources from AudioStreamSource to just AudioSource as I think it is a bit overkill to use AudioStreamSource here.
- Adding 3d sound. It can be disabled by using the command line argument --disable_3d_audio.
- Adding CLArgs --music_volume and --sfx_volume and improving help page formatting.
- Separating sfx gain and gain factor from sfx master volume. It now sounds right I think.
- Bumping patch instead of minor in order to remedy bumping of minor by mistake at version 1.1.0.2.

## 1.1.2.4
- Oops. Added the addendum in the wrong place.

## 1.1.1.3
- Added unblocking instructions in windows and macos workflows.

## 1.1.0.2
- Try try again

## 1.0.1.1
- Fixed partial redraw on linux and wsl.

## 1.0.0.0
- Updated dependencies (applaudio).
