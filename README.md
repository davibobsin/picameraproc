# picameraproc
RaspberryPi's camera application using V4L2 API.

Tested only with OV5647.

## Compiling
```
cd picameraproc
git submodule update --init
make
```
## File Tree

*  **sources** contains standard code for start and capture with camera using hard-coded parameters.
*  **samples** contains use examples of predefined startup and capture functions.

## Submodules:
* **[STB](https://github.com/nothings/stb) by nothings:** used to convert byte-array image into JPG.