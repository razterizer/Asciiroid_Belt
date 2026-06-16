#!/bin/bash

os_name=$(uname)

if [[ $os_name == *"Darwin"* ]]; then
  additional_flags="\
    -I../../lib/Core/include \
    -I../../lib/Termin8or/include \
    -I../../lib/8Beat/include \
    -I../../lib/AudioLibSwitcher_applaudio/include \
    -I../../lib/AudioLibSwitcher_applaudio/AudioLibSwitcher/include \
    -I../../lib/applaudio/include -framework AudioToolbox -framework CoreAudio -framework CoreFoundation -DUSE_APPLAUDIO"
  # -I../../lib/AudioLibSwitcher_OpenAL/include \
  # -I/opt/homebrew/opt/openal-soft/include -L/opt/homebrew/opt/openal-soft/lib -lopenal"
else
  additional_flags="\
    -I../../lib/Core/include \
    -I../../lib/Termin8or/include \
    -I../../lib/8Beat/include \
    -I../../lib/AudioLibSwitcher_applaudio/include \
    -I../../lib/AudioLibSwitcher_applaudio/AudioLibSwitcher/include \
    -I../../lib/applaudio/include -DUSE_APPLAUDIO"
  # -I../../lib/AudioLibSwitcher_OpenAL/include"
  #export BUILD_PKG_CONFIG_MODULES='openal'
  export BUILD_PKG_CONFIG_MODULES='alsa'
fi

../../lib/Core/build.sh asciiroid_belt "$1" "${additional_flags[@]}"

# Capture the exit code of Core/build.sh
exit_code=$?

if [ $exit_code -ne 0 ]; then
  echo "Core/build.sh failed with exit code $exit_code"
  exit $exit_code
fi

### Post-Build Actions ###

mkdir -p bin/fonts/
cp ../../lib/Termin8or/include/Termin8or/title/fonts/* bin/fonts/

cp music.ct bin/
cp title.txt bin/
