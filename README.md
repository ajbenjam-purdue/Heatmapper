# Heatmapper
(Ideally) cross platform GUI Heatmapper tool

Create build dir:
```sh
cmake -B build -DwxWidgets_CONFIG_EXECUTABLE=/Users/ajbenj/Development/Libraries/wxWidgets-3.3.1/build-osx/wx-config
```

Build application:
```sh
cmake --build build
```

Build test script:
```sh
g++ -I\c:\Users\ajben\Development\eigen-5.0.0 test.cpp ThermalNode.cpp ThermalEdge.cpp ThermalNetwork.cpp -o test && .\test.exe
```