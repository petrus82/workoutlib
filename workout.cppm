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
export std::string
intervalTypeToString (WorkoutType type)
{
  switch (type)
    {
    case WorkoutType::AbsoluteWatt:
      return "AbsoluteWatt";
    case WorkoutType::PercentFTP:
      return "PercentFTP";
    case WorkoutType::AbsoluteHeartRate:
      return "AbsoluteHeartRate";
    case WorkoutType::PercentMaxHeartRate:
      return "PercentMaxHeartRate";
    default:
      return "Unknown";
    }
}
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
  std::string_view intensityUnitTag; // Intensity Unit specification
  std::string_view headerEnd;        // Header closing sequence
  std::string_view intervalTag;      // Interval preceding sequence
  std::string_view subIntervalTag;
  std::string_view repeatTag;
  std::string_view intervalIntensityLoTag; // Intensity specification
  std::string_view intervalIntensityHiTag;
  std::string_view intervalDurationTag; // Duration specification
  IntensityType type;
};

EXPORT_TEST constexpr std::string
trim (std::string_view string)
{
  const auto first = string.find_first_not_of (' ');
  const auto last = string.find_last_not_of (' ');
  if (first != std::string_view::npos && last != std::string_view::npos)
    {
      return std::string (string.substr (first, last - first + 1));
    }
  return {};
}

EXPORT_TEST constexpr std::optional<std::string>
getTag (std::ranges::view auto view, std::string_view tag)
{
  if (tag.empty ())
    {
      return std::nullopt;
    }
  auto it = std::ranges::find_if (
      view, [&] (auto const &string)
        { return std::string_view{ string }.starts_with (tag); });

  if (it == view.end ())
    {
      return std::nullopt;
    }

  std::string_view sv = *it;
  auto pos = sv.find ('=');
  if (pos == std::string_view::npos)
    {
      return std::nullopt;
    }
  return trim (sv.substr (pos + 1));
};

EXPORT_TEST std::expected<std::list<Interval>, std::string>
readIntervals (std::istream &file, const TextFileFormat &fileformat,
               uint16_t ftp);
EXPORT_TEST std::expected<Workout, std::string>
readWorkout (std::istream &file, const TextFileFormat &fileformat);

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

EXPORT_TEST const constexpr TextFileFormat ergFile{
  .headerStart{ "[COURSE HEADER]\n"
                "VERSION = 2\n"
                "UNITS = METRIC\n" },
  .nameTag{ "FILE NAME = " },
  .noteTag{ "DESCRIPTION = " },
  .intensityUnitTag{ "FTP = " },
  .headerEnd{ "MINUTES WATTS\n"
              "[END COURSE HEADER]\n"
              "[COURSE DATA]\n" },
  .intervalTag{ "[COURSE DATA]" },
  .type = IntensityType::PowerAbsLow
};
EXPORT_TEST const constexpr TextFileFormat mrcFile{
  .headerStart{ "[COURSE HEADER]\n"
                "VERSION = 2\n"
                "UNITS = METRIC\n" },
  .nameTag{ "FILE NAME = " },
  .noteTag{ "DESCRIPTION = " },
  .headerEnd{ "MINUTES PERCENT\n"
              "[END COURSE HEADER]\n"
              "[COURSE DATA]\n" },
  .intervalTag{ "[COURSE DATA]" },
  .type = IntensityType::PowerRelLow
};

EXPORT_TEST const constexpr TextFileFormat planFileAbsolute{
  .headerStart{ "=HEADER=\n\n" },
  .nameTag{ "NAME=" },
  .headerDuration{ "DURATION=" },
  .noteTag{ "DESCRIPTION=" },
  .headerEnd{ "PLAN_TYPE=0\n"
              "WORKOUT_TYPE=0\n"
              "=STREAM=\n\n" },
  .intervalTag{ "=INTERVAL=" },
  .intervalIntensityLoTag{ "PWR_LO=" },
  .intervalIntensityHiTag{ "PWR_HI=" },
  .intervalDurationTag{ "MESG_DURATION_SEC>=" },
  .type = IntensityType::PowerAbsHigh
};

EXPORT_TEST const constexpr TextFileFormat planFilePercentFtp{
  .headerStart{ "=HEADER=\n\n" },
  .nameTag{ "NAME=" },
  .headerDuration{ "DURATION=" },
  .noteTag{ "DESCRIPTION=" },
  .headerEnd{ "PLAN_TYPE=0\n"
              "WORKOUT_TYPE=0\n"
              "=STREAM=\n\n" },
  .intervalTag{ "=INTERVAL=" },
  .intervalIntensityLoTag{ "PERCENT_FTP_LO=" },
  .intervalIntensityHiTag{ "PERCENT_FTP_HI=" },
  .intervalDurationTag{ "MESG_DURATION_SEC>=" },
  .type = IntensityType::PowerRelHigh
};

