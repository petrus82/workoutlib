export module workoutlib;

export import common;
export import intensity;
export import interval;

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

Apart from implementing this FileHandler class the new file type has to be
added to the fileextension array and FileType enum in filehandling.cppm. The
FileType enum is used by openFile to select the correct FileHandler to call
readFile with.

An instance of Workout is either created by calling its constructors and using
the setter functions to create Interval data or by calling openFile with the
desired std::filesystem::path file.
*/

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
  /*   Workout (const Workout &other)
        : m_workoutName (other.m_workoutName), m_notes (other.m_notes),
          m_ftp (other.m_ftp), m_maxHeartRate (other.m_maxHeartRate),
          m_minHeartRate (other.m_minHeartRate), m_intervals
    (other.m_intervals) { std::println ("Copy cstor."); }

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
    } */

  constexpr std::expected<void, std::string>
  writeFile (std::filesystem::path &file, FileType fileType,
             IntensityUnit IntensityUnit, uint16_t relativeTo)

  { return {}; }

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

private:
  std::string m_workoutName;
  std::string m_notes;
  uint16_t m_ftp{ 0 };
  uint8_t m_maxHeartRate{ 0 };
  uint8_t m_minHeartRate{ 0 };
  Intervals m_intervals;
};

export [[nodiscard]] std::expected<Workout, std::string>
readFile (FileHandlerC auto &&fileHandler)
{
  Workout workout;
  return
      // 1. Check if the std::filesystem::path file that was given to the
      // fileHandler can be opened by calling its checkFile function. It
      // returns an error message if needed thus terminating this function.
      fileHandler.checkFile ()
          .and_then (
              [&fileHandler] () -> std::expected<std::string, std::string>
                { // 2. If the file can be read, continue by reading the
                  // workout name from the file
                  return fileHandler.getWorkoutName ();
                })
          .and_then (
              [&fileHandler, &workout] (std::string_view name)
                  -> std::expected<std::string, std::string>
                {
                  // 3. Set Name and get Notes
                  workout.setName (name);
                  return fileHandler.getWorkoutNotes ();
                })
          .transform (
              [&workout] (std::string_view notes)
                {
                  // 4. Set notes
                  workout.setNotes (notes);
                })
          .and_then (
              [&fileHandler,
               &workout] () -> std::expected<Workout, std::string>
                {
                  // 5. Set Intervals
                  workout.setIntervals (fileHandler.getIntervals ());
                  return workout;
                });
}

[[nodiscard]] constexpr std::expected<Workout, std::string>
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