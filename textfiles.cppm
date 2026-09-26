export module textfiles;

import common;
import config;
import filehandling;
import interval;
import std;
import std.compat;

namespace Workouts
{
namespace textFiles
{
/*
All textfiles need
- a checkFile() function
- a readFile() function

  Textfiles can have two different set of tokens:
- The first are multiline tokens (=tokenSections) which are limited by the
start of the next token.
- The second type are single line tokens which are limited by the newline
character.
*/

using TokenSection = std::vector<std::string_view>;

// Key / Value pair
using Token = std::pair<std::string, std::string>;
using Tokens = std::vector<Token>;

export std::expected<std::string_view, std::string>
getTokenSection (std::string_view fileData, std::string_view beginToken,
                 std::string_view endToken = "")
{
  std::size_t beginIt{ fileData.find (beginToken) };
  std::size_t endIt{};
  TokenSection tokenSections{};
  if (beginIt != std::string_view::npos)
    {
      if (!endToken.empty ())
        {
          endIt = fileData.find (endToken, beginIt);
        }
      else
        {
          endIt = fileData.find (beginToken, beginIt);
        }
      if (beginIt == std::string_view::npos || endIt == std::string_view::npos
          || endIt == beginIt || endIt <= beginIt)
        {
          return std::unexpected ("No valid token section found.");
        }
      return fileData.substr (beginIt, endIt - beginIt);
    }
  return std::unexpected ("No token section found.");
}

static constexpr int MaxFileSize{ 1024 * 1024 }; // 1 MB in bytes

export class TextHandler
{
public:
  explicit TextHandler (const std::filesystem::path &file)
      : m_file (file), m_inputstream (m_file)
  {}
  // ReadFileC
  const auto &getWorkoutName () const { return m_workoutName; }
  const auto &getWorkoutNotes () const { return m_workoutNotes; }
  Intervals getIntervals () { return std::move (m_intervals); }

  // WriteFileC
  void setWorkoutName (std::string_view name) {}
  void setWorkoutNotes (std::string_view notes) {}
  void writeFile (std::filesystem::path file, std::string_view workoutName,
                  std::string_view notes, std::span<Interval> intervals)
  {}

  // TestAdapterC
  voidReturn checkFile ()
  {
    if (!std::filesystem::exists (m_file))
      {
        return std::unexpected (
            std::format ("File {} does not exist.", m_file.string ()));
      }

    if (!m_inputstream.is_open ())
      {
        return std::unexpected (std::format ("Cannot open file {}.",
                                             m_file.filename ().string ()));
      }
    return {};
  }

  voidReturn getFileHeader (std::string_view workoutSection)
  {
    auto tokens{ getTokens (workoutSection, "=") };
    for (const auto &[key, value] : tokens)
      {
        if (key == fileFormat.workoutNameToken)
          {
            m_workoutName = value;
          }
        else if (key == fileFormat.workoutNoteToken)
          {
            if (m_workoutNotes.empty ())
              {
                m_workoutNotes = value;
              }
            else
              {
                m_workoutNotes.append ("\n").append (value);
              }
          }
      }
    return {};
  }

  virtual std::expected<Intervals, std::string>
  getIntervalStrings (std::string_view intervalSection) = 0;

  std::expected<Intervals, std::string>
  getIntervals (std::string_view fileContent)
  {
    return
        // get std::vector<std::string_view> of interval strings
        getTokenSection (fileContent, fileFormat.intervalTokenBegin,
                         fileFormat.intervalTokenEnd)
            .and_then (
                // split the interval section into interval strings
                [this] (std::string_view intervalSection)
                    -> std::expected<Intervals, std::string>
                  { return getIntervalStrings (intervalSection); });
  }

