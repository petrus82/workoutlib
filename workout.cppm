export module workoutlib;

export import common;
export import intensity;
export import interval;

import file_concept;
import filehandling;
import textfiles;
import planfiles;
import fitfiles;

import config;
import std;
import fitmodule;
import std.compat;

/**
 * @brief Library for defining, managing, and serializing workout
 * routines for cyclists.
 *
 * This library provides structures for defining and managing workout routines
 * composed of timed intervals. It supports reading and writing Garmin Fit,
 * Wahoo Plan, Erg and MRC files.
 *
 * Core Components:
 * * **`Workout`**: Contains workout name, workout notes, the Training Stress
 * Score (TSS) and a collection of `Interval`s.
 * * **`Interval`**: Defines the duration, intensity, and optional repetition
 * structure of a single training segment, supporting nested sub-intervals.
 *
 *
 * Tests are in the testing subdirectory. A helper macro is used to export
 * internal functions only for testing purposes.
 */

namespace Workouts
{

/*
Requirements for a FileHandler class to provide a new file format:
- It has a constructor that takes a std::filesystem::path which can be used
to access the file content, e.g. by using std::ifstream
- It has a std::expected<void, std::string> public checkFile() function which
can return a std::unexpected<std::string> error message if the file cannot be
opened
- It has a public std::expected<std::string, std::string> getWorkoutName()
function that opens the file and returns the workout name as a std::string
- It has a public std::expected<std::string, std::string> getWorkoutNotes()
function that if implemented looks for workout notes which it returns as a
std::string
- It has a public std::expected<std::vector<Interval>> function that return a
vector of Interval. This vector is moved to Workout and Workout is returned by
Workouts::readFile.

- Writing Workout to a file is done by calling
Workout::saveFile(std::filesystem::path()) which calls its internal writeFile
function. It calls
- FileHandler.writeName(std::string_view name),
- FileHandler.writeNotes(std::string_view notes)
- FileHandler.writeIntervals(std::span<Interval> intervals)
Each function returns std::expected<void, std::string> with a std::string error
message in case of any failures.
Thus also these functions have to be declared and implemented in the
FileHandler class.

These requirements are enforced through the FileHandlerC concept declared in
the common module. The existence and correct function parameters for the
writing functions are required by the type system. Doing this by using the
concept is not feasible because this would require knowlegde of Interval in
the common module.

Apart from writing this FileHandler class the new file type
has to be added to the fileextension array and FileType enum in
filehandling.cppm. The FileType enum is used by openFile to select the correct
FileHandler to call readFile with.

An instance of Workout is either created by calling its constructors and using
the setter functions to create Interval data or by calling openFile with the
desired std::filesystem::path file.
*/

#if TESTING == TRUE
#define EXPORT_TEST export
// A constexpr template function cannot modify access modifiers
// So cpp guideline ES.31 does not apply here
// NOLINTNEXTLINE
#define EXPOSE_TEST_IMPL(...)                                                 \
public:                                                                       \
  __VA_ARGS__                                                                 \
private:

// NOLINTNEXTLINE
#define EXPOSE_TEST(...)                                                      \
  /* NOLINTBEGIN */                                                           \
  EXPOSE_TEST_IMPL (__VA_ARGS__)                                              \
  /* NOLINTEND */

#else
#define EXPOSE_TEST(x) x
#define EXPORT_TEST
#endif

export class Workout
{
public:
  Workout () = default;
  explicit Workout (std::string_view workoutName) : m_workoutName (workoutName)
  {
  }

  explicit Workout (std::string_view workoutName, std::string_view notes)
      : m_workoutName (workoutName), m_notes (notes)
  {
  }

// #define DEBUG_CSTOR
#ifdef DEBUG_CSTOR
  Workout (const Workout &other)
      : m_workoutName (other.m_workoutName), m_notes (other.m_notes),
        m_ftp (other.m_ftp), m_maxHeartRate (other.m_maxHeartRate),
        m_minHeartRate (other.m_minHeartRate), m_intervals (other.m_intervals)
  { std::println ("Copy cstor."); }

  Workout &operator= (const Workout &other)
  {
    m_workoutName = other.m_workoutName;
    m_notes = other.m_notes;
    m_ftp = other.m_ftp;
    m_maxHeartRate = other.m_maxHeartRate;
    m_minHeartRate = other.m_minHeartRate;
    m_intervals = other.m_intervals;
    std::println ("Copy assignment operator.");
  }

