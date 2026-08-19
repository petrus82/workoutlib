export module file_concept;

import std;
import common;
import interval;

namespace Workouts
{

template <typename T>
concept IsIntervalC = std::is_convertible_v<T, std::span<Interval>>;

template <typename T>
concept ReadFileC = requires (T fileHandler) {
  requires IsVoidExpectedC<decltype (fileHandler.checkFile ())>;
  { fileHandler.getWorkoutName () };
  { fileHandler.getWorkoutNotes () };
  { fileHandler.getIntervals () };
};

template <typename T>
concept WriteFileC = requires (T fileHandler) {
  { fileHandler.setWorkoutName (std::string_view{}) };
  { fileHandler.writeName (std::string_view{}) };
  { fileHandler.writeNotes (std::string_view{}) };
};

export template <typename T>
concept FileHandlerC = requires (T fileHandler) {
  requires ReadFileC<T>;
  requires WriteFileC<T>;
};

}; // Workout namespace