  voidReturn readFile ()
  {
    return
        [this] ()
            -> voidReturn
    // Check file
                 { return checkFile (); }()
                     .and_then (
                         // Check if it is a valid textfile
                         [this] () -> voidReturn
                           {
                             if (std::filesystem::file_size (m_file)
                                 > MaxFileSize)
                               {
                                 return std::unexpected (std::format (
                                     "The filesize of {} is above the "
                                     "filesize limit of 1 "
                                     "MB ({} bytes).",
                                     m_file.filename ().string (),
                                     MaxFileSize));
                               }
                             return {};
                           })
                     .and_then (
                         // Read file content into string
                         [this] () -> voidReturn
                           {
                             m_fileContent = {
                               std::istreambuf_iterator<char> (m_inputstream),
                               std::istreambuf_iterator<char> ()
                             };
                             if (m_fileContent.empty ())
                               {
                                 return std::unexpected (std::format (
                                     "Cannot read file {}.",
                                     m_file.filename ().string ()));
                               }
                             return {};
                           })
                     .and_then (
                         // get workout section
                         [this] ()
                             -> std::expected<std::string_view, std::string>
                           {
                             auto workoutSection{ getTokenSection (
                                 m_fileContent, fileFormat.headerStart,
                                 fileFormat.headerEnd) };
                             if (!workoutSection)
                               {
                                 return std::unexpected (
                                     workoutSection.error ());
                               }
                             return workoutSection;
                           })
                     .and_then (
                         // Extract workout name and notes from workout section
                         [this] (std::string_view workoutSection)
                           { return getFileHeader (workoutSection); })
                     .and_then (
                         // get interval sections
                         [this] ()
                             -> std::expected<std::string_view, std::string>
                           {
                             auto intervals{ getTokenSection (
                                 m_fileContent, fileFormat.intervalTokenBegin,
                                 fileFormat.intervalTokenEnd) };
                             if (!intervals)
                               {
                                 return std::unexpected (intervals.error ());
                               }

                             return { intervals };
                           })
                     .and_then (
                         [this] (
                             std::string_view &&intervalSection) -> voidReturn
                           {
                             if (intervalSection.empty ())
                               {
                                 return std::unexpected (
                                     "No interval sections found.");
                               }

                             auto tokens{ getTokens (
                                 intervalSection,
                                 fileFormat.intervalSeparator) };
                             if (tokens.empty ())
                               {
                                 return std::unexpected (
                                     "No tokens found in interval "
                                     "section.");

                                 // getInterval (tokens);
                               }
                             return {};
                           });
  }
  void addInterval (Interval &&interval)
  { m_intervals.emplace_back (std::move (interval)); }
  std::string_view getErrMsg () const {}

protected:
  struct TextFileFormat
  {
    std::string_view headerStart;
    std::string_view headerEnd;
    std::string_view workoutNameToken;
    std::string_view workoutNoteToken;
    std::string_view intervalTokenBegin;
    std::string_view intervalTokenEnd;
    std::string_view intensityUnitTag;
    std::string_view headerSeparator;
    std::string_view intervalToken;
    std::string_view intervalSeparator;
    IntensityUnit type;
  } fileFormat;

