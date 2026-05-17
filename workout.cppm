export module workoutlib;

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

#if TESTING == TRUE
#define EXPORT_TEST export
#else
#define EXPORT_TEST
#endif

namespace Workouts
{

/*
  Starting with forward declarations and type definitions
*/
export class Interval;
export class Workout;

export enum class WorkoutType : uint8_t {
  AbsoluteWatt,
  PercentFTP,
  AbsoluteHeartRate,
  PercentMaxHeartRate
};

export enum class IntensityType : uint8_t {
  PowerAbsLow,
  PowerAbsHigh,
  PowerRelLow,
  PowerRelHigh,
  PowerZone,
  HeartRateAbsLow,
  HeartRateAbsHigh,
  HeartRateRelLow,
  HeartRateRelHigh,
  HeartRateZone,
};

export enum HRZ : uint8_t { H1 = 1, H2 = 2, H3 = 3, H4 = 4, H5 = 5 };
export enum PWZ : uint8_t {
  P1 = 1,
  P2 = 2,
  P3 = 3,
  P4 = 4,
  P5 = 5,
  P6 = 6,
  P7 = 7
};

using uintType = uint16_t;
using pair = std::pair<uint16_t, uint16_t>;

EXPORT_TEST using Tag = std::pair<std::string, std::string>;
EXPORT_TEST using Tags = std::vector<Tag>;
EXPORT_TEST using voidReturn = std::expected<void, std::string>;
EXPORT_TEST using uintReturn = std::expected<uintType, std::string>;
EXPORT_TEST using intervalReturn = std::expected<Interval, std::string>;
EXPORT_TEST using Intervals = std::vector<Interval>;
using IteratorType = Intervals::iterator;
using WriteFunction = std::function<void (std::iostream &, Interval &,
                                          WorkoutType &, uint16_t)>;

export struct PowerZones
{
  pair Z1{ 0, 54 };
  pair Z2{ 55, 75 };
  pair Z3{ 76, 90 };
  pair Z4{ 91, 105 };
  pair Z5{ 106, 120 };
  pair Z6{ 121, 150 };
  pair Z7{ 151, 200 };
} const pwZone;

export struct HRZone
{
  pair Z1{ 50, 60 };
  pair Z2{ 61, 70 };
  pair Z3{ 71, 80 };
  pair Z4{ 81, 90 };
  pair Z5{ 91, 100 };
} const hrZone;

export constexpr uint8_t zoneIndex{ 4 };
export struct CapacityValues
{
  uint8_t maxHeartRate;
  uint16_t ftp;
};

static const constexpr uint8_t intensityTypes{ 5 };
static const constexpr uint8_t heartRateOffset{ 5 };

// TODO: Check if this is really needed
export struct Duration
{
  uintType Minutes;
  uintType Seconds;
};

export struct ValueRange
{
  uintType From;
  uintType To;
};
// until here

export enum class FileType : uint8_t { Fit, Plan, Erg, Mrc };

/*
  Internal free functions and declarations to handle ERG and MRC files.
*/
EXPORT_TEST struct TextFileFormat
{
  std::string_view headerStart; // Starting sequence
  std::string_view headerSpec;  // Extra sequence, like unit specifications
  std::string_view nameTag;     // Workout name sequence
  std::string_view
      headerDuration;       // Optional duration tag which specifies the total
                            // workout duration, required for plan files
  std::string_view noteTag; // Note sequence
  std::string_view intensityUnitTag;  // Intensity Unit specification
  std::string_view headerSeparator;   // Separates key from values (e.g. '=')
  std::string_view headerEnd;         // Header closing sequence
  std::string_view intervalTag;       // Interval preceding sequence
  std::string_view intervalSeparator; // Separates interval keys from their
                                      // values (e.g. '=' or ':')
  std::string_view subIntervalTag;
  std::string_view repeatTag;
  std::string_view intervalIntensityAbsLoTag; // Intensity specification
  std::string_view intervalIntensityAbsHiTag;
  std::string_view intervalIntensityRelLoTag;
  std::string_view intervalIntensityRelHiTag;
  std::string_view intervalDurationTag; // Duration specification
  IntensityType type;
};

// TODO: Remove that?
EXPORT_TEST constexpr std::string trim (std::string_view string)
{
  const auto first = string.find_first_not_of (' ');
  const auto last = string.find_last_not_of (' ');
  if (first != std::string_view::npos && last != std::string_view::npos)
    {
      return std::string (string.substr (first, last - first + 1));
    }
  return {};
}

EXPORT_TEST constexpr std::expected<std::string, std::string>
readFileContent (const std::filesystem::path &file)
{
  std::ifstream filestream (file);
  if (filestream)
    {
      // Get file size and reserve memory
      filestream.seekg (0, std::ios::end);

      // std::ifstream::read does not take more than std::streamsize for
      // the file size
      auto fileSize
          = static_cast<std::streamsize> (std::filesystem::file_size (file));
      std::string content (fileSize, '\0');

      // Read file into string
      filestream.seekg (0, std::ios::beg);
      filestream.read (content.data (), fileSize);
      return content;
    }
  return std::unexpected ("Cannot open file.");
}

EXPORT_TEST constexpr auto processContent (std::string_view fileContent,
                                           TextFileFormat format)
{
  auto intervalPos = fileContent.find (format.intervalTag);
  std::string_view workout{ fileContent.substr (0, intervalPos) };
  if (workout.starts_with (format.headerStart))
    {
      workout.remove_prefix (format.headerStart.length ());
    }
  intervalPos += format.intervalTag.length ();
  std::string_view intervals{ fileContent.substr (
      intervalPos, fileContent.length () - intervalPos) };
  return std::pair{ workout, intervals };
}

EXPORT_TEST constexpr Workout getWorkout (std::string_view view,
                                          const TextFileFormat &format);

// Used for erg and mrc file content
EXPORT_TEST namespace textFiles
{
  const constexpr TextFileFormat ergFile{ .headerStart{ "[COURSE HEADER]\n"
                                                        "VERSION = 2\n"
                                                        "UNITS = METRIC\n" },
                                          .nameTag{ "FILE NAME" },
                                          .noteTag{ "DESCRIPTION" },
                                          .intensityUnitTag{ "FTP" },
                                          .headerSeparator{ "=" },
                                          .headerEnd{ "MINUTES WATTS\n"
                                                      "[END COURSE HEADER]\n"
                                                      "[COURSE DATA]\n" },
                                          .intervalTag{ "[COURSE DATA]" },
                                          .intervalSeparator{ "\t" },
                                          .type = IntensityType::PowerAbsLow };
  const constexpr TextFileFormat mrcFile{ .headerStart{ "[COURSE HEADER]\n"
                                                        "VERSION = 2\n"
                                                        "UNITS = METRIC\n" },
                                          .nameTag{ "FILE NAME" },
                                          .noteTag{ "DESCRIPTION" },
                                          .headerSeparator{ "=" },
                                          .headerEnd{ "MINUTES PERCENT\n"
                                                      "[END COURSE HEADER]\n"
                                                      "[COURSE DATA]\n" },
                                          .intervalTag{ "[COURSE DATA]" },
                                          .intervalSeparator{ "\t" },
                                          .type = IntensityType::PowerRelLow };

  constexpr std::expected<std::vector<Interval>, std::string>
  getTextIntervals (std::string_view intervalView,
                    const TextFileFormat &format, IntensityType type,
                    uint16_t ftp = 0);
} // namespace textFiles

EXPORT_TEST namespace planFiles
{
  const constexpr TextFileFormat planFile{
    .headerStart{ "=HEADER=\n\n" },
    .headerSpec{ "PLAN_TYPE = 0\nWORKOUT_TYPE = 0\n" },
    .nameTag{ "NAME" },
    .headerDuration{ "DURATION" },
    .noteTag{ "DESCRIPTION" },
    .headerSeparator{ "=" },
    .headerEnd{ "=STREAM=\n\n" },
    .intervalTag{ "=INTERVAL=" },
    .intervalSeparator{ "=" },
    .subIntervalTag{ "=SUBINTERVAL=" },
    .repeatTag{ "=REPEAT=" },
    .intervalIntensityAbsLoTag{ "PWR_LO" },
    .intervalIntensityAbsHiTag{ "PWR_HI" },
    .intervalIntensityRelLoTag{ "PERCENT_FTP_LO" },
    .intervalIntensityRelHiTag{ "PERCENT_FTP_HI" },
    .intervalDurationTag{ "MESG_DURATION_SEC>" },
    .type = IntensityType::PowerAbsHigh
  };

  constexpr auto splitPlanContent (std::string_view fileData)
  {
    constexpr int intervalsTagLength = planFile.headerEnd.length ();
    auto workoutEnd = fileData.find (planFile.headerEnd) + intervalsTagLength;
    auto workout = fileData.substr (0, workoutEnd);
    auto intervals
        = fileData.substr (workoutEnd, fileData.length () - (workoutEnd));

    std::vector<std::string_view> intervalVec;
    size_t previousPos = 0;
    size_t intervalPos = intervals.find (planFile.intervalTag);

    while (intervalPos != std::string_view::npos)
      {
        if ((intervalPos - previousPos) > 3)
          {
            intervalVec.emplace_back (
                intervals.substr (previousPos, intervalPos - previousPos));
          }
        previousPos = intervalPos + planFile.intervalTag.length ();
        intervalPos = intervals.find (planFile.intervalTag, previousPos);
      }
    // Add the remaining part after the last intervalTag
    if (previousPos < intervals.length ())
      {
        intervalVec.emplace_back (intervals.substr (previousPos));
      }

    return std::pair (workout, intervalVec);
  }

  constexpr std::expected<Interval, std::string> createPlanInterval (
      std::span<Tag> data, uintType ftp);

  constexpr std::expected<std::vector<Interval>, std::string>
  getPlanIntervals (std::span<std::string_view> intervalData, uintType ftp);
}; // namespace planFiles

EXPORT_TEST namespace fitFiles
{
  const constexpr int AbsolutePowerOffset = 1000;
  const constexpr int AbsoluteHeartRateOffset = 100;
  // convert from minutes::seconds to msec.
  constexpr const auto msecInSec{ 1000U };
  constexpr const auto secInMinute{ 60U };
  intervalReturn getFitInterval (const fit::WorkoutStepMesg &msg,
                                 CapacityValues &capValues);
  constexpr fit::WorkoutStepMesg writeFitInterval (const Interval &interval);
}; // fitFiles

EXPORT_TEST constexpr Tags getTags (std::string_view data,
                                    std::string_view tagSeparator);

EXPORT_TEST void writeWorkout (std::iostream &file,
                               const TextFileFormat &fileformat,
                               Workout &workout);
EXPORT_TEST double writeIntensityDuration (std::iostream &file,
                                           const TextFileFormat &fileFormat,
                                           const Interval &interval,
                                           IntensityType type,
                                           double startTime);
EXPORT_TEST void writeIntensityTime (std::iostream &file,
                                     const TextFileFormat &fileFormat,
                                     const Interval &interval,
                                     IntensityType type);
EXPORT_TEST
void writeInterval (std::ostream &file, TextFileFormat &fileformat,
                    Interval &interval);

class Interval
{
public:
  Interval () = default;