class Interval
{
public:
  Interval () = default;

  explicit Interval (uintType intensity, std::chrono::seconds duration,
                     IntensityType type, uint8_t maxHeartRate)
      : m_type{ type }, m_duration{ duration }, m_maxHeartRate{ maxHeartRate }
  {
    calculateHeartRate (intensity, type, maxHeartRate);
  }

  explicit Interval (uintType intensity, std::chrono::seconds duration,
                     IntensityType type, uint16_t ftp)
      : m_type{ type }, m_duration{ duration }, m_ftp{ ftp }
  {
    calculatePower (intensity, type, ftp);
  }
  void
  setFtp (uint16_t ftp)
  {
    m_ftp = ftp;
  }
  uintType
  getFtp () const
  {
    return m_ftp;
  }
  void
  setIntensity (uintType intensity, IntensityType type)
  {
    if (type >= IntensityType::HeartRateAbsLow)
      {
        calculateHeartRate (intensity, type, m_maxHeartRate);
        return;
      }
    calculatePower (intensity, type, m_ftp);
  }

  uintType
  getIntensity (IntensityType type) const
  {
    auto typeValue{ std::to_underlying (type) };
    if (typeValue >= heartRateOffset)
      {
        return m_intensityHeartRate.at (typeValue - heartRateOffset);
      }
    return m_intensityPower.at (typeValue);
  }

