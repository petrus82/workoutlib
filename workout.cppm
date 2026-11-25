export module workoutlib;

export import common;
export import intensity;
export import interval;

import trainingdatawriter;

import file_concept;
import filehandling;
import textfiles;
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

export class Workout;

using WorkoutReturn = std::expected<Workout, std::string>;
using StatsPair = std::pair<uintType, uintType>;
using IntensityFactorPair = std::pair<double, double>;

constexpr std::string_view msgNoFtp{
  "Please provide a valid ftp or maxHeartRate "
  "first before setting power or heartrate"
};
constexpr std::string_view msgEmpty{ "No intervals to calculate the TSS." };
constexpr std::string_view msgNoIntensity{
  "Please provide an intensity for every interval before "
  "calculating the TSS."
};
constexpr std::string_view msgNotPower{
  "The TSS can only be calculated from power intervals, "
  "not heart rate intervals."
};
constexpr std::string_view msgTooExtreme{
  "That is something even Tadej Pogačar wouldn't survive, "
  "it is called training, not killing!"
};
constexpr std::string_view msgTooRelaxed{
  "You probably want to train, that is not even relaxing any more!"
};

class Workout
{
public:
  Workout () = default;
  explicit Workout (std::string_view workoutName) : m_workoutName (workoutName)
  {}

  explicit Workout (std::string_view workoutName, std::string_view notes)
      : m_workoutName (workoutName), m_notes (notes)
  {}

  voidReturn saveFile (std::filesystem::path file)
  {
    return getFileType (file).and_then (
        [&file, this] (auto fileType)
          {
            using namespace fitFiles;
            using namespace textFiles;
            using namespace planFiles;
            using namespace ergFiles;
            using namespace mrcFiles;
            switch (fileType)
              {
              case FileType::Fit: return writeFile (FitHandler (file), file);
              case FileType::Plan: return writeFile (PlanHandler (file), file);
              case FileType::Erg: return writeFile (ErgHandler (file), file);
              case FileType::Mrc: return writeFile (MrcHandler (file), file);
              }
          });
  }
  constexpr void addInterval (Interval &&interval)
  {
    invalidateStats ();
    m_intervals.emplace_back (std::move (interval));
  }

  constexpr void setIntervals (Intervals &&intervals)
  {
    invalidateStats ();
    m_intervals.clear ();
    m_intervals = std::move (intervals);
  }

  constexpr void removeIntervals (const Intervals::iterator &from,
                                  // NOLINTNEXTLINE
                                  const Intervals::iterator &to)
  {
    invalidateStats ();
    m_intervals.erase (from, to);
  }

  constexpr std::string getName () const { return m_workoutName; }

  constexpr void setName (std::string_view name) { m_workoutName = name; }

  constexpr std::string getNotes () const { return m_notes; }

  constexpr void setNotes (std::string_view notes) { m_notes = notes; }

  constexpr uint16_t getFtp () const
  { return TrainingData::instance ().getFtp (); }

  constexpr void setFtp (uint16_t ftp) { Workouts::setFtp (ftp); }

  constexpr uint8_t getMaxHeartRate () const
  { return TrainingData::instance ().getMaxHeartRate (); }

  constexpr void setMaxHeartRate (uint8_t heartRate)
  { Workouts::setMaxHeartRate (heartRate); }

  constexpr uint8_t getMinHeartRate () const
  { return TrainingData::instance ().getMinHeartRate (); }

  constexpr void setMinHeartRate (uint8_t heartRate)
  { Workouts::setMinHeartRate (heartRate); }

  // The view is recalculated in cbegin because mutations inside Interval
  // (repeats, sub-intervals, intensity, duration, copy/move) invalidate it
  // and no callback from Interval to Workout exists.
  constexpr auto begin () const
  {
    m_intervalView = std::ranges::views::join (
        std::ranges::ref_view<const Intervals>{ m_intervals });
    return std::ranges::begin (m_intervalView);
  }

  constexpr auto end () const { return std::ranges::end (m_intervalView); }
  auto getIntervals () const { return std::span{ m_intervals }; }

  /**
   * @brief Calculates the workout's normalized power.
   *
   * NP = (Σ P⁴·t / Σt)^(1/4), where P is the interval's power in
   * watts and t its duration in seconds: the fourth root of the sum
   * of every interval's power raised to the fourth, weighted by its
   * duration, divided by the total duration.
   *
   * @param level Intensity level used to evaluate each interval's power.
   * @return The normalized power in watts, or an error message if the
   * workout cannot be evaluated.
   */
  [[nodiscard]] uintReturn getNormalizedPower (Level level = Level::Low)
  {
    return computeStats ().transform (
        [this, level]
          {
            const auto &normalized{ m_normalized.value () };
            return level == Level::Low ? normalized.first : normalized.second;
          });
  }