  explicit Interval (uintType intensity, std::chrono::seconds duration,
                     IntensityType type, uint8_t maxHeartRate)
      : m_type{ type }, m_duration{ duration }, m_maxHeartRate{ maxHeartRate }
  { calculateHeartRate (intensity, type, maxHeartRate); }

  explicit Interval (uintType intensity, std::chrono::seconds duration,
                     IntensityType type, uint16_t ftp)
      : m_type{ type }, m_duration{ duration }, m_ftp{ ftp }
  { calculatePower (intensity, type, ftp); }

  friend bool operator== (const Interval &lhs, const Interval &rhs)
  {
    return std::tie (lhs.m_duration, lhs.m_type, lhs.m_intensityHeartRate,
                     lhs.m_intensityPower, lhs.m_maxHeartRate, lhs.m_ftp,
                     lhs.m_subIntervals, lhs.m_repeats)
           == std::tie (rhs.m_duration, rhs.m_type, rhs.m_intensityHeartRate,
                        rhs.m_intensityPower, rhs.m_maxHeartRate, rhs.m_ftp,
                        rhs.m_subIntervals, rhs.m_repeats);
  }
  void setFtp (uint16_t ftp) { m_ftp = ftp; }
  uintType getFtp () const { return m_ftp; }

  void setMaxHeartRate (uint8_t maxHeartRate)
  { m_maxHeartRate = maxHeartRate; };
  uint8_t getMaxHeartRate () const { return m_maxHeartRate; };
  constexpr voidReturn setIntensity (uintType intensity, IntensityType type);
  uintType getIntensity () { return getIntensity (m_type); }
  constexpr uintType getIntensity (IntensityType type) const;
  constexpr IntensityType getType () const { return m_type; }