  Workout (Workout &&other)
      : m_workoutName (std::move (other.m_workoutName)),
        m_notes (std::move (other.m_notes)), m_ftp (other.m_ftp),
        m_maxHeartRate (other.m_maxHeartRate),
        m_minHeartRate (other.m_minHeartRate),
        m_intervals (std::move (other.m_intervals))
  { std::println ("Move ctor."); }

  Workout &operator= (Workout &&other)
  {
    m_workoutName = std::move (other.m_workoutName);
    m_notes = std::move (other.m_notes);
    m_ftp = other.m_ftp;
    m_maxHeartRate = other.m_maxHeartRate;
    m_minHeartRate = other.m_minHeartRate;
    m_intervals = std::move (other.m_intervals);
    std::println ("Move assignment operator.");
  }
#endif

  voidReturn saveFile (std::filesystem::path file)
  {
    return getFileType (file).and_then (
        [&file, this] (auto fileType)
          {
            if (fileType == FileType::Fit)
              {
                return writeFile (fitFiles::FitHandler (file));
              }
            std::unreachable ();
          });
  }
  constexpr void addInterval (Interval &&interval)
  { m_intervals.emplace_back (std::move (interval)); }

  constexpr void setIntervals (Intervals &&intervals)
  {
    m_intervals.clear ();
    m_intervals = std::move (intervals);
  }

  constexpr void removeIntervals (const Intervals::iterator &from,
                                  // NOLINTNEXTLINE
                                  const Intervals::iterator &to)
  { m_intervals.erase (from, to); }

  constexpr std::string getName () const { return m_workoutName; }

  constexpr void setName (std::string_view name) { m_workoutName = name; }

  constexpr std::string getNotes () const { return m_notes; }

  constexpr void setNotes (std::string_view notes) { m_notes = notes; }

  constexpr uint16_t getFtp () const { return m_ftp; }

  constexpr void setFtp (uint16_t ftp) { m_ftp = ftp; }

  constexpr uint8_t getMaxHeartRate () const { return m_maxHeartRate; }

  constexpr void setMaxHeartRate (uint8_t heartRate)
  { m_maxHeartRate = heartRate; }

  constexpr uint8_t getMinHeartRate () const { return m_minHeartRate; }

  constexpr void setMinHeartRate (uint8_t heartRate)
  { m_minHeartRate = heartRate; }

  constexpr auto begin () { return m_intervals.begin (); }
  constexpr auto end () { return m_intervals.end (); }
  auto getIntervals () const { return std::span{ m_intervals }; }

private:
  EXPOSE_TEST (voidReturn writeFile (FileHandlerC auto &&fileHandler) {
    fileHandler.setWorkoutName (m_workoutName);
    fileHandler.setWorkoutNotes (m_notes);
    return fileHandler.writeFile (m_intervals);
  })

private:
  std::string m_workoutName;
  std::string m_notes;
  uint16_t m_ftp{ 0 };
  uint8_t m_maxHeartRate{ 0 };
  uint8_t m_minHeartRate{ 0 };
  Intervals m_intervals;
};

EXPORT_TEST [[nodiscard]] auto readFile (FileHandlerC auto &&fileHandler)
{
  // Executes the checkFile function on the fileHandler. If sucessfull continue
  // with getWorkoutName and getWorkoutNotes. Call setIntervals with
  // fileHandler.getIntervals which collects all intervals from the file. If
  // any of the failible functions return an error, the error message is
  // returned. Otherwise it will be Workout with the data from the file.
  return fileHandler.checkFile ().transform (
      [&fileHandler] ()
        {
          Workout workout{ fileHandler.getWorkoutName (),
                           fileHandler.getWorkoutNotes () };
          workout.setIntervals (fileHandler.getIntervals ());
          return workout;
        });
}

export [[nodiscard]] constexpr auto
openFile (const std::filesystem::path &file)
{
  return getFileType (file).and_then (
      [&file] (auto fileType)
        {
          if (fileType == FileType::Fit)
            {
              return readFile (fitFiles::FitHandler (file));
            }
          std::unreachable ();
        });
}

} // namespace Workouts