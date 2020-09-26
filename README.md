# picameraproc
RaspberryPi's camera application using V4L2 API.

Tested only with OV5647.

## Compiling
```
cd picameraproc
git submodule update --init
make
```

## Submodules:
* **[STB](https://github.com/nothings/stb) by nothings:** used to convert byte-array image into JPG.