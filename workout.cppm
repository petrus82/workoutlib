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
- It has a public pair of begin/end functions that return a
std::forward_iterator<Interval>. By creating a std::vector<Interval> in begin()
and adding all Intervals of the file they are added to the Workout returned by
Workouts::readFile.

Apart from implementing this FileHandler class the new file type has to be
added to the fileextension array and FileType enum in filehandling.cppm. The
FileType enum is used by openFile to select the correct FileHandler to call
readFile with.

An instance of Workout is either created by calling its constructors and using
the setter functions to create Interval data or by calling openFile with the
desired std::filesystem::path file.
*/

/*
  TODO:
- Check in testing that NRVO works in readFile
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

  {
    // WriteFunction writeFunc;
    std::fstream filestream (file, std::ios::out);
    if (filestream.fail ())
      {
        std::error_code error;
        return std::unexpected (std::format (
            "Cannot open file {}. {}", file.string (), error.message ()));
      }
    /*     switch (fileType)
          {
          case FileType::Erg:
            writeFunc = [] (std::iostream &filestream, Interval &interval,
                            WorkoutType type, uint16_t relativeTo)
              { ErgFile::writeInterval (filestream, interval, type,
       relativeTo); }; ErgFile::writeWorkout (filestream, *this); break;
       case FileType::Fit: writeFunc = [] (std::iostream &filestream,
       Interval &interval, WorkoutType type, uint16_t relativeTo) {
       FitFile::writeInterval (filestream, interval, type, relativeTo); };
            filestream.open (file, std::ios::out | std::ios::binary);
            FitFile::writeWorkout (filestream, *this);
            break;
          case FileType::Mrc:
            writeFunc = [] (std::iostream &filestream, Interval &interval,
                            WorkoutType type, uint16_t relativeTo)
              { MrcFile::writeInterval (filestream, interval, type,
       relativeTo); }; MrcFile::writeWorkout (filestream, *this); break;
       case FileType::Plan: writeFunc = [] (std::iostream &filestream,
       Interval &interval, WorkoutType type, uint16_t relativeTo)
              {
                PlanFile::writeInterval (filestream, interval, type,
       relativeTo);
              };
            PlanFile::writeWorkout (filestream, *this);
            break;
          } */
    for (auto &interval : m_intervals)
      {
        // writeFunc (filestream, interval, IntensityUnit, relativeTo);
      }
    return {};
  }

  constexpr void addInterval (Interval &&interval)
  { m_intervals.emplace_back (std::move (interval)); }

  constexpr void setIntervals (Intervals &&intervals)
  {
    m_intervals.clear ();
    m_intervals = std::move (intervals);
  }

  /* constexpr void removeIntervals (const IteratorType &from,
                                    const IteratorType &to) // NOLINT
    {
      m_intervals.erase (from, to);
      updateView ();
    } */

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

  /*   auto fileContent{ readFileContent (file) };
    if (fileContent)
      {

        if (filetype == FileType::Erg)
          {
            using namespace textFiles;
            auto returnPair{ processContent (*fileContent, ergFile) };
            auto workout{ getWorkout (returnPair.first, ergFile) };
            auto intervals{ getTextIntervals (returnPair.second, ergFile,
                                              IntensityUnit::Watts,
                                              workout.getFtp ()) };
            if (intervals)
              {
                 //               workout.setIntervals (std::move
                  //(*intervals));
                return workout;
              }
            return std::unexpected (intervals.error ());
          }
        if (filetype == FileType::Mrc)
          {
            using namespace textFiles;
            auto returnPair{ processContent (*fileContent, mrcFile) };
            auto workout{ getWorkout (returnPair.first, mrcFile) };
            auto intervals{ getTextIntervals (returnPair.second, mrcFile,
                                              IntensityUnit::PercentFTP,
                                              workout.getFtp ()) };
            if (intervals)
              {
                //                workout.setIntervals (std::move
                 // (*intervals));
                return workout;
              }
            return std::unexpected (intervals.error ());
          }
        if (filetype == FileType::Plan)
          {
            auto returnPair{ planFiles::splitPlanContent (*fileContent) };
            auto workout{ getWorkout (returnPair.first, planFiles::planFile) };
            auto intervals{ planFiles::getPlanIntervals (returnPair.second,
                                                         workout.getFtp ()) };
            if (intervals)
              {
                //                 workout.setIntervals (std::move
                 // (*intervals));
                return workout;
              }
            return std::unexpected (intervals.error ());
          }
        std::unreachable ();
      }
    return std::unexpected (fileContent.error ()); */
}

/*************************************************************************
/                                                                        /
/                     Free function implementations                      /
/                                                                        /
*************************************************************************/
/*
  Splitting definition and declaration is needed because the declaration is
  using definition of Workout and it needs the  definition of Workout.
*/

/* export void writeWorkout (std::iostream &file,
                          const TextFileFormat &fileformat, Workout &workout)
{
  file << fileformat.headerStart;

  textFiles::writeToStream (file, fileformat.nameTag, workout.getName (),
                            fileformat.headerSeparator);
  if (!fileformat.headerDuration.empty ())
    {
      long workoutDuration{};
      for (const auto &interval : workout)
        {
          workoutDuration += interval.get ().getDuration ().count ();
        }
      textFiles::writeToStream (file, fileformat.headerDuration,
                                std::to_string (workoutDuration).append ("\n"),
                                fileformat.headerSeparator);
    }
  if (!fileformat.headerSpec.empty ())
    {
      file << fileformat.headerSpec;
    }
  textFiles::writeToStream (file, fileformat.noteTag, workout.getNotes (),
                            fileformat.headerSeparator);
  if (!fileformat.intensityUnitTag.empty ())
    {
      textFiles::writeToStream (file, fileformat.intensityUnitTag,
                                std::to_string (workout.getFtp ()),
                                fileformat.headerSeparator);
    }

  file << fileformat.headerEnd;
  double startTime{};
  for (const auto &interval : workout)
    {
      startTime += textFiles::writeIntensityDuration (file, fileformat,
                                                      interval, startTime);
    }
} */

constexpr Workout getWorkout (std::string_view view,
                              const TextFileFormat &format)
{
  Workout workout;
  auto tags{ getTags (view, "=") };
  for (const auto &[key, value] : tags)
    {
      if (key == format.nameTag)
        {
          workout.setName (value);
        }
      else if (key == format.noteTag)
        {
          auto notes = workout.getNotes () + value;
          workout.setNotes (notes);
        }
      else if (key == format.intensityUnitTag)
        {
          workout.setFtp (std::stoi (value));
        }
    }
  return workout;
}

} // namespace Workouts