  std::chrono::seconds getDuration () const { return m_duration; }

  template <class Rep, class Period>
  void setDuration (std::chrono::duration<Rep, Period> duration)
  { m_duration = std::chrono::duration_cast<std::chrono::seconds> (duration); }

  void setRepeats (int repeats) { m_repeats = repeats; }
  void addSubInterval (Interval &&interval)
  { m_subIntervals.emplace_back (std::move (interval)); }

  void removeSubInterval (std::size_t index)
  {
    auto it{ m_subIntervals.begin () };
    std::advance (it, index);
    if (it != m_subIntervals.end ())
      {
        m_subIntervals.erase (it);
        return;
      }
    throw std::out_of_range (
        std::format ("Interval of index {} does not exist.", index));
  }

  struct expandedView : std::ranges::view_interface<expandedView>
  {
    const Interval *self = nullptr;

    struct iterator
    {
      const Interval *self = nullptr;
      std::size_t repeat_index = 0;
      std::size_t pos_in_block = 0;

      using value_type = const Interval;
      using difference_type = std::ptrdiff_t;

      const Interval &operator* () const
      {
        if (pos_in_block == 0)
          {
            return *self;
          }
        return self->m_subIntervals[pos_in_block - 1];
      }

      iterator &operator++ ()
      {
        ++pos_in_block;
        if (pos_in_block > self->m_subIntervals.size ())
          {
            pos_in_block = 0;
            ++repeat_index;
          }
        return *this;
      }

      void operator++ (int) { ++(*this); }

      friend bool operator== (const iterator &it, std::default_sentinel_t)
      {
        return it.repeat_index
               >= static_cast<std::size_t> (it.self->m_repeats);
      }
    };

    iterator begin () const { return { self, 0, 0 }; }
    static std::default_sentinel_t end () { return {}; }
  };
  constexpr auto getIntervalsExpanded () const
  { return expandedView{ {}, this }; }

private:
  constexpr voidReturn calculatePower (uint16_t power, IntensityType type,
                                       uint16_t ftp);
  constexpr voidReturn calculateHeartRate (uint8_t heartRate,
                                           IntensityType type,
                                           uint8_t maxHeartRate);
  static constexpr std::expected<uint16_t, std::string>
  convertToRelative (uint16_t intensity, uint16_t value);
  static constexpr uint16_t convertToAbsolute (uint16_t intensity,
                                               uint16_t value);
  static constexpr uint8_t convertToPowerZone (uint16_t intensity,
                                               uint16_t ftp = 0);
  static constexpr uint8_t convertFromPowerZone (uint8_t intensity,
                                                 bool getLower = true);
  static constexpr uint8_t convertToHeartRateZone (uint8_t intensity,
                                                   uint8_t maxHeartRate = 0);
  static constexpr uint8_t convertFromHeartRateZone (uint8_t intensity,
                                                     bool getLower = true);

private:
  std::chrono::seconds m_duration{};
  IntensityType m_type{};
  std::array<std::uint8_t, intensityTypes> m_intensityHeartRate{ 0 };
  std::array<std::uint16_t, intensityTypes> m_intensityPower{ 0 };
  uint8_t m_maxHeartRate{ 0 };
  uint16_t m_ftp{ 0 };
  std::vector<Interval> m_subIntervals;
  int m_repeats{ 1 };
};

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
  openFile (const std::filesystem::path &file);

  constexpr std::expected<void, std::string>
  writeFile (std::filesystem::path &file, FileType fileType,
             WorkoutType workoutType, uint16_t relativeTo);

  constexpr void createInterval (Interval &interval)
  { m_intervals.emplace_back (interval); }