  Tokens getTokens (std::string_view tokenSection,
                    std::string_view tagSeparator)
  {
    return tokenSection
           // Split into lines using newline character
           | std::views::split ('\n')
           // Convert const char* to std::string_view and remove
           // intervalTokenBegin and End
           | std::views::transform ([] (auto line)
                                      { return std::string_view (line); })
           | std::views::drop_while (
               [this] (auto line)
               // Drop intervalTokenBegin and intervalTokenEnd and empty lines
                 {
                   return line == fileFormat.intervalTokenBegin
                          || line == fileFormat.intervalTokenEnd
                          || line.empty ();
                 })
           // Split into key / value pairs using tagSeparator
           | std::views::transform (
               [tagSeparator] (auto line)
                 {
                   auto pos{ line.find (tagSeparator) };
                   if (pos != std::string_view::npos)
                     {

                       // Remove trailing / leading spaces
                       auto trim = [] (std::string_view string)
                         {
                           const auto start{ std::find_if (
                               string.begin (), string.end (),
                               [] (unsigned char character)
                                 { return character >= 33; }) };
                           const auto end{
                             std::find_if (string.rbegin (), string.rend (),
                                           [] (unsigned char character)
                                             { return character >= 33; })
                                 .base ()
                           };
                           return std::string (start, end);
                         };

                       const std::string &key{ trim (line.substr (0, pos)) };
                       const std::string &value{ trim (
                           line.substr (pos + tagSeparator.size ())) };
                       return Token{ key, value };
                     }
                   return Token{ std::string (line), std::string () };
                 })
           // Convert to std::vector<Token>
           | std::ranges::to<Tokens> ();
  }

private:
  std::filesystem::path m_file;
  std::ifstream m_inputstream;
  std::string m_fileContent;
  std::string_view m_workoutSection;
  std::string m_workoutName;
  std::string m_workoutNotes;
  TokenSection m_intervalSections;
  Intervals m_intervals;
};

export namespace planFiles
{
class PlanHandler : public TextHandler
{
public:
  explicit PlanHandler (const std::filesystem::path &file) : TextHandler (file)
  {
    fileFormat.headerStart = "=HEADER=";
    fileFormat.headerEnd = "=STREAM=";
    fileFormat.intervalTokenBegin = "=INTERVAL=";
    fileFormat.intervalTokenEnd = "=INTERVAL=";
    fileFormat.workoutNameToken = "NAME";
    fileFormat.workoutNoteToken = "DESCRIPTION";
  }

