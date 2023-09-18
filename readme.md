# voiceseeker
 
The code in `voiceseeker` folder is used for generating the
library used for AFE wrapper, please check the TODO.md in AFE.

### AEC (Acoustic Echo Cancellation)

VoiceSeeker Release without Acoustic Echo Cancellation. Please contact NXP
agent to get the library with AEC enabled.

---

# voicespot

`voicespot` is one of the libraries used for generating the `voice_ui_app`.
This application uses AFE output for Voice detection.
 
### Wake word detection

VoiceSpot is a compact speech detection engine that can be used for
applications such as wake word detection and small vocabulary command
detection.

Try saying **Hey NXP!**

---

# vit

The Voice Intelligent Technology(VIT) product release. It provides
voice services aiming to wakeup and control the IOT devices.

---

# Utils
The `utils` folder has some common code for the above libraries.

---

# Project architecture

Each of above folder has a `lib` folder where you can find the core of the
library. There is a `src` folder (a wrapper of the library) where user can edit
and explorer. Some of them has a `include` where public headers are.

---

# How to compile

### i.MX 8M
* With AEC: `make AEC=1` or `make BUILD_ARCH=CortexA53 AEC=1` or `export AEC=1 && make`.
* Without AEC: `make` or `make BUILD_ARCH=CortexA53`.

### i.MX 9
* With AEC: `make BUILD_ARCH=CortexA55 AEC=1`.
* Without AEC: `make BUILD_ARCH=CortexA55`.

### After build
After a successful compilation the binaries and related files will be located
on the `release` folder. Also on each library will be a `build` folder with the
architecture used for the build.