  constexpr void setIntervals (Intervals intervals)
  {
    m_intervals.clear ();
    m_intervals = std::move (intervals);
  }

  constexpr void createRepeat (const IteratorType &from,
                               const IteratorType &to, // NOLINT
                               uint8_t times);
  constexpr void removeIntervals (const IteratorType &from,
                                  const IteratorType &to) // NOLINT
  { m_intervals.erase (from, to); }
  constexpr auto getIntervals () const
  {
    return m_intervals
           | std::views::transform (
               [] (const Interval &interval)
                 { return interval.getIntervalsExpanded (); });
  }
  constexpr auto begin () const { return getIntervals ().begin (); }

  constexpr auto end () const { return getIntervals ().end (); }
  constexpr auto intervalCount () const { return m_intervals.size (); }
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
  std::string m_workoutName;
  std::string m_notes;
  uint16_t m_ftp{ 0 };
  uint8_t m_maxHeartRate{ 0 };
  uint8_t m_minHeartRate{ 0 };
  Intervals m_intervals;
};

EXPORT_TEST constexpr std::expected<std::ifstream, std::string>
getFileStream (const std::filesystem::path &file)
{
  std::ifstream filestream (file, std::ios::binary);
  if (!filestream)
    {
      return std::unexpected (
          std::format ("Cannot open file {}", file.string ()));
    }
  return filestream;
}

EXPORT_TEST constexpr std::expected<bool, std::string>
isValidFit (const std::filesystem::path &file, fit::Decode &decoder)
{
  auto filestream{ getFileStream (file) };
  if (filestream)
    {
      return decoder.CheckIntegrity (filestream.value ());
    }
  return std::unexpected (filestream.error ());
}
/*************************************************************************
/                                                                        /
/                         Interval implementations                       /
/                                                                        /
*************************************************************************/

constexpr voidReturn Interval::setIntensity (uintType intensity,
                                             IntensityType type)
{
  if (type >= IntensityType::HeartRateAbsLow)
    {
      return calculateHeartRate (intensity, type, m_maxHeartRate);
    }
  return calculatePower (intensity, type, m_ftp);
}

constexpr uintType Interval::getIntensity (IntensityType type) const
{
  auto typeValue{ std::to_underlying (type) };
  if (typeValue >= heartRateOffset)
    {
      return m_intensityHeartRate.at (typeValue - heartRateOffset);
    }
  return m_intensityPower.at (typeValue);
}

constexpr voidReturn
Interval::calculatePower (uint16_t power, IntensityType type, uint16_t ftp)
{
  std::size_t typeValue{ std::to_underlying (type) };
  if (type == IntensityType::PowerAbsLow
      || type == IntensityType::PowerAbsHigh)
    {

      m_intensityPower.at (typeValue) = power;
      if (auto retVal{ convertToRelative (power, ftp) }; retVal)
        {
          m_intensityPower.at (typeValue + 2) = *retVal;
        }
      else
        {
          return std::unexpected (retVal.error ());
        }
      if (type == IntensityType::PowerAbsHigh
          && m_intensityPower.at (typeValue - 1) == 0)
        {
          m_intensityPower.at (typeValue - 1) = power;
        }
      m_intensityPower.at (zoneIndex) = convertToPowerZone (power, ftp);
      return {};
    }
  if (type == IntensityType::PowerRelLow
      || type == IntensityType::PowerRelHigh)
    {
      m_intensityPower.at (typeValue) = power;
      m_intensityPower.at (typeValue - 2) = convertToAbsolute (power, ftp);
      m_intensityPower.at (zoneIndex) = convertToPowerZone (power);
      if (type == IntensityType::PowerRelHigh
          && m_intensityPower.at (typeValue - 1) == 0)
        {
          m_intensityPower.at (typeValue - 1) = power;
        }
    }
  if (type == IntensityType::PowerZone)
    {
      auto relPwrLow{ convertFromPowerZone (power, true) };
      auto relPwrHigh{ convertFromPowerZone (power, false) };
      m_intensityPower.at (0) = convertToAbsolute (relPwrLow, ftp);
      m_intensityPower.at (1) = convertToAbsolute (relPwrHigh, ftp);
      m_intensityPower.at (2) = relPwrLow;
      m_intensityPower.at (3) = relPwrHigh;
      m_intensityPower.at (4) = power;
    }
  return {};
}

constexpr voidReturn Interval::calculateHeartRate (uint8_t heartRate,
                                                   IntensityType type,
                                                   uint8_t maxHeartRate)
{
  std::size_t typeValue{ static_cast<size_t> (std::to_underlying (type)
                                              - heartRateOffset) };
  if (type == IntensityType::HeartRateAbsLow
      || type == IntensityType::HeartRateAbsHigh)
    {
      m_intensityHeartRate.at (typeValue) = heartRate;
      if (auto retVal{ convertToRelative (heartRate, maxHeartRate) }; retVal)
        {
          m_intensityHeartRate.at (typeValue + 2) = *retVal;
        }
      else
        {
          return std::unexpected (retVal.error ());
        }
      m_intensityHeartRate.at (zoneIndex)
          = convertToHeartRateZone (heartRate, maxHeartRate);
      return {};
    }
  if (type == IntensityType::HeartRateRelLow
      || type == IntensityType::HeartRateRelHigh)
    {
      m_intensityHeartRate.at (typeValue) = heartRate;
      m_intensityHeartRate.at (typeValue - 2)
          = convertToAbsolute (heartRate, maxHeartRate);
      m_intensityHeartRate.at (zoneIndex)
          = convertToHeartRateZone (heartRate, maxHeartRate);
    }
  if (type == IntensityType::HeartRateZone)
    {
      auto hrLow{ convertFromHeartRateZone (heartRate, true) };
      auto hrHigh{ convertFromHeartRateZone (heartRate, false) };
      m_intensityHeartRate.at (0) = convertToAbsolute (hrLow, maxHeartRate);
      m_intensityHeartRate.at (1) = convertToAbsolute (hrHigh, maxHeartRate);
      m_intensityHeartRate.at (2) = hrLow;
      m_intensityHeartRate.at (3) = hrHigh;
      m_intensityHeartRate.at (4) = heartRate;
    }
  return {};
}