  std::expected<Intervals, std::string>
  getIntervalStrings (std::string_view intervalSectionString) override
  {
    Intervals intervals;
    for (auto intervalString :
         std::ranges::split_view (intervalSectionString,
                                  fileFormat.intervalTokenBegin)
             | std::views::transform ([] (auto line)
                                        { return std::string_view (line); })
             | std::views::drop_while ([] (auto line)
                                         { return line.empty (); }))
      {
        Intensity intensity;
        std::chrono::seconds duration;
        Tokens tokens{ getTokens (intervalString, "=") };
        try
          {
            for (const auto &[key, value] : tokens)
              {
                if (key == "PWR_LO")
                  {
                    intensity.setTarget (std::stoi (value),
                                         IntensityUnit::Watts, Level::Low);
                  }
                else if (key == "PWR_HI")
                  {
                    intensity.setTarget (std::stoi (value),
                                         IntensityUnit::Watts, Level::High);
                  }
                else if (key == "PERCENT_FTP_LO")
                  {
                    intensity.setTarget (std::stoi (value),
                                         IntensityUnit::PercentFTP,
                                         Level::Low);
                  }
                else if (key == "PERCENT_FTP_HI")
                  {
                    intensity.setTarget (std::stoi (value),
                                         IntensityUnit::PercentFTP,
                                         Level::High);
                  }
                else if (key == "HR_LO")
                  {
                    intensity.setTarget (std::stoi (value),
                                         IntensityUnit::HeartRateBPM,
                                         Level::Low);
                  }
                else if (key == "HR_HI")
                  {
                    intensity.setTarget (std::stoi (value),
                                         IntensityUnit::HeartRateBPM,
                                         Level::High);
                  }
                else if (key == "MESG_DURATION_SEC>")
                  {
                    duration = std::chrono::seconds (
                        std::stoi (value.substr (0, value.find ("?"))));
                  }
              }
          }
        catch (std::exception e)
          {
            /* return std::unexpected(
                std::format("Error converting {} into numbers.", tokens)); */
          }
        intervals.emplace_back (Interval{ std::move (intensity), duration });
      }
    return intervals;
  }
};
}; // namespace planFiles

struct Block
{
  std::size_t startIndex{};
  std::size_t endIndex{};
  std::size_t blockLength{};
  uint16_t repeatCount{};
  std::size_t score{};
};

export constexpr auto generateBlock (std::ranges::range auto &&intervals)
{
  // Generate a range of blockSizes for intervals
  // starting with size() / 2, down to 1
  // blockSize is the number of elements in a comparison of
  // sourceRange and targetRange.
  // If sourceRange == targetRange a repetition is found

  constexpr const std::size_t minimalLength{ 1 };

  // blockSizeSentinel is 1 number above the target blockSize because
  // Iota takes a sentinel value
  const auto blockSizeSentinel{ (intervals.size () / 2) + 1 };

  auto block = [] (std::size_t minimalLength, std::size_t blockSizeSentinel)
    {
      return std::views::iota (minimalLength, blockSizeSentinel)
             | std::ranges::views::reverse;
    };
  if (blockSizeSentinel == 0)
    {
      return block (blockSizeSentinel, blockSizeSentinel);
    }
  return block (minimalLength, blockSizeSentinel);
}

std::vector<Interval> &
compress (std::vector<Interval> &intervals,
          std::ranges::view auto subIntervals,
          std::ranges::iterator_t<decltype (subIntervals)> parentInterval,
          std::ranges::iterator_t<decltype (subIntervals)> lastRepeat,
          const uint16_t repeats)
{
  std::ranges::for_each (subIntervals,
                         [&parentInterval, &repeats] (auto subInterval)
                           {
                             parentInterval->addSubInterval (
                                 std::move (subInterval));
                             parentInterval->setRepeats (repeats);
                           });

  // Remove all redundant intervals
  // Iterators are std::reverse_iterator, and std::reverse_iterator::base
  // points one element in forward direction beyond the iterator
  intervals.erase (parentInterval.base (), lastRepeat.base ());
  return intervals;
};

export auto blockEncode (std::vector<Interval> &intervals)
{
  const auto blockRange{ generateBlock (intervals) };

  // Iterate over intervals with a blockSize in descending order
  std::ranges::for_each (
      blockRange,
      [&intervals] (const std::ptrdiff_t blockSize)
        {
          // A comparison window has a sequenceLength of 2*blockSize because it
          // contains a source and a target range
          const auto sequenceLength{ (2 * blockSize) };

          // Slide a comparison window from right to left
          // over intervals to prevent iterator invalidation
          // if items should be removed
          const auto windows{ intervals | std::views::reverse
                              | std::views::slide (sequenceLength) };

          // Index based loop to be able to switch to end of repeating sequence
          for (std::ptrdiff_t startIndex{}; startIndex <= std::ssize (windows);
               ++startIndex)
            {

              // split the comparison window in half and
              // compare
              //
              // This starts at end of interval and moves to beginning
              const auto window{ windows.begin () + startIndex };
              const auto source{ *window | std::views::take (blockSize) };
              const auto target{ *window | std::views::drop (blockSize) };
              if (std::ranges::equal (source, target))
                {
                  // get all possible following repeating
                  // sequences
                  const auto intervalsReverse{ intervals
                                               | std::views::reverse };
                  const auto allRepeats{ std::ranges::find_end (
                      intervalsReverse, source) };

                  const auto repeatLength{ std::ranges::distance (
                                               intervalsReverse.begin (),
                                               allRepeats.end ())
                                           - startIndex };
                  // Reverse view, so parent interval is at the end!
                  auto parentInterval{ std::ranges::prev (allRepeats.end ()) };
                  const auto lastRepeat{ source.begin () };
                  const auto times{ static_cast<unsigned int> (repeatLength
                                                               / blockSize) };
                  auto subIntervals{ std::ranges::subrange (
                      source.begin (), std::ranges::prev (source.end ())) };

                  intervals = compress (intervals, subIntervals,
                                        parentInterval, lastRepeat, times);
                  parentInterval->addRepeat (
                      Repeat{ .begin = -1,
                              .end = std::ssize (subIntervals) - 1,
                              .times = times });
                  startIndex += repeatLength;
                }
            }
        });
  return intervals;
}

export class ErgMrcHandler : public TextHandler
{
public:
  virtual ~ErgMrcHandler () = default;
  explicit ErgMrcHandler (const std::filesystem::path &file)
      : TextHandler (file)
  {
    fileFormat.headerStart = "[COURSE HEADER]";
    fileFormat.headerEnd = "[END COURSE HEADER]";
    fileFormat.workoutNameToken = "FILE NAME";
    fileFormat.workoutNoteToken = "DESCRIPTION";
    fileFormat.intervalTokenBegin = "[COURSE DATA]";
    fileFormat.intervalTokenEnd = "[END COURSE DATA]";
    fileFormat.intervalSeparator = "\t";
  }

