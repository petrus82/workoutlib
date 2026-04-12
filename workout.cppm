export module workoutlib;

import config;
import std;
import fitmodule;
import std.compat;

#if TESTING == TRUE
#define EXPORT_TEST export
#else
#define EXPORT_TEST
#endif

namespace Workouts
{

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
  PowerZoneLow,
  PowerZoneHigh,
  HeartRateAbsLow,
  HeartRateAbsHigh,
  HeartRateRelLow,
  HeartRateRelHigh,
  HeartRateZoneLow,
  HeartRateZoneHigh
};

static const constexpr uint8_t intensityTypes{ 6 };
static const constexpr uint8_t heartRateOffset{ 6 };

using uintType = uint16_t;
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

export class Interval;
export class Workout;
export enum class FileType : uint8_t { Fit, Plan, Erg, Mrc };

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
EXPORT_TEST using Tag = std::pair<std::string, std::string>;
EXPORT_TEST using Tags = std::vector<Tag>;

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
readFileContent (const std::filesystem::path &file);

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
    .nameTag{ "NAME" },
    .headerDuration{ "DURATION" },
    .noteTag{ "DESCRIPTION" },
    .headerSeparator{ "=" },
    .headerEnd{ "=STREAM=" },
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
} // namespace planFiles

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

  void setFtp (uint16_t ftp) { m_ftp = ftp; }
  uintType getFtp () const { return m_ftp; }
  constexpr void setIntensity (uintType intensity, IntensityType type);
  constexpr uintType getIntensity (IntensityType type) const;
  std::chrono::seconds getDuration () const { return m_duration; }

  template <class Rep, class Period>
  void setDuration (std::chrono::duration<Rep, Period> duration)
  { m_duration = std::chrono::duration_cast<std::chrono::seconds> (duration); }

private:
  constexpr void calculatePower (uint16_t power, IntensityType type,
                                 uint16_t ftp);
  constexpr void calculateHeartRate (uint8_t heartRate, IntensityType type,
                                     uint8_t maxHeartRate);
  static constexpr uint16_t convertToRelative (uint16_t intensity,
                                               uint16_t value);
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
  std::array<uint8_t, intensityTypes> m_intensityHeartRate{ 0 };
  std::array<uint16_t, intensityTypes> m_intensityPower{ 0 };
  uint8_t m_maxHeartRate{ 0 };
  uint16_t m_ftp{ 0 };
};
EXPORT_TEST using Intervals = std::vector<Interval>;
using IteratorType = Intervals::iterator;
using WriteFunction = std::function<void (std::iostream &, Interval &,
                                          WorkoutType &, uint16_t)>;
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
  { m_order.emplace_back (interval); }

  constexpr void setIntervals (Intervals intervals)
  {
    m_order.clear ();
    m_order = std::move (intervals);
  }

  constexpr void createRepeat (const IteratorType &from,
                               const IteratorType &to, // NOLINT
                               uint8_t times);
  constexpr void removeIntervals (const IteratorType &from,
                                  const IteratorType &to) // NOLINT
  { m_order.erase (from, to); }

  constexpr auto begin () { return m_order.begin (); }

  static constexpr auto
  next (std::list<std::weak_ptr<Interval>>::iterator current)
  { return std::next (current); }

  static constexpr auto
  prev (std::list<std::weak_ptr<Interval>>::iterator current)
  { return std::prev (current); }

  constexpr auto end () { return m_order.end (); }
  constexpr auto intervalCount () const { return m_order.size (); }
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
  Intervals m_order;
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