constexpr std::expected<uint16_t, std::string>
Interval::convertToRelative (uint16_t intensity, uint16_t value)
{
  if (value == 0)
    {
      return std::unexpected ("Please provide a valid ftp or maxHeartRate "
                              "first before setting power or heartrate");
    }
  constexpr uint16_t percent{ 100 };
  intensity *= percent;
  return intensity / value;
}

constexpr uint16_t Interval::convertToAbsolute (uint16_t intensity,
                                                uint16_t value)
{
  constexpr double percent{ 100.0 };
  // Do the divsion and multiplication as double and cast the result back to
  // uint16_t
  return static_cast<uint16_t> (static_cast<double> (intensity)
                                * static_cast<double> (value) / percent);
}

constexpr uint8_t Interval::convertToPowerZone (uint16_t intensity,
                                                uint16_t ftp)
{
  if (ftp > 0)
    // Intensity is % of FTP
    // Calculate relative power first
    {
      if (auto retVal{ convertToRelative (intensity, ftp) }; retVal)
        {
          intensity = *retVal;
        }
    }

  if (intensity <= pwZone.Z1.second)
    {
      return PWZ::P1;
    }
  if (intensity <= pwZone.Z2.second)
    {
      return PWZ::P2;
    }
  if (intensity <= pwZone.Z3.second)
    {
      return PWZ::P3;
    }
  if (intensity <= pwZone.Z4.second)
    {
      return PWZ::P4;
    }
  if (intensity <= pwZone.Z5.second)
    {
      return PWZ::P5;
    }
  if (intensity <= pwZone.Z6.second)
    {
      return PWZ::P6;
    }
  return PWZ::P7;
}

constexpr uint8_t Interval::convertFromPowerZone (uint8_t intensity,
                                                  bool getLower)
{
  switch (intensity)
    {
    case PWZ::P1: return getLower ? pwZone.Z1.first : pwZone.Z1.second;
    case PWZ::P2: return getLower ? pwZone.Z2.first : pwZone.Z2.second;
    case PWZ::P3: return getLower ? pwZone.Z3.first : pwZone.Z3.second;
    case PWZ::P4: return getLower ? pwZone.Z4.first : pwZone.Z4.second;
    case PWZ::P5: return getLower ? pwZone.Z5.first : pwZone.Z5.second;
    case PWZ::P6: return getLower ? pwZone.Z6.first : pwZone.Z6.second;
    case PWZ::P7: return getLower ? pwZone.Z7.first : pwZone.Z7.second;
    default: return 0;
    }
}

constexpr uint8_t Interval::convertToHeartRateZone (uint8_t intensity,
                                                    uint8_t maxHeartRate)
{
  if (maxHeartRate > 0)
    {
      if (auto retVal{ convertToRelative (intensity, maxHeartRate) }; retVal)
        {
          intensity = *retVal;
        }
    }

  if (intensity > hrZone.Z1.first && intensity <= hrZone.Z1.second)
    {
      return HRZ::H1;
    }
  if (intensity > hrZone.Z2.first && intensity <= hrZone.Z2.second)
    {
      return HRZ::H2;
    }
  if (intensity > hrZone.Z3.first && intensity <= hrZone.Z3.second)
    {
      return HRZ::H3;
    }
  if (intensity > hrZone.Z4.first && intensity <= hrZone.Z4.second)
    {
      return HRZ::H4;
    }
  if (intensity > hrZone.Z5.first && intensity <= hrZone.Z5.second)
    {
      return HRZ::H5;
    }
  return 0;
}

constexpr uint8_t Interval::convertFromHeartRateZone (uint8_t intensity,
                                                      bool getLower)
{
  switch (intensity)
    {
    case HRZ::H1: return getLower ? hrZone.Z1.first : hrZone.Z1.second;
    case HRZ::H2: return getLower ? hrZone.Z2.first : hrZone.Z2.second;
    case HRZ::H3: return getLower ? hrZone.Z3.first : hrZone.Z3.second;
    case HRZ::H4: return getLower ? hrZone.Z4.first : hrZone.Z4.second;
    case HRZ::H5: return getLower ? hrZone.Z5.first : hrZone.Z5.second;
    default: return 0;
    }
}

/*************************************************************************
/                                                                        /
/                          Workout implementations                       /
/                                                                        /
*************************************************************************/
[[nodiscard]] constexpr std::expected<Workout, std::string>
Workout::openFile (const std::filesystem::path &file)
{
  static constexpr std::array fileextensions{ ".fit", ".plan", ".erg",
                                              ".mrc" };

  const auto *const it = std::find (fileextensions.begin (),
                                    fileextensions.end (), file.extension ());
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
                                            IntensityType::PowerAbsHigh,
                                            workout.getFtp ()) };
          if (intervals)
            {
              workout.setIntervals (*intervals);
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
                                            IntensityType::PowerRelHigh,
                                            workout.getFtp ()) };
          if (intervals)
            {
              workout.setIntervals (*intervals);
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
              workout.setIntervals (*intervals);
              return workout;
            }
          return std::unexpected (intervals.error ());
        }
      std::unreachable ();
    }
  return std::unexpected (fileContent.error ());
}