  std::chrono::seconds
  getDuration () const
  {
    return m_duration;
  }
  template <class Rep, class Period>
  void
  setDuration (std::chrono::duration<Rep, Period> duration)
  {
    m_duration = std::chrono::duration_cast<std::chrono::seconds> (duration);
  }

private:
  void calculatePower (uint16_t power, IntensityType type, uint16_t ftp);
  void calculateHeartRate (uint8_t heartRate, IntensityType type,
                           uint8_t maxHeartRate);
  static uint16_t convertToRelative (uint16_t intensity, uint16_t value);
  static uint16_t convertToAbsolute (uint16_t intensity, uint16_t value);
  static uint8_t convertToPowerZone (uint16_t intensity, uint16_t ftp = 0);
  static uint8_t convertFromPowerZone (uint8_t intensity,
                                       bool getLower = true);
  static uint8_t convertToHeartRateZone (uint8_t intensity,
                                         uint8_t maxHeartRate = 0);
  static uint8_t convertFromHeartRateZone (uint8_t intensity,
                                           bool getLower = true);

private:
  std::chrono::seconds m_duration{};
  IntensityType m_type{};
  std::array<uint8_t, intensityTypes> m_intensityHeartRate{ 0 };
  std::array<uint16_t, intensityTypes> m_intensityPower{ 0 };
  uint8_t m_maxHeartRate{ 0 };
  uint16_t m_ftp{ 0 };
};
using Intervals = std::list<Interval>;
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
  [[nodiscard]] static std::expected<Workout, std::string>
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

    std::ifstream filestream;
    filetype == FileType::Fit ? filestream.open (file, std::ios::binary)
                              : filestream.open (file);

    if (!filestream)
      {
        return std::unexpected (
            std::format ("Cannot open file {}.", file.string ()));
      }

    /*     switch (filetype)
          {
          case FileType::Fit:
            return FitFile::readWorkout (filestream);
          case FileType::Plan:
            return readWorkout (filestream, PlanFile::HeaderName,
                                PlanFile::HeaderNotes,
       PlanFile::IntervalStart);

          case FileType::Erg:
            return readWorkout (filestream, ErgFile::HeaderName,
                                ErgFile::HeaderNotes, ErgFile::IntervalStart,
                                ErgFile::HeaderIntensity);
          case FileType::Mrc:
            return readWorkout (filestream, MrcFile::HeaderName,
                                MrcFile::HeaderNotes, MrcFile::IntervalStart);
          }
        std::unreachable (); */
  }
  std::expected<void, std::string>
  writeFile (std::filesystem::path &file, FileType fileType,
             WorkoutType workoutType, uint16_t relativeTo)
  {
    WriteFunction writeFunc;
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
  static constexpr std::expected<std::string, std::string>
  readFileContent (const std::filesystem::path &file)
  {
    std::ifstream filestream (file);
    if (filestream)
      {
        // Get file size and reserve memory
        filestream.seekg (0, std::ios::end);

        // std::ifstream::read does not take more than std::streamsize for the
        // file size
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
  static constexpr auto
  processContent (std::string_view fileContent, TextFileFormat format)
  {

    auto lines = fileContent | std::views::split ('\n')
                 | std::views::transform (
                     [] (auto line) { return std::string_view (line); });

    auto checkWorkout = [&] (auto line) { return line != format.intervalTag; };
    auto workout = std::views::take_while (lines, checkWorkout);
    auto intervalPos = fileContent.find (format.intervalTag);
    intervalPos += format.intervalTag.length ();
    std::string_view intervals{ fileContent.substr (
        intervalPos, fileContent.length () - intervalPos) };
    return std::pair{ workout, intervals };
  }

  static constexpr Workout
  getWorkout (std::ranges::view auto view, const TextFileFormat &format)
  {
    Workout workout;
    if (auto name = getTag (view, format.nameTag); name)
      {
        workout.setName (name.value ());
      }
    if (auto notes = getTag (view, format.noteTag); notes)
      {
        workout.setNotes (notes.value ());
      }
    if (auto ftp = getTag (view, format.intensityUnitTag); ftp)
      {
        workout.setFtp (std::stoi (ftp.value ()));
      }
    return workout;
  }
  static constexpr std::expected<std::vector<Interval>, std::string>
  getIntervals (std::string_view intervalView, const TextFileFormat &format,
                IntensityType type, uint16_t ftp = 0)
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
    auto intensities = intervals | std::views::drop (1)
                       | std::views::stride (2)
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
  static constexpr auto
  splitPlanContent (std::string_view fileData)
  {
    constexpr int intervalsTagLength = 8;
    std::string_view intervalsTag{ "=STREAM=" };
    auto workoutEnd = fileData.find (intervalsTag) + intervalsTagLength;
    auto workout = fileData.substr (0, workoutEnd);
    auto intervals
        = fileData.substr (workoutEnd, fileData.length () - (workoutEnd));
    return std::pair (workout, intervals);
  }
  static constexpr std::expected<std::vector<Interval>, std::string>
  getPlanIntervals (std::string_view intervalData, uintType ftp)
  {
    std::string_view intervalDelim{ "=INTERVAL=" };
    std::string_view subIntervalDelim{ "=SUBINTERVAL=" };
    std::string_view powerRelLow{ "PERCENT_FTP_LO=" };
    constexpr int characterRelPower{ 16 };
    std::string_view powerRelHigh{ "PERCENT_FTP_HI=" };
    std::string_view powerAbsLow{ "PWR_LO=" };
    std::string_view powerAbsHigh{ "PWR_HI=" };
    std::string_view timeTag{ "MESG_DURATION_SEC>=" };
    constexpr int characterAbsPower{ 7 };
    uintType intensityLow{};
    uintType intensityHigh{};
    IntensityType type{};

    auto lines
        = intervalData | std::views::split ('\n')
          | std::views::transform ([] (auto line)
                                     { return std::string_view (line); })
          | std::views::filter ([] (auto line) { return !line.empty (); });
    auto intervals
        = lines
          | std::views::chunk_by ([&] (auto a, auto b)
                                    { return !b.starts_with ("=INTERVAL="); });
    auto createInterval
        = [&] (auto data) -> std::expected<Interval, std::string>
      {
        Interval interval;
        interval.setFtp (ftp);
        if (auto retVal{ getTag (data, powerRelLow) }; retVal)
          {
            uintType pwrLow;
            std::string_view pwrString{ retVal.value () };
            auto [ptr, error]{ std::from_chars (
                pwrString.data (), pwrString.data () + pwrString.size (),
                pwrLow) };
            if (error != std::errc{})
              {
                return std::unexpected (std::format (
                    "Cannot convert to powerLow int value from {}",
                    pwrString));
              }
            interval.setIntensity (pwrLow, IntensityType::PowerRelLow);
          }
        if (auto retVal{ getTag (data, powerRelHigh) }; retVal)
          {
            std::string_view pwrString{ retVal.value () };
            uintType pwrHigh;
            auto [ptr, error]{ std::from_chars (
                pwrString.data (), pwrString.data () + pwrString.size (),
                pwrHigh) };
            if (error != std::errc{})
              {
                return std::unexpected (std::format (
                    "Cannot convert to powerRelHigh from {}", pwrString));
              }
            interval.setIntensity (pwrHigh, IntensityType::PowerRelHigh);
          }
        if (auto retVal{ getTag (data, powerAbsLow) }; retVal)
          {
            std::string_view pwrStr{ retVal.value () };
            uintType pwrAbsLow{};
            auto [ptr, error]{ std::from_chars (
                pwrStr.data (), pwrStr.data () + pwrStr.size (), pwrAbsLow) };
            if (error != std::errc{})
              {
                return std::unexpected (std::format (
                    "Cannot convert to pwrAbsLow from {}", pwrStr));
              }
            interval.setIntensity (pwrAbsLow, IntensityType::PowerAbsLow);
          }
        if (auto retVal{ getTag (data, powerAbsHigh) }; retVal)
          {
            std::string_view pwrStr{ retVal.value () };
            uintType pwrAbsHigh{};
            auto [ptr, error]{ std::from_chars (
                pwrStr.data (), pwrStr.data () + pwrStr.size (), pwrAbsHigh) };
            if (error != std::errc{})
              {
                return std::unexpected (std::format (
                    "Cannot convert to pwrAbsHigh from {}", pwrStr));
              }
            interval.setIntensity ((pwrAbsHigh), IntensityType::PowerAbsHigh);
          }
        if (auto retVal{ getTag (data, timeTag) }; retVal)
          {
            std::string_view time{ retVal.value () };
            int result;
            if (auto [ptr, error] = std::from_chars (
                    time.data (), time.data () + time.size (), result);
                error == std::errc{})
              {
                std::chrono::seconds seconds{ std::chrono::seconds (result) };
                interval.setDuration (seconds);
              }
            else
              {
                return std::unexpected (
                    std::format ("Cannot convert time from string {}", time));
              }
          }
        return interval;
      };

    std::vector<Interval> intervalVector;
    for (auto interval : intervals)
      {
        if (auto retVal{ createInterval (interval) }; retVal)
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
  void
  createInterval (Interval &interval)
  {
    m_order.emplace_back (interval);
  }
  void
  setIntervals (std::list<Interval> intervals)
  {
    m_order.clear ();
    m_order = std::move (intervals);
  }
  void
  createRepeat (const IteratorType &from, const IteratorType &to,
                uint8_t times)
  {
    auto range = std::ranges::subrange (from, to);
    auto repeated = std::views::repeat (range, times) | std::views::join;
    m_order.insert_range (from, repeated);
  }
  void
  removeIntervals (const IteratorType &from, const IteratorType &to)
  {
    m_order.erase (from, to);
  }

  auto
  begin ()
  {
    return m_order.begin ();
  }
  static auto
  next (std::list<std::weak_ptr<Interval>>::iterator current)
  {
    return std::next (current);
  }

  static auto
  prev (std::list<std::weak_ptr<Interval>>::iterator current)
  {
    return std::prev (current);
  }

  auto
  end ()
  {
    return m_order.end ();
  }
  auto
  intervalCount () const
  {
    return m_order.size ();
  }
  std::string
  getName () const
  {
    return m_workoutName;
  }
  void
  setName (std::string_view name)
  {
    m_workoutName = name;
  }

  std::string
  getNotes () const
  {
    return m_notes;
  }
  void
  setNotes (std::string_view notes)
  {
    m_notes = notes;
  }

  uint16_t
  getFtp () const
  {
    return m_ftp;
  }
  void
  setFtp (uint16_t ftp)
  {
    m_ftp = ftp;
  }

  uint8_t
  getMaxHeartRate () const
  {
    return m_maxHeartRate;
  }
  void
  setMaxHeartRate (uint8_t heartRate)
  {
    m_maxHeartRate = heartRate;
  }

  uint8_t
  getMinHeartRate () const
  {
    return m_minHeartRate;
  }
  void
  setMinHeartRate (uint8_t heartRate)
  {
    m_minHeartRate = heartRate;
  }

private:
  std::string m_workoutName;
  std::string m_notes;
  uint16_t m_ftp{ 0 };
  uint8_t m_maxHeartRate{ 0 };
  uint8_t m_minHeartRate{ 0 };
  Intervals m_order;
};

EXPORT_TEST std::expected<std::ifstream, std::string>
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

EXPORT_TEST std::expected<bool, std::string>
isValidFit (const std::filesystem::path &file, fit::Decode &decoder)
{
  auto filestream{ getFileStream (file) };
  if (filestream)
    {
      return decoder.CheckIntegrity (filestream.value ());
    }
  return std::unexpected (filestream.error ());
}

} // namespace Workouts