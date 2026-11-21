# Workout Library

This library provides a C++ framework for managing cycling training sessions. Each training session consists of one instance of a `Workout` class, which holds a name, workout notes, and a list of `Interval` instances. If functional threshold power (FTP) or maximum heart rate is set, relative power (%FTP), heart rate values (%max heart rate), or training zones will be calculated. This library uses a 7-step zone model for power and a 5-step zone model for heart rate zones.

The `Interval` class holds the interval duration and intensity and can have associated sub-intervals.

**Example: Designing a 8x30/30 VO2max Session**

To create a 8x30/30 VO2max training session, one would construct a `Interval` with VO2max intensity and a 30-second duration. A recovery subinterval of 30 seconds could then be added, and the main `Interval`'s repeat count set to 8. The repeat count of the subinterval does not need to be modified. Iterating over the `Workout` class will result in the desired output: 8 VO2max intervals interleaved with 8 recovery intensity intervals.

## Dependencies
- [Clang](https://clang.llvm.org) >= 22 (with C++20 module support)
- [CMake](https://cmake.org) >= 4.4.0
- [Ninja](https://ninja-build.org/)
- [Garmin FIT C++ SDK](https://github.com/garmin/fit-cpp-sdk)
- [Google gtest](https://github.com/google/googletest) (for testing)
- [libmd](https://www.hadrons.org/software/libmd) (for testing)
- Java with the SDK's `FitCSVTool.jar` installed to `/usr/lib/garminfit/` (used by the FIT file tests)

## Project architecture and design

- This uses C++23 with CMake and Ninja.
- The library is a shared library composed of the C++20 module files in the main folder; `workout.cppm` is the main module (`workoutlib`).
- Code formatting is managed by `clang-format` using a `.clang-format` file.
- All tests are located in `testing/` (`file_tests.cpp`, `intensity_tests.cpp`, `interval_tests.cpp`, `workout_tests.cpp`) and are written with GoogleTest.
- Build configuration is defined in `CMakePresets.json` (`Debug` and `Release` presets, which set the Clang module-map and std-module flags). Additional CMake files are in the `CMake` subfolder.
- Include directives are avoided by using a `fit.cppm` interface file. A Python script (`strip_macros.py`) is used to generate `fit_profile.cppm`, which replaces macros with `constexpr` declarations. Run it via the `strip_macros.sh` script, e.g. after a Garmin FIT SDK upgrade.
- The `config.cppm` module (holding the `TESTING` switch) is generated into the build directory during CMake's configure phase and must not be edited by hand.

- To indicate test coverage, a `Coverage.cmake` file is used to generate `lcov.info` in the build directory. This file can be used by the [Coverage Gutters vscode extension](https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters) by setting the `Coverage Base Dir` to `build` and the `Coverage Report File Name` to `lcov.info`.

## Compile
Ensure you have Clang >= 22, CMake >= 4.4 and Ninja. Install the Garmin FIT C++ SDK, gtest, libmd and Java, for example using `yay clang cmake llvm llvm-libs lld gtest libmd java`. A PKGBUILD for Arch Linux-style distributions to install the Garmin FIT C++ SDK is available [here](https://github.com/petrus82/GarminFit). Download the PKGBUILD into a folder and execute `makepkg -si` inside that folder if you have an Arch Linux style distribution.

With the preparations out of the way:
```
git clone https://github.com/petrus82/workoutlib
cmake -S . -B build --preset Debug
cmake --build build
```

## Tests

Run all tests with CTest:
```
ctest --test-dir build
```
or directly:
```
./build/Tests
```

To run a single test (or a filtered set of tests), use the GoogleTest filter:
```
./build/Tests --gtest_filter=WorkoutTest*
```

The FIT file tests convert reference files to CSV via `java -jar /usr/lib/garminfit/FitCSVTool.jar`; make sure Java and the jar are installed.

To regenerate the coverage report, run the tests (CTest does this automatically via the `coverage_target` dependency) and then:
```
bash build/coverage.sh
```
