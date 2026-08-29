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
  { fileHandler.setWorkoutNotes (std::string_view{}) };
  {
    fileHandler.writeFile (std::filesystem::path{}, std::string_view{},
                           std::string_view{}, std::span<Interval>{})
  };
};

export template <typename T>
concept TestAdapterC = requires (T fileHandler) {
  { fileHandler.getFileHeader () };
  { fileHandler.checkFile () };
};

export template <typename T>
concept FileHandlerC = requires (T fileHandler) {
  requires ReadFileC<T>;
  requires WriteFileC<T>;
};

}; // Workout namespace