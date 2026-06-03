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

// A shorter version of std::to_underlying to make the code a bit shorter
template <class Enum>
constexpr auto
enumVal (Enum e) noexcept // NOLINT(readability-identifier-naming)
{ return std::to_underlying (e); }

namespace Workouts
{

export class Interval;
export class Workout;

export enum HRZ : uint8_t {
  H1 = 1, // 50-60% max heart rate
  H2 = 2, // 61-70% max heart rate
  H3 = 3, // 71-80% max heart rate
  H4 = 4, // 81-90% max heart rate
  H5 = 5  // 91-100% max heart rate
};

using ZonePair = std::pair<uint8_t, uint8_t>; // Low and high values of a zone
/*
  Low is the beginning of the target intensity,
  High is the end of the target intensity.
*/
export enum class Level : bool { Low, High };

export struct HRZone
{
  ZonePair Z1{ 50, 60 };
  ZonePair Z2{ 61, 70 };
  ZonePair Z3{ 71, 80 };
  ZonePair Z4{ 81, 90 };
  ZonePair Z5{ 91, 100 };
} const hrZone;

export enum PWZ : uint8_t {
  P1 = 1, // 0-54% FTP
  P2 = 2, // 55-75% FTP
  P3 = 3, // 76-90% FTP
  P4 = 4, // 91-105% FTP
  P5 = 5, // 106-120% FTP
  P6 = 6, // 121-150% FTP
  P7 = 7  // >150% FTP
};

export struct PowerZones
{
  ZonePair Z1{ 0, 54 };
  ZonePair Z2{ 55, 75 };
  ZonePair Z3{ 76, 90 };
  ZonePair Z4{ 91, 105 };
  ZonePair Z5{ 106, 120 };
  ZonePair Z6{ 121, 150 };
  ZonePair Z7{ 151, 200 };
} const pwZone;

/*
  Each Interval can be either heart rate or power based. Each of those can be
  either an absolute value, relative to max heart rate or FTP, or it can be a
  target zone. If it is a relative intensity or a target zone, FTP or max heart
  rate should be provided.
*/
export enum class IntensityUnit : uint8_t {
  Watts,
  PercentFTP,
  PowerZone,
  HeartRateBPM,
  PercentMaxHR,
  HeartRateZone
};

export enum class PowerType : uint8_t { Watts, PercentFTP, PowerZone };

export enum class HeartRateUnit : uint8_t {
  HeartRateBPM,
  PercentMaxHR,
  HeartRateZone
};

// low and high target values
using IntensityPair = std::pair<uint16_t, uint16_t>;
using HeartRatePair = std::pair<uint8_t, uint8_t>;

// The number at which heart rate intensity enum values begin
constexpr const uint8_t heartRateOffset{ 3 };

// The number of different intensity types the IntensityUnit enum holds
constexpr const uint8_t IntensityUnits{ 6 };

// The number of power types
constexpr const uint8_t powerTypes{ 3 };

constexpr const uint8_t HeartRateUnits{ 3 };

using FtpType = uint16_t;
using HrType = uint8_t;
using CapacityT = std::variant<FtpType, HrType>; // FTP or max heart rate

export class Intensity
{
public:
  Intensity () = default;
  void setTarget (uint16_t target, IntensityUnit unit,
                  Level level = Level::Low) noexcept
  {
    m_unit = unit;
    level == Level::Low ? m_target.first = target : m_target.second = target;
  }

  void setTarget (IntensityPair target) noexcept { m_target = target; }

  constexpr uint16_t getTarget (Level level = Level::Low) const noexcept
  { return level == Level::Low ? m_target.first : m_target.second; }

  constexpr std::string getUnitStr () const noexcept
  {
    std::array<std::string, IntensityUnits> units{
      "watts",          "\%FTP", "power zone", "bpm", "\%max heart rate",
      "heart rate zone"
    };
    return units.at (std::to_underlying (m_unit));
  }

  constexpr IntensityUnit getType () const noexcept { return m_unit; }