constexpr std::expected<void, std::string>
Workout::writeFile (std::filesystem::path &file, FileType fileType,
                    WorkoutType workoutType, uint16_t relativeTo)
{
  WriteFunction writeFunc;
  std::fstream filestream (file, std::ios::out);
  if (filestream.fail ())
    {
      std::error_code error;
      return std::unexpected (std::format ("Cannot open file {}. {}",
                                           file.string (), error.message ()));
    }
  /*     switch (fileType)
        {
        case FileType::Erg:
          writeFunc = [] (std::iostream &filestream, Interval &interval,
                          WorkoutType type, uint16_t relativeTo)
            { ErgFile::writeInterval (filestream, interval, type,
     relativeTo); }; ErgFile::writeWorkout (filestream, *this); break; case
     FileType::Fit: writeFunc = [] (std::iostream &filestream, Interval
     &interval, WorkoutType type, uint16_t relativeTo) {
     FitFile::writeInterval (filestream, interval, type, relativeTo); };
          filestream.open (file, std::ios::out | std::ios::binary);
          FitFile::writeWorkout (filestream, *this);
          break;
        case FileType::Mrc:
          writeFunc = [] (std::iostream &filestream, Interval &interval,
                          WorkoutType type, uint16_t relativeTo)
            { MrcFile::writeInterval (filestream, interval, type,
     relativeTo); }; MrcFile::writeWorkout (filestream, *this); break; case
     FileType::Plan: writeFunc = [] (std::iostream &filestream, Interval
     &interval, WorkoutType type, uint16_t relativeTo)
            {
              PlanFile::writeInterval (filestream, interval, type,
     relativeTo);
            };
          PlanFile::writeWorkout (filestream, *this);
          break;
        } */
  for (auto &interval : m_intervals)
    {
      writeFunc (filestream, interval, workoutType, relativeTo);
    }
  return {};
}

constexpr void Workout::createRepeat (const IteratorType &from,
                                      const IteratorType &to, // NOLINT
                                      uint8_t times)
{
  auto range = std::ranges::subrange (from, to);
  auto repeated = std::views::repeat (range, times) | std::views::join;
  m_intervals.insert_range (from, repeated);
}
/*************************************************************************
/                                                                        /
/                     Free function implementations                      /
/                                                                        /
*************************************************************************/

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

constexpr Tags getTags (std::string_view data, std::string_view tagSeparator)
{
  auto wordDelim = [tagSeparator] (auto first, auto second)
    { return !(first == '\n' || second == *tagSeparator.data ()); };

  auto isUpperCase = [] (auto word)
    {
      bool upperCase{ true };
      for (const auto character : word)
        {
          if (std::isalnum (character))
            {
              if (!(std::isupper (character)) && upperCase)
                {
                  upperCase = false;
                }
            }
        }
      return upperCase;
    };

  // Split by newline and by tag separator (e.g. '=')
  auto words{ data | std::views::chunk_by (wordDelim)
              | std::views::transform (
                  [] (auto line) { return std::string_view (line); }) };

  // Remove the tag separator, cleanup unnecessary spaces and trailing '\n',
  // split into key and value
  auto chunks = words
                | std::views::transform (
                    [tagSeparator] (auto word)
                      {
                        std::string_view wordString (word);
                        if (wordString.starts_with (tagSeparator))
                          {
                            wordString.remove_prefix (1);
                          }
                        if (wordString.starts_with (' '))
                          {
                            wordString.remove_prefix (1);
                          }
                        if (wordString.ends_with ('\n'))
                          {
                            wordString.remove_suffix (1);
                          }
                        else if (wordString.ends_with (' '))
                          {
                            wordString.remove_suffix (1);
                          }

                        return wordString;
                      })
                | std::views::chunk_by (
                    [&] (auto first, auto second)
                      { return isUpperCase (first) == isUpperCase (second); });

  // Keys are the odd chunks, values are uneven
  // Flatten both
  auto keys = chunks | std::views::stride (2);
  auto joinSubranges = [] (auto &&range)
    {
      return std::ranges::fold_left (
          range, std::string{},
          [] (std::string_view first, std::string_view second)
            { return std::string (first).append (second); });
    };
  auto values = chunks | std::views::drop (1) | std::views::stride (2);
  auto joinedKeys
      = std::ranges::subrange (keys) | std::views::transform (joinSubranges);
  auto joinedValues
      = std::ranges::subrange (values) | std::views::transform (joinSubranges);

  // return a vector of std::pairs with key, value
  return std::ranges::to<Tags> (std::views::zip (joinedKeys, joinedValues)
                                | std::views::transform (
                                    [] (auto data)
                                      {
                                        auto [key, value] = data;
                                        return std::pair (std::string (key),
                                                          std::string (value));
                                      }));
}