  /**
   * @brief Calculates the workout's Training Stress Score (TSS).
   *
   * TSS = (totalDuration · NP · IF) / (FTP · secInHour) · 100,
   * where IF = NP / FTP, which simplifies to
   * IF² · totalDuration / secInHour · 100.
   *
   * @param level Intensity level used to evaluate each interval's power.
   * @return The TSS or an error message if the workout cannot be evaluated.
   */
  [[nodiscard]] uintReturn getTSS (Level level = Level::Low)
  {
    return computeStats ().transform (
        [this, level]
          {
            const auto &tss{ m_tss.value () };
            return level == Level::Low ? tss.first : tss.second;
          });
  }

  /**
   * @brief Calculates the workout's intensity factor.
   *
   * The intensity factor is the ratio of the normalized power to the
   * FTP: IF = NP / FTP. Together with the duration it determines the
   * TSS (TSS = IF² · totalDuration / secInHour · 100).
   *
   * @param level Intensity level used to evaluate each interval's power.
   * @return The intensity factor, or an error message if the workout
   * cannot be evaluated.
   */
  [[nodiscard]] doubleReturn getIntensityFactor (Level level = Level::Low)
  {
    return computeStats ().transform (
        [this, level]
          {
            const auto &factor{ m_intensityFactor.value () };
            return level == Level::Low ? factor.first : factor.second;
          });
  }

private:
  EXPOSE_TEST (voidReturn writeFile (FileHandlerC auto &&fileHandler,
                                     const std::filesystem::path &file) {
    fileHandler.setWorkoutName (m_workoutName);
    fileHandler.setWorkoutNotes (m_notes);
    return fileHandler.writeFile (file, m_workoutName, m_notes, m_intervals);
  })

private:
  /**
   * @brief Validates a single interval segment against the global limits.
   *
   * Checks, in this order: that an intensity is present, that it is a
   * power value and not a heart rate, that watts resolve for both levels,
   * that power and duration stay within the limits, and that both levels
   * are at least minPercent of the FTP.
   *
   * @param interval The segment to validate.
   * @param ftp FTP used for the minimum percent check.
   * @return Nothing on success, or the error of the first failed check.
   */
  [[nodiscard]] voidReturn checkInterval (const Interval &interval,
                                          const FtpType ftp) const
  {
    if (interval.getIntensity () == Intensity ())
      {
        return std::unexpected (std::string (msgNoIntensity));
      }
    const Intensity &intensity{ interval.getIntensity () };
    if (enumVal (intensity.getType ()) >= heartRateOffset)
      {
        return std::unexpected (std::string (msgNotPower));
      }
    const auto low{ intensity.getWatts (Level::Low) };
    if (!low.has_value ())
      {
        return std::unexpected (low.error ());
      }
    const auto high{ intensity.getWatts (Level::High) };
    if (!high.has_value ())
      {
        return std::unexpected (high.error ());
      }
    const uint32_t duration{ static_cast<uint32_t> (
        interval.getDuration ().count ()) };
    if (low.value () > maxWatts || high.value () > maxWatts
        || duration > maxDuration)
      {
        return std::unexpected (std::string (msgTooExtreme));
      }
    if (low.value () * minPercent < ftp || high.value () * minPercent < ftp)
      {
        return std::unexpected (std::string (msgTooRelaxed));
      }
    return {};
  }