  void setFTP (uint16_t ftp) noexcept { m_capacity = ftp; }
  void setMaxHeartRate (uint8_t maxHeartRate) noexcept
  { m_capacity = maxHeartRate; }

  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getWatts (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getPercentFTP (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getPowerZone (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getHeartRateBPM (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getPercentMaxHR (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getHeartRateZone (Level level = Level::Low) noexcept
  { return 0; };

private:
  /**
   * @brief Converts an absolute power or heart rate value to a
   * percentage of ftp or max heart rate
   *
   * @param intensity Intensity in watts or bpm
   * @param capacityValue FTP or max heart rate
   * @return constexpr std::expected<uint16_t, std::string>
   */
  static constexpr std::expected<uint16_t, std::string>
  convertToRelative (uint16_t intensity, uint16_t capacityValue) noexcept
  {
    if (capacityValue == 0)
      {
        return std::unexpected ("Please provide a valid ftp or maxHeartRate "
                                "first before setting power or heartrate");
      }
    constexpr uint16_t percent{ 100 };
    intensity *= percent;
    return intensity / capacityValue;
  }

  /**
   * @brief Converts a relative value like ftp or max heart rate back to
   * an absolute power or heart rate value in watts or bpm
   *
   * @param intensity The relative intensity
   * @param value FTP or max heart rate
   * @return constexpr uint16_t
   */
  static constexpr uint16_t convertToAbsolute (uint16_t intensity,
                                               uint16_t value) noexcept
  {
    constexpr double percent{ 100.0 };
    // Do the divsion and multiplication as double and cast the result
    // back to uint16_t
    return static_cast<uint16_t> (static_cast<double> (intensity)
                                  * static_cast<double> (value) / percent);
  }

  static constexpr uint8_t convertToPowerZone (uint16_t intensity,
                                               uint16_t ftp = 0) noexcept
  {
    if (ftp > 0)
      // Intensity is % of FTP
      // Calculate relative power first
      {
        /*         if (auto retVal{ convertToRelative (intensity, ftp) };
           retVal)
                  {
                    intensity = *retVal;
                  } */
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

  static constexpr uint8_t convertFromPowerZone (PWZ zone,
                                                 bool getLower) noexcept
  {
    switch (zone)
      {
      case PWZ::P1: return getLower ? pwZone.Z1.first : pwZone.Z1.second;
      case PWZ::P2: return getLower ? pwZone.Z2.first : pwZone.Z2.second;
      case PWZ::P3: return getLower ? pwZone.Z3.first : pwZone.Z3.second;
      case PWZ::P4: return getLower ? pwZone.Z4.first : pwZone.Z4.second;
      case PWZ::P5: return getLower ? pwZone.Z5.first : pwZone.Z5.second;
      case PWZ::P6: return getLower ? pwZone.Z6.first : pwZone.Z6.second;
      case PWZ::P7: return getLower ? pwZone.Z7.first : pwZone.Z7.second;
      default: std::unreachable;
      }
  }

  static constexpr uint8_t
  convertToHeartRateZone (uint8_t intensity, uint8_t maxHeartRate = 0) noexcept
  {
    if (maxHeartRate > 0)
      {
        /*         if (auto retVal{ convertToRelative (intensity, maxHeartRate)
           }; retVal)
                  {
                    intensity = *retVal;
                  } */
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

  static constexpr uint8_t
  convertFromHeartRateZone (HRZ intensity, bool getLower = true) noexcept
  {
    switch (intensity)
      {
      case HRZ::H1: return getLower ? hrZone.Z1.first : hrZone.Z1.second;
      case HRZ::H2: return getLower ? hrZone.Z2.first : hrZone.Z2.second;
      case HRZ::H3: return getLower ? hrZone.Z3.first : hrZone.Z3.second;
      case HRZ::H4: return getLower ? hrZone.Z4.first : hrZone.Z4.second;
      case HRZ::H5: return getLower ? hrZone.Z5.first : hrZone.Z5.second;
      default: std::unreachable;
      }
  }

private:
  IntensityPair m_target{ 0, 0 };
  IntensityUnit m_unit{ IntensityUnit::Watts };
  Level m_level{ Level::Low };
  CapacityT m_capacity;
};

// private cstor with factory function to enable data checks and return an
// error message on failure
/* export class AbsolutePower
{
public:
  static constexpr std::expected<AbsolutePower, std::string>
  create (PowerPair watts, uint16_t ftp)
  {
    if (watts.first > 0 && watts.second > 0)
      {
        return AbsolutePower{ watts, ftp };
      }
    return std::unexpected (
        "Don't construct AbsolutePower with zero watts. If you don't need a "
        "range, set watts.first = watts.second or call create with a single "
        "uint16_t intensity value.");
  }

  static constexpr std::expected<AbsolutePower, std::string>
  create (uint16_t watts, uint16_t ftp)
  {
    if (watts > 0 && ftp > 0)
      {
        return AbsolutePower{ PowerPair{ watts, watts }, ftp };
      }
    return std::unexpected ("Intensity data ({}) or ftp ({}) are zero. This "
                            "would mess up conversion calculations.");
  }

  auto constexpr getIntensity () const { return m_watts; }
  auto constexpr getCapacity () const { return m_ftp; }

private:
  explicit AbsolutePower (PowerPair watts, uint16_t ftp)
      : m_watts (watts), m_ftp (ftp)
  {
  }
  PowerPair m_watts;
  uint16_t m_ftp;
}; */

// private cstor with factory function to enable data checks and return an
// error message on failure
/* export class RelativePower
{
public:
  static constexpr std::expected<RelativePower, std::string>
  create (PowerPair percentFTP, uint16_t ftp)
  {
    if (percentFTP.first > 0 && percentFTP.second > 0 && ftp > 0)
      {
        return RelativePower{ percentFTP, ftp };
      }
    return std::unexpected (std::format (
        "Invalid power value ({}) or FTP ({}. If you don't need a range of "
        "percentFTP, set percentFTP.first = percentFTP.second.)",
        percentFTP, ftp));
  }

  auto constexpr getIntensity () const { return m_percentFTP; }
  auto constexpr getCapacity () const { return m_ftp; }

private:
  explicit RelativePower (PowerPair percentFTP, uint16_t ftp)
      : m_percentFTP (percentFTP), m_ftp (ftp)
  {
  }
  PowerPair m_percentFTP;
  uint16_t m_ftp;
}; */

// private cstor with factory function to enable data checks and return an
// error message on failure
/* export class PowerZone
{
public:
  static constexpr std::expected<PowerZone, std::string> create (PWZ powerZone,
                                                                 uint16_t ftp)
  {
    if (ftp > 0)
      {
        return PowerZone{ powerZone, ftp };
      }
    return std::unexpected (
        "Please provide a FTP value > 0 when creating a PowerZone.");
  }

  auto constexpr getIntensity () const
  {
    switch (m_powerZone)
      {
      case P1: return pwZone.Z1;
      case P2: return pwZone.Z2;
      case P3: return pwZone.Z3;
      case P4: return pwZone.Z4;
      case P5: return pwZone.Z5;
      case P6: return pwZone.Z6;
      case P7: return pwZone.Z7;
      };
  }
  auto constexpr getCapacity () const { return m_ftp; }

private:
  explicit PowerZone (PWZ powerZone, uint16_t ftp)
      : m_powerZone (powerZone), m_ftp (ftp)
  {
  }
  PWZ m_powerZone;
  uint16_t m_ftp;
}; */

constexpr uint8_t minimalHeartRate{ 30 };
// private cstor with factory function to enable data checks and return an
// error message on failure
/* export class AbsoluteHeartRate
{
public:
  static constexpr std::expected<AbsoluteHeartRate, std::string>
  create (uint8_t bpm, uint8_t maxHeartRate)
  {
    if (bpm >= minimalHeartRate && maxHeartRate > 0)
      {
        return AbsoluteHeartRate{ bpm, maxHeartRate };
      }
    return std::unexpected (
        std::format ("Heart rate and max heart rate must be at least {} bpm.",
                     maxHeartRate));
  }
  auto constexpr getIntensity () const { return m_bpm; }
  auto constexpr getCapacity () const { return m_maxHeartRate; }

private:
  explicit AbsoluteHeartRate (uint8_t bpm, uint8_t maxHeartRate)
      : m_bpm (bpm), m_maxHeartRate (maxHeartRate)
  {
  }
  uint8_t m_bpm;
  uint8_t m_maxHeartRate;
}; */

// private cstor with factory function to enable data checks and return an
// error message on failure
/* export class RelativeHeartRate
{
public:
  static constexpr std::expected<RelativeHeartRate, std::string>
  create (uint8_t percentMaxHeartRate, uint8_t maxHeartRate)
  {
    if (percentMaxHeartRate > 0 && percentMaxHeartRate <= 100
        && maxHeartRate > minimalHeartRate)
      {
        return RelativeHeartRate{ percentMaxHeartRate, maxHeartRate };
      }
    return std::unexpected (std::format (
        "Relative Heart rate must be between 0\% and 100\% and maxHeartRate "
        "must be above the minimal heart rate of {} bpm",
        minimalHeartRate));
  }

  auto constexpr getIntensity () const { return m_percentMaxHeartRate; }
  auto constexpr getCapacity () const { return m_maxHeartRate; }

private:
  explicit RelativeHeartRate (uint8_t percentMaxHeartRate,
                              uint8_t maxHeartRate)
      : m_percentMaxHeartRate (percentMaxHeartRate),
        m_maxHeartRate (maxHeartRate)
  {
  }
  uint8_t m_percentMaxHeartRate;
  uint8_t m_maxHeartRate;
}; */

// private cstor with factory function to enable data checks and return an
// error message on failure
/* export class HeartRateZone
{
public:
  static constexpr std::expected<HeartRateZone, std::string>
  create (HRZ heartRateZone, uint8_t maxHeartRate)
  {
    if (maxHeartRate > minimalHeartRate)
      {
        return HeartRateZone{ heartRateZone, maxHeartRate };
      }
    return std::unexpected (std::format (
        "Max heart rate must be above the minimal heart rate of {} bpm",
        minimalHeartRate));
  }
  auto constexpr getIntensity () const { return enumVal (m_heartRateZone); }
  auto constexpr getCapacity () const { return m_maxHeartRate; }

private:
  explicit HeartRateZone (HRZ heartRateZone, uint8_t maxHeartRate)
      : m_heartRateZone (heartRateZone), m_maxHeartRate (maxHeartRate)
  {
  }
  HRZ m_heartRateZone;
  uint8_t m_maxHeartRate;
}; */

/* export class Capacity
{
public:
  explicit Capacity (uint16_t ftp) : m_value (ftp) {}
  explicit Capacity (uint8_t maxHeartRate) : m_value (maxHeartRate) {}

  std::optional<uint16_t> constexpr getFtp () const
  {
    if (std::holds_alternative<uint16_t> (m_value))
      {
        return std::get<uint16_t> (m_value);
      }
    return std::nullopt;
  }

  std::optional<uint8_t> constexpr getMaxHeartRate () const
  {
    if (std::holds_alternative<uint8_t> (m_value))
      {
        return std::get<uint8_t> (m_value);
      }
    return std::nullopt;
  }

  constexpr void getValue (auto &value) const
  {
    std::visit ([&value] (auto &visit) { value = visit; });
  }

private:
  std::variant<uint8_t, uint16_t> m_value;
}; */

/* template <typename T>
concept PowerConcept = (std::same_as<std::remove_cvref_t<T>, AbsolutePower>
                        || std::same_as<std::remove_cvref_t<T>, RelativePower>
                        || std::same_as<std::remove_cvref_t<T>, PowerZone>);

template <typename T>
concept HeartRateConcept
    = (std::same_as<std::remove_cvref_t<T>, AbsoluteHeartRate>
       || std::same_as<std::remove_cvref_t<T>, RelativeHeartRate>
       || std::same_as<std::remove_cvref_t<T>, HeartRateZone>);

template <typename T>
concept IntensityConcept = requires (T object) {
  object.getIntensity ();
  object.getCapacity ();
}; */

export enum class FileType : uint8_t { Fit, Plan, Erg, Mrc };

using PowerData = std::expected<uint16_t, std::string>;
using HeartRateData = std::expected<uint8_t, std::string>;

using uintType = uint16_t;

EXPORT_TEST using Tag = std::pair<std::string, std::string>;
EXPORT_TEST using Tags = std::vector<Tag>;
EXPORT_TEST using voidReturn = std::expected<void, std::string>;
EXPORT_TEST using uintReturn = std::expected<uintType, std::string>;
EXPORT_TEST using intervalReturn = std::expected<Interval, std::string>;
EXPORT_TEST using Intervals = std::vector<std::unique_ptr<Interval>>;
EXPORT_TEST using IntervalView = std::vector<std::reference_wrapper<Interval>>;
using IteratorType = Intervals::iterator;
using IteratorViewType = IntervalView::iterator;

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
  IntensityUnit type;
};

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

/************************************************************************
 *                                                                       *
 *               Free function forward declarations                      *
 *                                                                       *
 *      All functions which return an Interval have to be declared       *
 *       and defined seperately to break cyclic dependency issues        *
 ************************************************************************/

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
                                          .type = IntensityUnit::Watts };
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
                                          .type = IntensityUnit::PercentFTP };

  constexpr std::expected<std::vector<std::unique_ptr<Interval>>, std::string>
  getTextIntervals (std::string_view intervalView,
                    const TextFileFormat &format, IntensityUnit type,
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
    .type = IntensityUnit::Watts
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

  constexpr std::expected<std::unique_ptr<Interval>, std::string>
  createPlanInterval (std::span<Tag> data, uintType ftp);

  constexpr std::expected<std::vector<std::unique_ptr<Interval>>, std::string>
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
                                 const CapacityT &capacity);
  constexpr fit::WorkoutStepMesg writeFitInterval (const Interval &interval);
}; // fitFiles

/**
 * @brief Holds intensity, duration and a vector of sub-intervals.
 *
 * To construct an Interval, create an instance of PowerAbsolute,
 * PowerRelative, PowerZone, HeartRateAbsolute, HeartRateRelative or
 * HeartRateZone using their create functions. Create a default Interval
 * instance, pass interval and instance to the static set function of Interval.
 *
 * This approach is chosen because it enables the creation of an empty Interval
 * class in case intensity and / or duration are not known at construction of
 * Interval.
 *
 * Additionally this ensures validity of the intensity data and an
 * expressive error message without using exceptions by using std::expected.
 */
class Interval
{
public:
  Interval () = default;

  template <class Rep, class Period>
  constexpr void
  setDuration (const std::chrono::duration<Rep, Period> &duration) noexcept
  { m_duration = std::chrono::duration_cast<std::chrono::seconds> (duration); }

  constexpr std::chrono::seconds getDuration () const noexcept
  { return m_duration; }

  void setIntensity (Intensity &&intensity) noexcept
  { m_intensity = std::make_unique<Intensity> (std::move (intensity)); }

  Intensity *getIntensity () const { return m_intensity.get (); }

  void setRepeats (int repeats) { m_repeats = repeats; }

  void addSubInterval (Interval &&interval)
  { m_subIntervals.emplace_back (std::move (interval)); }

  voidReturn removeSubInterval (std::size_t index) noexcept
  {
    auto it{ m_subIntervals.begin () };
    std::advance (it, index);
    if (it != m_subIntervals.end ())
      {
        m_subIntervals.erase (it);
        return {};
      }
    return std::unexpected (
        std::format ("Interval of index {} does not exist.", index));
  }

  /**
   * @brief Provides an iterable view over Interval, while expanding the
   * repeats.
   * It manages two indices: pos_in_block to cycle through
   * sub-intervals and repeat_index to account for the number of repeats.
   *
   * expandedView implements std::ranges::view_interface, thus enabling
   * range-based iterations using begin() and end().
   *
   */
  struct expandedView : std::ranges::view_interface<expandedView>
  {
    const Interval *self = nullptr;

    struct iterator
    {
      const Interval *self = nullptr;
      std::size_t repeat_index{ 0 };
      std::size_t pos_in_block{ 0 };

      using value_type = const Interval &;
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

    expandedView () = default;
    constexpr expandedView (std::nullptr_t, const Interval *p) : self (p) {}

    iterator begin () const { return { self, 0, 0 }; }
    static std::default_sentinel_t end () { return {}; }
  };

  constexpr auto getIntervalsExpanded () noexcept
  { return expandedView{ nullptr, this }; }

private:
  std::chrono::seconds m_duration{};
  std::vector<Interval> m_subIntervals;
  std::unique_ptr<Intensity> m_intensity = std::make_unique<Intensity> ();
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
                          interval.getIntensity ()->getTarget ());
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

EXPORT_TEST constexpr std::expected<std::ifstream, std::string>
getFileStream (const std::filesystem::path &file) noexcept
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
isValidFit (const std::filesystem::path &file, fit::Decode &decoder) noexcept
{
  return getFileStream (file).transform (
      [&decoder] (auto filestream)
        { return decoder.CheckIntegrity (filestream); });
}

double writeIntensityDuration (std::iostream &file,
                               const TextFileFormat &fileFormat,
                               const Interval &interval, double startTime)
{
  double endTime{ startTime
                  + std::chrono::duration<double, std::ratio<60>> (
                        interval.getDuration ())
                        .count () };
  auto intensityLo{ interval.getIntensity ()->getTarget (Level::Low) };
  auto intensityHi{ interval.getIntensity ()->getTarget (Level::High) };

  file << std::fixed << std::setprecision (3) << startTime << "\t"
       << intensityLo << "\n";
  file << std::fixed << std::setprecision (3) << endTime << "\t" << intensityHi
       << "\n";
  return endTime;
}

constexpr void writeToStream (std::iostream &file, std::string_view key,
                              std::string_view value,
                              std::string_view tagSeparator)
{ file << key << " " << tagSeparator << " " << value << '\n'; }

void writeWorkout (std::iostream &file, const TextFileFormat &fileformat,
                   Workout &workout)
{
  file << fileformat.headerStart;

  writeToStream (file, fileformat.nameTag, workout.getName (),
                 fileformat.headerSeparator);
  if (!fileformat.headerDuration.empty ())
    {
      long workoutDuration{};
      for (const auto &interval : workout)
        {
          workoutDuration += interval.get ().getDuration ().count ();
        }
      writeToStream (file, fileformat.headerDuration,
                     std::to_string (workoutDuration).append ("\n"),
                     fileformat.headerSeparator);
    }
  if (!fileformat.headerSpec.empty ())
    {
      file << fileformat.headerSpec;
    }
  writeToStream (file, fileformat.noteTag, workout.getNotes (),
                 fileformat.headerSeparator);
  if (!fileformat.intensityUnitTag.empty ())
    {
      writeToStream (file, fileformat.intensityUnitTag,
                     std::to_string (workout.getFtp ()),
                     fileformat.headerSeparator);
    }

  file << fileformat.headerEnd;
  double startTime{};
  for (const auto &interval : workout)
    {
      startTime
          += writeIntensityDuration (file, fileformat, interval, startTime);
    }
}

void writeIntensityTime (std::iostream &file, const TextFileFormat &fileFormat,
                         const Interval &interval)
{
  file << fileFormat.intervalTag << "\n\n";
  file << fileFormat.intervalIntensityAbsLoTag << fileFormat.intervalSeparator
       << interval.getIntensity ()->getTarget (Level::Low) << '\n';
  file << fileFormat.intervalIntensityAbsHiTag << fileFormat.intervalSeparator
       << interval.getIntensity ()->getTarget (Level::High) << '\n';
  file << fileFormat.intervalDurationTag << fileFormat.intervalSeparator
       << interval.getDuration ().count () << "?EXIT\n";
}

namespace fitFiles
{
/* Untested!
  void writeCommon (Duration &duration, uint16_t index)
  {
    fit::WorkoutStepMesg workoutStepMsg;
    workoutStepMsg.SetMessageIndex (index);
    workoutStepMsg.SetIntensity (FIT_INTENSITY_ACTIVE);
    workoutStepMsg.SetDurationType (FIT_WKT_STEP_DURATION_TIME);
    workoutStepMsg.SetDurationValue (
        ((duration.Minutes * secInMinute) + duration.Seconds) * msecInSec);
  }
  void writeAbsoluteWatt (fit::Encode &encoder,
                          fit::WorkoutStepMesg &workoutStepMsg,
                          ValueRange &value)
  {
    workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
    if (value.To > 0)
      {
        // Power range
        workoutStepMsg.SetCustomTargetPowerLow (value.From
                                                + AbsolutePowerOffset);
        workoutStepMsg.SetCustomTargetPowerHigh (value.To +
  AbsolutePowerOffset);
      }
    else
      {
        // Power zone
        workoutStepMsg.SetTargetPowerZone (value.From);
      }
    encoder.Write (workoutStepMsg);
  }
  void writePercentFTP (fit::Encode &encoder,
                        fit::WorkoutStepMesg &workoutStepMsg, ValueRange
  &value)
  {
    workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
    if (value.To > 0)
      {
        // power range
        workoutStepMsg.SetCustomTargetPowerLow (value.From);
        workoutStepMsg.SetCustomTargetPowerHigh (value.To);
      }
    else
      {
        // power zone
        workoutStepMsg.SetTargetPowerZone (value.From);
      }
    encoder.Write (workoutStepMsg);
  }
  void writeAbsoluteHeartRate (fit::Encode &encoder,
                              fit::WorkoutStepMesg &workoutStepMsg,
                              ValueRange &value)
  {
    workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
    if (value.To > 0)
      {
        // heart rate range
        workoutStepMsg.SetCustomTargetHeartRateLow (value.From
                                                    +
  AbsoluteHeartRateOffset); workoutStepMsg.SetCustomTargetHeartRateHigh
  (value.To
                                                    +
  AbsoluteHeartRateOffset);
      }
    else
      {
        // heart rate zone
        workoutStepMsg.SetTargetHrZone (value.From);
      }
    encoder.Write (workoutStepMsg);
  }
  void writePercentMaxHeartRate (fit::Encode &encoder,
                                fit::WorkoutStepMesg &workoutStepMsg,
                                ValueRange &value)
  {
    workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
    if (value.To > 0)
      {
        // heart rate range

        workoutStepMsg.SetCustomTargetHeartRateLow (value.From);
        workoutStepMsg.SetCustomTargetHeartRateHigh (value.To);
      }
    else
      {
        // heart rate zone
        workoutStepMsg.SetTargetHrZone (value.From);
      }
    encoder.Write (workoutStepMsg);
  }
  void writeWorkout (std::iostream &file, std::string_view workoutName,
                    uint16_t intervalSteps)
  {
    auto m_encoder = std::make_unique<fit::Encode>
  (fit::ProtocolVersion::V20); m_encoder->Open (file);

    fit::FileIdMesg fileIDMesg;
    fileIDMesg.SetType (FIT_FILE_WORKOUT);
    fileIDMesg.SetManufacturer (FIT_MANUFACTURER_DEVELOPMENT);
    fileIDMesg.SetProduct (1);

    fit::DateTime startTime (std::time (0));
    fileIDMesg.SetTimeCreated (startTime.GetTimeStamp ());

    // Create unique serial number
    srand ((unsigned int)time (nullptr));
    constexpr auto seed{ 10000U };
    fileIDMesg.SetSerialNumber ((rand () % seed) + 1);
    m_encoder->Write (fileIDMesg);

    // Workout Message
    fit::WorkoutMesg workoutMsg;
    workoutMsg.SetWktName (
        std::wstring (workoutName.begin (), workoutName.end ()));
    workoutMsg.SetSport (FIT_SPORT_CYCLING);
    workoutMsg.SetNumValidSteps (intervalSteps);
    m_encoder->Write (workoutMsg);
  }
  void writeWorkout (std::iostream &file, Workout &workout)
  {
    auto workoutName{ workout.getName () };
    auto intervalSteps{ workout.intervalCount () };
    writeWorkout (file, workoutName, intervalSteps);
  }
  struct Listener : public fit::MesgListener
  {
    void OnMesg (fit::Mesg &mesg) override
    {
      auto mesgName = mesg.GetName ();
      std::println ("Name: {}", mesgName);
      if (mesgName == "file_id")
        {
          fit::FileIdMesg fileIdMesg (mesg);
          std::println ("File ID: {}", fileIdMesg.GetSerialNumber ());
          std::println ("Type: {}", fileIdMesg.GetType ());
        }
      else if (mesgName == "workout")
        {
          fit::WorkoutMesg workoutMesg (mesg);
          if (workoutMesg.IsWktNameValid () != 0U)
            {
              auto w_workoutName = workoutMesg.GetWktName ();
              std::string workoutName{ w_workoutName.begin (),
                                      w_workoutName.end () };
              m_workout.setName (workoutName);
            }
          if (workoutMesg.IsWktDescriptionValid () != 0U)
            {
              auto w_description{ workoutMesg.GetWktDescription () };
              std::string description{ w_description.begin (),
                                      w_description.end () };
              m_workout.setNotes (description);
            }
        }
      else if (mesgName == "workout_step")
        {
          fit::WorkoutStepMesg workoutStepMsg (mesg);
          auto targetType{ workoutStepMsg.GetTargetType () };
          std::println ("Target Type: {}", targetType);

          // TODO: Get the real values
          CapacityValues capValues{ .maxHeartRate = 180, .ftp = 300 };

          if (auto interval{ getFitInterval (mesg, capValues) }; interval)
            {
              m_workout.createInterval (*interval);
            }
        }
    }

    Workout getWorkout () { return std::move (m_workout); }
    Workout m_workout;
  };
  std::expected<Workout, std::string> readWorkout (std::istream &file)
  {
    fit::Decode decoder;
    Listener listener;
    decoder.Read (file, listener);
    Workout workout{ listener.getWorkout () };
    return workout;
  }

  // readIntervals does not exist for fit files. It has to be done in
  // readWorkout for fit files.
  }
*/
/*   std::string wrapDescription (std::string_view stringview)
  {
    std::string string (stringview);
    constexpr uint8_t lineLength{ 80 };
    constexpr uint8_t descriptionLength{ 12 };
    for (std::size_t i = 0; i < string.size (); ++i)
      {
        if (i > 0 && (i % lineLength) == 0)
          {
            string.insert (i, "\nDESCRIPTION=");
            i += descriptionLength;
          }
      }
    return string;
  } */
} // namespace fitFiles

constexpr Workout getWorkout (std::string_view view,
                              const TextFileFormat &format)
{
  Workout workout;
  /*   auto tags{ getTags (view, "=") };
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
      } */
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

  // Remove the tag separator, cleanup unnecessary spaces and trailing
  // '\n', split into key and value
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

namespace textFiles
{
constexpr std::expected<std::vector<std::unique_ptr<Interval>>, std::string>
getTextIntervals (std::string_view intervalView, const TextFileFormat &format,
                  IntensityUnit type, uint16_t ftp)
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
      if (type == IntensityUnit::Watts)
        {
          interval.getIntensity ()->setTarget (intensityStart, type,
                                               Level::Low);
          interval.getIntensity ()->setTarget (intensityEnd, type,
                                               Level::High);
          interval.getIntensity ()->setFTP (ftp);
        }
      interval.setDuration (duration);
      return std::unique_ptr<Interval> (&interval);
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
  return std::ranges::to<std::vector<std::unique_ptr<Interval>>> (
      intervalData);
}
} // namespace textFiles

namespace planFiles
{
constexpr std::expected<std::unique_ptr<Interval>, std::string>
createPlanInterval (std::span<Tag> data, uintType ftp)
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
  interval.getIntensity ()->setFTP (ftp);

  for (const auto &[key, value] : data)
    {
      IntensityUnit type{};
      uintType intensity{};
      Level level;
      if (key == planFile.intervalIntensityAbsLoTag)
        {
          type = IntensityUnit::Watts;
          level = Level::Low;
        }
      else if (key == planFile.intervalIntensityAbsHiTag)
        {
          type = IntensityUnit::Watts;
          level = Level::High;
        }
      else if (key == planFile.intervalIntensityRelLoTag)
        {
          type = IntensityUnit::PercentFTP;
          level = Level::Low;
        }
      else if (key == planFile.intervalIntensityRelHiTag)
        {
          type = IntensityUnit::PercentFTP;
          level = Level::High;
        }

      if (auto retVal{ convertNumber (value) }; retVal)
        {
          interval.getIntensity ()->setTarget (*retVal, type, level);
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
  return std::unique_ptr<Interval> (&interval);
}

constexpr std::expected<std::vector<std::unique_ptr<Interval>>, std::string>
getPlanIntervals (std::span<std::string_view> intervalData, uintType ftp)
{
  std::vector<std::unique_ptr<Interval>> intervalVector;
  for (const auto interval : intervalData)
    {
      auto tags{ getTags (interval, planFile.intervalSeparator) };
      if (auto retVal{ createPlanInterval (tags, ftp) }; retVal)
        {
          intervalVector.emplace_back (std::move (*retVal));
        }
      else
        {
          return std::unexpected (retVal.error ());
        }
    }
  return intervalVector;
}
} // namespace planFiles

namespace fitFiles
{

/**
 * @brief Internal helper function used by getFitInterval to set the
 * intensity of the interval from the fit message.
 *
 * @param interval The interval to set the intensity for.
 * @param intensity The numerical intensity value from the fit message. If
 * above the AbsolutePowerOffset or AbsoluteHeartRateOffset, it is an
 * absolute value; otherwise, it is a relative value.
 * @param isLowValue Indicates whether the intensity value is the low or
 * high value of the interval.
 * @param isPower If true, the intensity value is a power value; if false,
 * it is a heart rate value.
 * @return A std::expected<void, std::string>. In case of an error (e.g.
 * invalid intensity value), the error message is returned as a string.
 */
constexpr void applyIntensity (Interval &interval, uintType intensity,
                               Level level, bool isPower)
{
  if (isPower)
    {
      if (intensity >= AbsolutePowerOffset)
        {
          interval.getIntensity ()->setTarget (intensity - AbsolutePowerOffset,
                                               IntensityUnit::Watts, level);
          return;
        }
      interval.getIntensity ()->setTarget (intensity,
                                           IntensityUnit::PercentFTP, level);
      return;
    }
  if (intensity >= AbsoluteHeartRateOffset)
    {
      interval.getIntensity ()->setTarget (intensity - AbsoluteHeartRateOffset,
                                           IntensityUnit::HeartRateBPM, level);
      return;
    }
  interval.getIntensity ()->setTarget (intensity, IntensityUnit::PercentMaxHR,
                                       level);
}

/**
 * @brief Returns an Interval object based on the data from a
 * fit::WorkoutStepMesg
 *
 * @param msg The fit::WorkoutStepMesg containing the interval data.
 * @param capValues This struct holds the ftp and max heart rate values
 * that are needed to convert relative intensity values to absolute values.
 * @return A std::expected<Interval, std::string>. In case of an error
 * (e.g. invalid intensity value), the error message is returned as a
 * string.
 */
intervalReturn getFitInterval (const fit::WorkoutStepMesg &msg,
                               CapacityT capValues)
{
  Interval interval{};
  if (std::holds_alternative<FtpType> (capValues))
    {
      interval.getIntensity ()->setFTP (std::get<FtpType> (capValues));
    }
  else
    {
      interval.getIntensity ()->setMaxHeartRate (std::get<HrType> (capValues));
    }
  if (msg.IsCustomTargetPowerLowValid () != 0U)
    {
      auto intensityLow{ msg.GetCustomTargetPowerLow () };
      applyIntensity (interval, intensityLow, Level::Low, true);
    }
  if (msg.IsCustomTargetPowerHighValid () != 0U)
    {
      auto intensityHigh{ msg.GetCustomTargetPowerHigh () };
      applyIntensity (interval, intensityHigh, Level::High, true);
    }
  if (msg.IsCustomTargetHeartRateLowValid () != 0U)
    {
      auto heartRateLow{ msg.GetCustomTargetHeartRateLow () };
      applyIntensity (interval, heartRateLow, Level::Low, false);
    }
  if (msg.IsCustomTargetHeartRateHighValid () != 0U)
    {
      auto heartRateHigh{ msg.GetCustomTargetHeartRateHigh () };
      applyIntensity (interval, heartRateHigh, Level::High, false);
    }
  if (msg.IsTargetPowerZoneValid () != 0U)
    {
      auto pwrZone{ msg.GetTargetPowerZone () };
      interval.getIntensity ()->setTarget (pwrZone, IntensityUnit::PowerZone,
                                           Level::Low);
    }
  if (msg.IsTargetHrZoneValid () != 0U)
    {
      auto hrZone{ msg.GetTargetHrZone () };
      interval.getIntensity ()->setTarget (
          hrZone, IntensityUnit::HeartRateZone, Level::Low);
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
  auto IntensityUnit{ interval.getIntensity ()->getType () };

  std::pair<uintType, uintType> intensity{
    interval.getIntensity ()->getTarget (Level::Low),
    interval.getIntensity ()->getTarget (Level::High)
  };

  switch (IntensityUnit)
    {
    case IntensityUnit::Watts:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetCustomTargetPowerLow (intensity.first + AbsolutePowerOffset);
      msg.SetCustomTargetPowerHigh (intensity.second + AbsolutePowerOffset);
      break;
    case IntensityUnit::PercentFTP:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetCustomTargetPowerLow (intensity.first);
      msg.SetCustomTargetPowerHigh (intensity.second);
      break;
    case IntensityUnit::HeartRateBPM:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetCustomTargetHeartRateLow (intensity.first
                                       + AbsoluteHeartRateOffset);
      msg.SetCustomTargetHeartRateHigh (intensity.second
                                        + AbsoluteHeartRateOffset);
      break;
    case IntensityUnit::PercentMaxHR:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetCustomTargetHeartRateLow (intensity.first);
      msg.SetCustomTargetHeartRateHigh (intensity.second);
      break;
    case IntensityUnit::PowerZone:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetTargetPowerZone (intensity.first);
      break;
    case IntensityUnit::HeartRateZone:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetTargetHrZone (intensity.first);
      break;
    default: std::unreachable ();
    }
  msg.SetDurationTime (interval.getDuration ().count ());
  return msg;
}
} // namespace fitFiles

} // namespace Workouts