constexpr std::expected<std::vector<Interval>, std::string>
textFiles::getTextIntervals (std::string_view intervalView,
                             const TextFileFormat &format, IntensityType type,
                             uint16_t ftp)
{
  constexpr auto intervalDelim
      = [] (auto x, auto y) { return !(x == '\n' || y == '\t'); }; // NOLINT
  constexpr auto cleanup = [] (auto line)
    {
      auto string{ std::string_view (line) };
      if (string.ends_with ('\n'))
        {
          string.remove_suffix (1);
        }
      if (string.starts_with ('\t'))
        {
          string.remove_prefix (1);
        }
      return string;
    };
  constexpr auto convert2seconds = [] (auto elem)
    {
      constexpr int secondsInMinute{ 60 };
      double timeD{ std::stod (std::string (elem)) };
      auto minutes{
        std::chrono::duration<double, std::ratio<secondsInMinute>> (timeD)
      };
      return std::chrono::duration_cast<std::chrono::seconds> (minutes);
    };
  auto createIntervalData = [&] (auto data)
    {
      auto &[start, end, intensityStart, intensityEnd] = data;
      auto duration = end - start;
      Interval interval;
      interval.setFtp (ftp);
      interval.setIntensity (intensityEnd, type);
      interval.setDuration (duration);
      if (type == IntensityType::PowerAbsHigh
          || type == IntensityType::PowerRelHigh
          || type == IntensityType::HeartRateAbsHigh
          || type == IntensityType::HeartRateRelHigh)
        {
          auto typeLow
              = static_cast<IntensityType> (std::to_underlying (type) - 1);
          interval.setIntensity (intensityStart, typeLow);
        }
      return interval;
    };

  // Every Interval consists of two lines.
  // The first specifies the intensity at beginning of the interval, the
  // second line is the intensity at the end of the interval

  // First get a view of all intervals
  auto intervals{ intervalView | std::views::chunk_by (intervalDelim)
                  | std::views::transform (cleanup)
                  | std::views::filter ([] (auto line)
                                          { return !line.empty (); }) };

  // Every second odd entry is a time, convert it to seconds
  auto times = intervals | std::views::stride (2)
               | std::views::transform (convert2seconds);

  // Every second odd time entry is a start time
  auto startTime = times | std::views::stride (2);

  // Every second uneven time entry is an end time
  auto endTime = times | std::views::drop (1) | std::views::stride (2);

  // Every second uneven entry is an intensity, convert it to int
  auto intensities = intervals | std::views::drop (1) | std::views::stride (2)
                     | std::views::transform (
                         [] (auto intensity)
                           { return std::stoi (std::string (intensity)); });

  // Every second odd intensity is the intensity at the beginning of the
  // interval
  auto intensityStart = intensities | std::views::stride (2);

  // Every second unveven intensity is the intensity at the end of the
  // interval
  auto intensityEnd
      = intensities | std::views::drop (1) | std::views::stride (2);

  // Generate a std::tuple of all interval data and create an interval
  auto intervalData
      = std::views::zip (startTime, endTime, intensityStart, intensityEnd)
        | std::views::transform (createIntervalData);

  // return a vector with all intervals constructed
  return std::ranges::to<std::vector<Interval>> (intervalData);
}

constexpr std::expected<Interval, std::string>
planFiles::createPlanInterval (std::span<Tag> data, uintType ftp)
{
  auto convertNumber
      = [&] (std::string_view string) -> std::expected<uintType, std::string>
    {
      uintType result{};
      auto [ptr,
            error]{ std::from_chars (string.data (),
                                     string.data () + string.size (), // NOLINT
                                     result) };
      if (error != std::errc{})
        {
          return std::unexpected (
              std::format ("Cannot convert string {} to number", string));
        }
      return result;
    };
  Interval interval;
  interval.setFtp (ftp);

  for (const auto &[key, value] : data)
    {
      IntensityType type{};
      uintType intensity{};
      if (key == planFile.intervalIntensityAbsLoTag)
        {
          type = IntensityType::PowerAbsLow;
        }
      else if (key == planFile.intervalIntensityAbsHiTag)
        {
          type = IntensityType::PowerAbsHigh;
        }
      else if (key == planFile.intervalIntensityRelLoTag)
        {
          type = IntensityType::PowerRelLow;
        }
      else if (key == planFile.intervalIntensityRelHiTag)
        {
          type = IntensityType::PowerRelHigh;
        }

      if (auto retVal{ convertNumber (value) }; retVal)
        {
          interval.setIntensity (*retVal, type);
        }
      else
        {
          return std::unexpected (retVal.error ());
        }
      if (key == planFile.intervalDurationTag)
        {
          int result{};
          if (auto [ptr, error]
              = std::from_chars (value.data (),
                                 value.data () + value.size (), // NOLINT
                                 result);
              error == std::errc{})
            {
              std::chrono::seconds seconds{ std::chrono::seconds (result) };
              interval.setDuration (seconds);
            }
          else
            {
              return std::unexpected (
                  std::format ("Cannot convert time from string {}", value));
            }
        }
    }
  return interval;
}

constexpr std::expected<std::vector<Interval>, std::string>
planFiles::getPlanIntervals (std::span<std::string_view> intervalData,
                             uintType ftp)
{
  std::vector<Interval> intervalVector;
  for (const auto interval : intervalData)
    {
      auto tags{ getTags (interval, planFile.intervalSeparator) };
      if (auto retVal{ createPlanInterval (tags, ftp) }; retVal)
        {
          intervalVector.emplace_back (*retVal);
        }
      else
        {
          return std::unexpected (retVal.error ());
        }
    }
  return intervalVector;
}