  /**
   * @brief Computes the workout's normalized power, intensity factor and TSS
   * for both levels and caches the results until the intervals change.
   *
   * Formulas, per level:
   * - Normalized power: NP = (Σ P⁴·t / Σt)^(1/4), where P is the
   *   interval's power in watts and t its duration in seconds
   * - Intensity factor: IF = NP / FTP
   * - Training stress score: TSS = (totalDuration · NP · IF) /
   *   (FTP · secInHour) · 100, which simplifies to
   *   IF² · totalDuration / secInHour · 100.
   *
   * Both levels are validated against the same limits; if either level
   * fails, the whole computation fails and nothing is cached.
   */
  [[nodiscard]] voidReturn computeStats ()
  {
    if (m_normalized.has_value () && m_tss.has_value ()
        && m_intensityFactor.has_value ())
      {
        return {};
      }

    const FtpType ftp{ TrainingData::instance ().getFtp () };
    if (ftp == 0)
      {
        return std::unexpected (std::string (msgNoFtp));
      }
    if (m_intervals.empty ())
      {
        return std::unexpected (std::string (msgEmpty));
      }

    long double sumLow{ 0L };
    long double sumHigh{ 0L };
    uint32_t totalDuration{ 0 };
    // Iterate the flattened interval view so that sub-intervals and
    // repeated blocks are evaluated as individual segments.
    for (const auto &interval : *this)
      {
        const auto check{ checkInterval (interval, ftp) };
        if (!check.has_value ())
          {
            return std::unexpected (check.error ());
          }
        const auto low{ interval.getIntensity ().getWatts (Level::Low) };
        const auto high{ interval.getIntensity ().getWatts (Level::High) };
        const uint32_t duration{ static_cast<uint32_t> (
            interval.getDuration ().count ()) };
        // Power raised to the fourth, weighted by the segment duration:
        // the unnormalized NP numerator.
        sumLow += std::pow (low.value (), 4.0) * duration;
        sumHigh += std::pow (high.value (), 4.0) * duration;
        totalDuration += duration;
      }
    // NP = (Σ P⁴·t / Σt)^(1/4)
    const long double npLow{ std::pow (sumLow / totalDuration, 0.25L) };
    const long double npHigh{ std::pow (sumHigh / totalDuration, 0.25L) };
    const uintType normalizedLow{ static_cast<uintType> (std::round (npLow)) };
    const uintType normalizedHigh{ static_cast<uintType> (
        std::round (npHigh)) };
    // IF = NP / FTP
    const long double ifLow{ npLow / ftp };
    const long double ifHigh{ npHigh / ftp };
    // TSS = (totalDuration · NP · IF) / (FTP · secInHour) · 100,
    // which simplifies to IF² · totalDuration / secInHour · 100
    constexpr int tssScale{ 100 };
    const long double tssLow{ std::pow (ifLow, 2L) * totalDuration / secInHour
                              * tssScale };
    const long double tssHigh{ std::pow (ifHigh, 2L) * totalDuration
                               / secInHour * tssScale };
    constexpr int maxTss{ std::numeric_limits<uintType>::max () };
    if (tssLow > maxTss || tssHigh > maxTss)
      {
        return std::unexpected (std::string (msgTooExtreme));
      }
    m_normalized = { normalizedLow, normalizedHigh };
    m_tss = { static_cast<uintType> (std::round (tssLow)),
              static_cast<uintType> (std::round (tssHigh)) };
    m_intensityFactor
        = { static_cast<double> (ifLow), static_cast<double> (ifHigh) };
    return {};
  }

  void invalidateStats ()
  {
    m_normalized = std::nullopt;
    m_tss = std::nullopt;
    m_intensityFactor = std::nullopt;
  }

private:
  static constexpr uint16_t maxWatts{ 2000 };
  static constexpr uint32_t maxDuration{ 10 * secInHour };
  static constexpr uint16_t minPercent{ 10 };
  std::string m_workoutName;
  std::string m_notes;
  Intervals m_intervals;
  mutable std::ranges::join_view<std::ranges::ref_view<const Intervals>>
      m_intervalView{ std::ranges::views::join (
          std::ranges::ref_view<const Intervals>{ m_intervals }) };
  std::optional<StatsPair> m_normalized;
  std::optional<StatsPair> m_tss;
  std::optional<IntensityFactorPair> m_intensityFactor;
};

EXPORT_TEST [[nodiscard]] WorkoutReturn
readFile (FileHandlerC auto &&fileHandler)
{
  // Executes the checkFile function on the fileHandler. If sucessfull
  // continue with getWorkoutName and getWorkoutNotes. Call setIntervals with
  // fileHandler.getIntervals which collects all intervals from the file. If
  // any of the failible functions return an error, the error message is
  // returned. Otherwise it will be Workout with the data from the file.
  return fileHandler.checkFile ()
      .and_then ([&fileHandler] () { return fileHandler.readFile (); })
      .transform (
          [&fileHandler] ()
            {
              Workout workout{
                fileHandler.getWorkoutName (),
                fileHandler.getWorkoutNotes (),
              };
              workout.setIntervals (fileHandler.getIntervals ());
              return workout;
            });
}

export [[nodiscard]] WorkoutReturn openFile (const std::filesystem::path &file)
{
  return getFileType (file).and_then (
      [&file] (auto fileType)
        {
          using namespace fitFiles;
          using namespace textFiles;
          using namespace planFiles;
          using namespace ergFiles;
          using namespace mrcFiles;
          switch (fileType)
            {
            case FileType::Fit: return readFile (FitHandler (file));
            case FileType::Plan: return readFile (PlanHandler (file));
            case FileType::Erg: return readFile (ErgHandler (file));
            case FileType::Mrc: return readFile (MrcHandler (file));
            }
        });
}

} // namespace Workouts