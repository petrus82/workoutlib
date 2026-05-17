# Workout Library

This library provides a C++ framework for managing cycling training sessions. 
Each training session consists of one instance of a Workout class. It holds a name, workout notes and a list of Interval instances.

If the functional threshold power (FTP) or the max heart rate is set, relative power (%ftp) or heart rate values (% max heart rate) or training zones will be calculated. This library uses a 7 step zone model for power and a 5 step zone model for heart rate zones.

The Interval class holds the interval duration and intensity. It can have associated sub intervals.

**Example: Designing a 8x30/30 VO2max Session**

To create a 8x30/30 VO2max training session, you would construct an `Interval` with VO2max intensity and 30 seconds duration, then add a recovery subinterval with 30 seconds and set the repeat count of the Interval to 8. The repeat count of the subinterval doesn't have to be changed. Iterating over the Workout class would result in the desired output of 8 Intervals with VO2Max interleaved by 8 Intervals with recovery intensity.

## Dependencies
- [Clang](https://clang.llvm.org) > version 22.x
- [CMake](https://cmake.org) > version 4.2.x
- [Garmin FIT C++ SDK](https://github.com/garmin/fit-cpp-sdk)
- [Google gtest](https://github.com/google/googletest)

## Project architecture and design

- This is using C++23 with CMake. 
- The library consists of one main C++20 module file workout.cppm in the main folder. 
- Code formatting is done by use of clang-format with a .clang-format file. 
- All tests are in testing/workout_test.cpp. 
- Additional CMake files are in the CMake subfolder.
- Include directives are avoided by using a fit.cppm interface file. To get rid of all the macros in fit_profile.hpp a python script (strip_macros.py) is used. It generates fit_profile.cppm which uses constexpr declarations to replace the macros. It can be called by using the strip_macros.sh batch script.

- To indicate test coverage a coverage.cmake file is used to generate lcov.info in the build directory. This can be used by the [Coverage Gutters vscode extension](https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters) by setting the 'Coverage Base Dir' to 'build' and the 'Coverage Report File Name' to 'coverage.lcov'.

## Compile
Make sure you have clang > 22.x, CMake >4.2. Install Garmin FIT C++ SDK and gtest,
e.g. `yay clang cmake llvm llvm-libs lld` A PKGBUILD for arch linux style distributions to install the Garmin FIT C++ SDK is [here](https://github.com/petrus82/GarminFit). Download the PKGBUILD into a folder and execute makepkg -si inside this folder if you have an Arch Linux style distribution.

With the preparations out of the way:
```
git clone https://github.com/petrus82/workoutlib
cmake -B build -G Ninja .
cmake --build build
``` 