constexpr void Interval::setIntensity (uintType intensity, IntensityType type)
{
  if (type >= IntensityType::HeartRateAbsLow)
    {
      calculateHeartRate (intensity, type, m_maxHeartRate);
      return;
    }
  calculatePower (intensity, type, m_ftp);
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

constexpr void Interval::calculatePower (uint16_t power, IntensityType type,
                                         uint16_t ftp)
{
  std::size_t typeValue{ std::to_underlying (type) };
  if (type == IntensityType::PowerAbsLow
      || type == IntensityType::PowerAbsHigh)
    {
      m_intensityPower.at (typeValue) = power;
      m_intensityPower.at (typeValue + 2) = convertToRelative (power, ftp);
      m_intensityPower.at (typeValue + 4) = convertToPowerZone (power, ftp);
      if (type == IntensityType::PowerAbsHigh
          && m_intensityPower.at (typeValue - 1) == 0)
        {
          m_intensityPower.at (typeValue - 1) = power;
        }
    }
  else if (type == IntensityType::PowerRelLow
           || type == IntensityType::PowerRelHigh)
    {
      m_intensityPower.at (typeValue) = power;
      m_intensityPower.at (typeValue - 2) = convertToAbsolute (power, ftp);
      m_intensityPower.at (typeValue + 2) = convertToPowerZone (power);
      if (type == IntensityType::PowerRelHigh
          && m_intensityPower.at (typeValue - 1) == 0)
        {
          m_intensityPower.at (typeValue - 1) = power;
        }
    }
}

constexpr void Interval::calculateHeartRate (uint8_t heartRate,
                                             IntensityType type,
                                             uint8_t maxHeartRate)
{
  std::size_t typeValue{ std::to_underlying (type) };
  if (type == IntensityType::HeartRateAbsLow
      || type == IntensityType::HeartRateAbsHigh)
    {
      m_intensityHeartRate.at (typeValue) = heartRate;
      m_intensityHeartRate.at (typeValue + 2)
          = convertToRelative (heartRate, maxHeartRate);
      m_intensityHeartRate.at (typeValue + 4)
          = convertToHeartRateZone (heartRate, maxHeartRate);
    }
  else if (type == IntensityType::HeartRateRelLow
           || type == IntensityType::HeartRateRelHigh)
    {
      m_intensityHeartRate.at (typeValue) = heartRate;
      m_intensityHeartRate.at (typeValue - 2)
          = convertToAbsolute (heartRate, maxHeartRate);
      m_intensityHeartRate.at (typeValue + 2)
          = convertToHeartRateZone (heartRate, maxHeartRate);
    }
}

constexpr uint16_t Interval::convertToRelative (uint16_t intensity,
                                                uint16_t value)
{
  constexpr uint16_t percent{ 100 };
  intensity *= percent;
  return intensity / value;
}

constexpr uint16_t Interval::convertToAbsolute (uint16_t intensity,
                                                uint16_t value)
{
  constexpr int percent{ 100 };
  return intensity * std::div (value, percent).quot;
}

constexpr uint8_t Interval::convertToPowerZone (uint16_t intensity,
                                                uint16_t ftp)
{
  if (ftp > 0)
    // Intensity is % of FTP
    // Calculate relative power first
    {
      intensity = convertToRelative (intensity, ftp);
    }

  if (intensity < 55)
    {
      return 1;
    }
  else if (intensity >= 55 && intensity <= 75)
    {
      return 2;
    }
  else if (intensity > 75 && intensity <= 90)
    {
      return 3;
    }
  else if (intensity > 90 && intensity <= 105)
    {
      return 4;
    }
  else if (intensity > 105 && intensity <= 120)
    {
      return 5;
    }
  else if (intensity > 120 && intensity <= 150)
    {
      return 6;
    }
  else
    {
      return 7;
    }
}

constexpr uint8_t Interval::convertFromPowerZone (uint8_t intensity,
                                                  bool getLower)
{
  switch (intensity)
    {
    case 1: return getLower ? 0 : 54;
    case 2: return getLower ? 55 : 75;
    case 3: return getLower ? 76 : 90;
    case 4: return getLower ? 91 : 105;
    case 5: return getLower ? 106 : 120;
    case 6: return getLower ? 121 : 150;
    case 7: return getLower ? 151 : 200;
    default: return 0;
    }
}

constexpr uint8_t Interval::convertToHeartRateZone (uint8_t intensity,
                                                    uint8_t maxHeartRate)
{
  if (maxHeartRate > 0)
    {
      intensity = convertToRelative (intensity, maxHeartRate);
    }

  if (intensity > 50 && intensity <= 60)
    {
      return 1;
    }
  if (intensity > 60 && intensity <= 70)
    {
      return 2;
    }
  if (intensity > 70 && intensity <= 80)
    {
      return 3;
    }
  if (intensity > 80 && intensity <= 90)
    {
      return 4;
    }
  if (intensity > 90 && intensity <= 100)
    {
      return 5;
    }
  return 0;
}

constexpr uint8_t Interval::convertFromHeartRateZone (uint8_t intensity,
                                                      bool getLower)
{
  switch (intensity)
    {
    case 1: return getLower ? 50 : 59;
    case 2: return getLower ? 60 : 69;
    case 3: return getLower ? 70 : 79;
    case 4: return getLower ? 80 : 89;
    case 5: return getLower ? 90 : 100;
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
  for (auto &interval : m_order)
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
  m_order.insert_range (from, repeated);
}
/*************************************************************************
/                                                                        /
/                     Free function implementations                      /
/                                                                        /
*************************************************************************/
constexpr std::expected<std::string, std::string>
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
          intervalVector.emplace_back (retVal.value ());
        }
      else
        {
          return std::unexpected (retVal.error ());
        }
    }
  return intervalVector;
}

} // namespace Workouts