  static std::expected<std::chrono::seconds, std::string>
  getDuration (const std::string &begin, const std::string &end)
  {
    try
      {
        int startTime{ std::stoi (begin) };
        int endTime{ std::stoi (end) };
        auto duration = std::chrono::duration_cast<std::chrono::seconds> (
            std::chrono::minutes (endTime - startTime));
        startTime = endTime;
        return duration;
      }
    catch (std::exception e)
      {
        return std::unexpected (std::format (
            "Cannot convert to duration with {} to {}.", begin, end));
      }
  }
  void setFTP (uint16_t ftp) { m_ftp = ftp; }

protected:
  std::expected<Intervals, std::string>
  getIntervalStrings (std::string_view intervalSectionString) override
  {
    // vector of std::pair with second being intensities. On odd indexes there
    // are start times, on even indexes endtimes.
    Intervals intervals;
    auto intervalTokens{ getTokens (intervalSectionString,
                                    fileFormat.intervalSeparator) };
    bool isStart{ true };
    std::string intervalString;
    std::chrono::seconds duration;
    std::chrono::seconds startTime{};
    for (const auto &intervalString : intervalTokens)
      {
        if (isStart)
          {
            try
              {
                startTime = std::chrono::duration_cast<std::chrono::seconds> (
                    std::chrono::duration<double, std::ratio<60>>{
                        std::stof (intervalString.first) });
                isStart = false;
              }
            catch (std::exception e)
              {}
          }
        else
          {
            try
              {
                auto endTime{
                  std::chrono::duration_cast<std::chrono::seconds> (
                      std::chrono::duration<double, std::ratio<60>>{
                          std::stod (intervalString.first) })
                };
                duration = endTime - startTime;
                auto intensity{ std::stoi (intervalString.second) };
                intervals.emplace_back (*getInterval (intensity, duration));
                isStart = true;
              }
            catch (std::exception e)
              {}
          }
      }
    auto retVal{ blockEncode (intervals) };
    return intervals;
  }

  virtual intervalReturn getInterval (uint16_t intensity,
                                      std::chrono::seconds duration) = 0;

private:
  uint16_t m_ftp{};
};

export namespace ergFiles
{
class ErgHandler : public ErgMrcHandler
{
public:
  ~ErgHandler () override = default;
  explicit ErgHandler (const std::filesystem::path &file)
      : ErgMrcHandler (file)
  {}

  intervalReturn getInterval (uint16_t intensity,
                              std::chrono::seconds duration) override
  {
    return Interval{ Intensity{ intensity, IntensityUnit::Watts, 0 },
                     duration };
  }
};
}; // namespace ergFiles

export namespace mrcFiles
{
class MrcHandler : public ErgMrcHandler
{
public:
  ~MrcHandler () override = default;
  explicit MrcHandler (const std::filesystem::path &file)
      : ErgMrcHandler (file)
  {}

