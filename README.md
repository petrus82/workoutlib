# Workout Library

This library provides a C++ framework for managing cycling training sessions. Each training session consists of one instance of a `Workout` class, which holds a name, workout notes, and a list of `Interval` instances. If functional threshold power (FTP) or maximum heart rate is set, relative power (%FTP), heart rate values (%max heart rate), or training zones will be calculated. This library uses a 7-step zone model for power and a 5-step zone model for heart rate zones.

The `Interval` class holds the interval duration and intensity and can have associated sub-intervals.

**Example: Designing a 8x30/30 VO2max Session**

To create an 8x30/30 VO2max training session, one would construct an `Interval` with VO2max intensity and a 30-second duration. A recovery subinterval of 30 seconds could then be added, and the main `Interval`'s repeat count set to 8. The repeat count of the subinterval does not need to be modified. Iterating over the `Workout` class will result in the desired output: 8 VO2max intervals interleaved with 8 recovery intensity intervals.

## Dependencies
- [Clang](https://clang.llvm.org) > version 22.x
- [CMake](https://cmake.org) > version 4.2.x
- [Ninja](https://ninja-build.org/)
- [Garmin FIT C++ SDK](https://github.com/garmin/fit-cpp-sdk)
- [Google gtest](https://github.com/google/googletest) (for testing)
- [libmd](https://www.hadrons.org/software/libmd) (for testing)

## Project architecture and design

- This uses C++23 with CMake and Ninja.
- The library consists of one main C++20 module file, `workout.cppm`, in the main folder. 
- Code formatting is managed by `clang-format` using a `.clang-format` file. 
- All tests are located in `testing/workout_test.cpp`. 
- Additional CMake files are in the `CMake` subfolder.
- Include directives are avoided by using a `fit.cppm` interface file. A Python script (`strip_macros.py`) is used to generate `fit_profile.cppm`, which replaces macros with `constexpr` declarations. This generated file can be called by the `strip_macros.sh` batch script.

- To indicate test coverage, a `Coverage.cmake` file is used to generate `lcov.info` in the build directory. This file can be used by the [Coverage Gutters vscode extension](https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters) by setting the `Coverage Base Dir` to `build` and the `Coverage Report File Name` to `coverage.lcov`.

## Compile
Ensure you have Clang > 22.x, CMake > 4.2. Install the Garmin FIT C++ SDK and gtest, for example, using `yay clang cmake llvm llvm-libs lld`. A PKGBUILD for Arch Linux-style distributions to install the Garmin FIT C++ SDK is available [here](https://github.com/petrus82/GarminFit). Download the PKGBUILD into a folder and execute `makepkg -si` inside that folder if you have an Arch Linux style distribution.

With the preparations out of the way:
```
git clone https://github.com/petrus82/workoutlib
cmake \
    -DCMAKE_BUILD_TYPE:STRING=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
    -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang \
    -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++ \
    -S . -B build -G Ninja
touch CMakeLists.txt
cmake --build build
``` 