namespace fitFiles
{

/**
 * @brief Internal helper function used by getFitInterval to set the intensity
 * of the interval from the fit message.
 *
 * @param interval The interval to set the intensity for.
 * @param intensity The numerical intensity value from the fit message. If
 * above the AbsolutePowerOffset or AbsoluteHeartRateOffset, it is an absolute
 * value; otherwise, it is a relative value.
 * @param isLowValue Indicates whether the intensity value is the low or high
 * value of the interval.
 * @param isPower If true, the intensity value is a power value; if false, it
 * is a heart rate value.
 * @return A std::expected<void, std::string>. In case of an error (e.g.
 * invalid intensity value), the error message is returned as a string.
 */
constexpr voidReturn applyIntensity (Interval &interval, uintType intensity,
                                     bool isLowValue, bool isPower)
{
  IntensityType type{};
  if (isPower)
    {
      if (intensity >= AbsolutePowerOffset)
        {
          type = isLowValue ? IntensityType::PowerAbsLow
                            : IntensityType::PowerAbsHigh;
          return interval.setIntensity (intensity - AbsolutePowerOffset, type);
        }
      type = isLowValue ? IntensityType::PowerRelLow
                        : IntensityType::PowerRelHigh;
      return interval.setIntensity (intensity, type);
    }
  if (intensity >= AbsoluteHeartRateOffset)
    {
      type = isLowValue ? IntensityType::HeartRateAbsLow
                        : IntensityType::HeartRateAbsHigh;
      return interval.setIntensity (intensity - AbsoluteHeartRateOffset, type);
    }
  type = isLowValue ? IntensityType::HeartRateRelLow
                    : IntensityType::HeartRateRelHigh;
  return interval.setIntensity (intensity, type);
}

/**
 * @brief Returns an Interval object based on the data from a
 * fit::WorkoutStepMesg
 *
 * @param msg The fit::WorkoutStepMesg containing the interval data.
 * @param capValues This struct holds the ftp and max heart rate values that
 * are needed to convert relative intensity values to absolute values.
 * @return A std::expected<Interval, std::string>. In case of an error (e.g.
 * invalid intensity value), the error message is returned as a string.
 */
intervalReturn getFitInterval (const fit::WorkoutStepMesg &msg,
                               CapacityValues &capValues)
{
  Interval interval{};
  interval.setFtp (capValues.ftp);
  interval.setMaxHeartRate (capValues.maxHeartRate);
  if (msg.IsCustomTargetPowerLowValid () != 0U)
    {
      auto intensityLow{ msg.GetCustomTargetPowerLow () };
      if (auto retVal{ applyIntensity (interval, intensityLow, true, true) };
          !retVal)
        {
          return std::unexpected (retVal.error ());
        }
    }
  if (msg.IsCustomTargetPowerHighValid () != 0U)
    {
      auto intensityHigh{ msg.GetCustomTargetPowerHigh () };
      if (auto retVal{ applyIntensity (interval, intensityHigh, false, true) };
          !retVal)
        {
          return std::unexpected (retVal.error ());
        }
    }
  if (msg.IsCustomTargetHeartRateLowValid () != 0U)
    {
      auto heartRateLow{ msg.GetCustomTargetHeartRateLow () };
      if (auto retVal{ applyIntensity (interval, heartRateLow, true, false) };
          !retVal)
        {
          return std::unexpected (retVal.error ());
        }
    }
  if (msg.IsCustomTargetHeartRateHighValid () != 0U)
    {
      auto heartRateHigh{ msg.GetCustomTargetHeartRateHigh () };
      if (auto retVal{
              applyIntensity (interval, heartRateHigh, false, false) };
          !retVal)
        {
          return std::unexpected (retVal.error ());
        }
    }
  if (msg.IsTargetPowerZoneValid () != 0U)
    {
      auto pwrZone{ msg.GetTargetPowerZone () };
      interval.setIntensity (pwrZone, IntensityType::PowerZone);
    }
  if (msg.IsTargetHrZoneValid () != 0U)
    {
      auto hrZone{ msg.GetTargetHrZone () };
      interval.setIntensity (hrZone, IntensityType::HeartRateZone);
    }
  if (msg.IsDurationTimeValid () != 0U)
    {
      auto duration{ msg.GetDurationValue () };
      interval.setDuration (std::chrono::seconds (duration));
    }
  return interval;
}
constexpr fit::WorkoutStepMesg writeFitInterval (const Interval &interval)
{
  fit::WorkoutStepMesg msg;
  msg.SetIntensity (FIT_INTENSITY_ACTIVE);
  msg.SetDurationType (FIT_WKT_STEP_DURATION_TIME);
  auto intensityType{ interval.getType () };
  auto intensity{ interval.getIntensity (IntensityType (intensityType)) };

  switch (intensityType)
    {
    case IntensityType::PowerAbsLow:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetCustomTargetPowerLow (intensity + AbsolutePowerOffset);
      break;
    case IntensityType::PowerAbsHigh:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetCustomTargetPowerHigh (intensity + AbsolutePowerOffset);
      break;
    case IntensityType::PowerRelLow:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetCustomTargetPowerLow (intensity);
      break;
    case IntensityType::PowerRelHigh:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetCustomTargetPowerHigh (intensity);
      break;
    case IntensityType::HeartRateAbsLow:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetCustomTargetHeartRateLow (intensity + AbsoluteHeartRateOffset);
      break;
    case IntensityType::HeartRateAbsHigh:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetCustomTargetHeartRateHigh (intensity + AbsoluteHeartRateOffset);
      break;
    case IntensityType::HeartRateRelLow:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetCustomTargetHeartRateLow (intensity);
      break;
    case IntensityType::HeartRateRelHigh:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetCustomTargetHeartRateHigh (intensity);
      break;
    case IntensityType::PowerZone:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetTargetPowerZone (intensity);
      break;
    case IntensityType::HeartRateZone:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetTargetHrZone (intensity);
      break;
    default: std::unreachable ();
    }
  msg.SetDurationTime (interval.getDuration ().count ());
  return msg;
}
} // namespace fitFiles
} // namespace Workouts