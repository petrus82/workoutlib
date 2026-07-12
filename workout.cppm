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
export class Workout;

using PowerData = std::expected<uint16_t, std::string>;
using HeartRateData = std::expected<uint8_t, std::string>;

export constexpr Workout getWorkout (std::string_view view,
                                     const TextFileFormat &format);

class Workout
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

  [[nodiscard]] static constexpr std::expected<Workout, std::string>
  openFile (const std::filesystem::path &file)
  {
    static constexpr std::array fileextensions{ ".fit", ".plan", ".erg",
                                                ".mrc" };

    const auto *const it = std::find (
        fileextensions.begin (), fileextensions.end (), file.extension ());
    if (it == fileextensions.end ())
      {
        return std::unexpected (std::format (
            "No valid Workout file extension for file {}.", file.string ()));
      }

    const auto filetype
        = static_cast<FileType> (std::distance (fileextensions.begin (), it));
    if (filetype == FileType::Fit)
      {
        std::ifstream filestream (file, std::ios::binary);
        return Workout ();
      }
    auto fileContent{ readFileContent (file) };
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
                workout.setIntervals (std::move (*intervals));
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
                workout.setIntervals (std::move (*intervals));
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
                workout.setIntervals (std::move (*intervals));
                return workout;
              }
            return std::unexpected (intervals.error ());
          }
        std::unreachable ();
      }
    return std::unexpected (fileContent.error ());
  }

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

  constexpr void createInterval (Interval &&interval)
  {
    m_intervals.emplace_back (
        std::make_unique<Interval> (std::move (interval)));
  }

  constexpr void setIntervals (Intervals &&intervals)
  {
    m_intervals.clear ();
    m_intervals = std::move (intervals);
  }

  constexpr void createRepeat (const IteratorViewType &from,
                               const IteratorViewType &to, // NOLINT
                               uint8_t times)
  {
    updateView ();
    auto range = std::ranges::subrange (from, to);
    auto repeated = std::views::repeat (range, times) | std::views::join;
    m_intervalView.insert_range (from, repeated);
  }

  constexpr void removeIntervals (const IteratorType &from,
                                  const IteratorType &to) // NOLINT
  {
    m_intervals.erase (from, to);
    updateView ();
  }

  constexpr auto begin () { return m_intervalView.begin (); }

  constexpr auto end () { return m_intervalView.end (); }

  constexpr auto intervalCount () const { return m_intervalView.size (); }

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

private:
  constexpr auto getIntervals ()
  {
    auto intervalView{ m_intervalView
                       | std::views::transform (
                           [] (Interval &interval)
                             { return interval.getIntervalsExpanded (); }) };

    m_expanded.clear ();
    for (const auto &view : intervalView)
      {
        static int viewCount{};
        std::println ("View {}:", viewCount++);

        for (const auto &interval : view)
          {
            static int intervalCount{};
            std::println ("Interval {}: duration {}s, intensity {}",
                          intervalCount++, interval.getDuration ().count (),
                          interval.getIntensity ().getTarget ());
            m_expanded.push_back (interval);
          }
      }
    return m_expanded;
  }

  void updateView ()
  {
    m_intervalView.clear ();
    std::ranges::for_each (m_intervals, [this] (const auto &interval)
                             { m_intervalView.emplace_back (*interval); });
  }

private:
  std::string m_workoutName;
  std::string m_notes;
  uint16_t m_ftp{ 0 };
  uint8_t m_maxHeartRate{ 0 };
  uint8_t m_minHeartRate{ 0 };
  Intervals m_intervals;
  IntervalView m_intervalView;
  std::vector<std::reference_wrapper<const Interval>> m_expanded;
};

/*************************************************************************
/                                                                        /
/                     Free function implementations                      /
/                                                                        /
*************************************************************************/
/*
  Splitting definition and declaration is needed because the declaration is
  using definition of Workout and it needs the  definition of Workout.
*/

export void writeWorkout (std::iostream &file,
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
}

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