  intervalReturn getInterval (uint16_t intensity,
                              std::chrono::seconds duration) override
  {
    return Interval{ Intensity{ intensity, IntensityUnit::PercentFTP, 0 },
                     duration };
  }
};

}; // namespace mrcFiles
}; // namespace textFiles

// Used for erg and mrc file content
/* export const constexpr TextFileFormat ergFile{
  .headerStart{ "[COURSE HEADER]\n"
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
  .type = IntensityUnit::Watts
};
export const constexpr TextFileFormat mrcFile{
  .headerStart{ "[COURSE HEADER]\n"
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
  .type = IntensityUnit::PercentFTP
};

export constexpr std::expected<std::vector<std::unique_ptr<Interval>>,
                               std::string>
getTextIntervals (std::string_view intervalView, const TextFileFormat &format,
                  IntensityUnit type, uint16_t ftp = 0);

export constexpr std::expected<std::vector<std::unique_ptr<Interval>>,
                               std::string>
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
          interval.getIntensity ().setTarget (intensityStart, type,
                                              Level::Low);
          interval.getIntensity ().setTarget (intensityEnd, type, Level::High);
          interval.getIntensity ().setFTP (ftp);
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
  auto times = intervals // | std::views::stride (2)
               | std::views::transform (convert2seconds);

  // Every second odd time entry is a start time
  auto startTime = times; // | std::ranges::views::stride (2);

  // Every second uneven time entry is an end time
  auto endTime
      = times
        | std::ranges::views::drop (1); // | std::ranges::views::stride (2);

  // Every second uneven entry is an intensity, convert it to int
  auto intensities = intervals
                     | std::ranges::views::drop (1)
                     //| std::ranges::views::stride (2)
                     | std::ranges::views::transform (
                         [] (auto intensity)
                           { return std::stoi (std::string (intensity)); });

  // Every second odd intensity is the intensity at the beginning of the
  // interval
  auto intensityStart = intensities; // | std::ranges::views::stride (2);

  // Every second unveven intensity is the intensity at the end of the
  // interval
  auto intensityEnd
      = intensities
        | std::ranges::views::drop (1); //| std::ranges::views::stride (2);

  // Generate a std::tuple of all interval data and create an interval
  auto intervalData = std::ranges::views::zip (startTime, endTime,
                                               intensityStart, intensityEnd)
                      | std::ranges::views::transform (createIntervalData);

  // return a vector with all intervals constructed
  return std::ranges::to<std::vector<std::unique_ptr<Interval>>> (
      intervalData);
}
export double writeIntensityDuration (std::iostream &file,
                                      const TextFileFormat &fileFormat,
                                      const Interval &interval,
                                      double startTime)
{
  double endTime{ startTime
                  + std::chrono::duration<double, std::ratio<60>> (
                        interval.getDuration ())
                        .count () };
  auto intensityLo{ interval.getIntensity ().getTarget (Level::Low) };
  auto intensityHi{ interval.getIntensity ().getTarget (Level::High) };

  file << std::fixed << std::setprecision (3) << startTime << "\t"
       << intensityLo << "\n";
  file << std::fixed << std::setprecision (3) << endTime << "\t" << intensityHi
       << "\n";
  return endTime;
}

export constexpr void writeToStream (std::iostream &file, std::string_view key,
                                     std::string_view value,
                                     std::string_view tagSeparator)
{ file << key << " " << tagSeparator << " " << value << '\n'; }

export void writeIntensityTime (std::iostream &file,
                                const TextFileFormat &fileFormat,
                                const Interval &interval)
{
  file << fileFormat.intervalTag << "\n\n";
  file << fileFormat.intervalIntensityAbsLoTag << fileFormat.intervalSeparator
       << interval.getIntensity ().getTarget (Level::Low) << '\n';
  file << fileFormat.intervalIntensityAbsHiTag << fileFormat.intervalSeparator
       << interval.getIntensity ().getTarget (Level::High) << '\n';
  file << fileFormat.intervalDurationTag << fileFormat.intervalSeparator
       << interval.getDuration ().count () << "?EXIT\n";
} */
}; // namespace Workouts
