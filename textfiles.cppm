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

std::vector<Interval> &removeDuplicates (std::vector<Interval> &intervals,
                                         const Block &bestBlock)
{
  // After looping over the whole intervals sequence, the intervals which
  // follow the first interval in the repeat sequence have to be moved into
  // the subInterval vector of the parent interval of the repeat.
  // The parent interval is the first interval of the repeating sequence.

  if (bestBlock.score > 0)
    {
      // Start at the end of intervals to not invalidate iterators which have
      // to be used in the following loop execution.
      //
      // TODO: Switch to iterators and RBF and use the return value of
      // std::vector::erase to omit Iterator invalidation
      for (std::size_t index{ bestBlock.startIndex + bestBlock.blockLength
                              - 1 };
           index >= bestBlock.startIndex + 1; --index)
        {
          intervals.at (bestBlock.startIndex)
              .addSubInterval (std::move (intervals.at (index)));

          // Erase the iterator to the interval we just moved into the
          // parent interval
          intervals.erase (intervals.begin () + index);
          intervals.at (bestBlock.startIndex)
              .setRepeats (bestBlock.repeatCount);
        }

      // All intervals which follow after parent + blockLength - 1 until the
      // end of the sequence are now semantic duplicates. They can be deleted
      // because they will be replicated by executing the repeating sequence
      // again the number of times specified by repeatCount.
      for (std::size_t index{ intervals.size () - 1 };
           index >= bestBlock.blockLength - 1; --index)
        {
          intervals.erase (intervals.begin () + index);
        }
    }
  return intervals;
}

export std::vector<Interval> &blockEncode (std::vector<Interval> &intervals)
{
  /*
Intervals can be written as a repetitive, sometimes nested sequence.
An example to this would be a workout with intervals of
150 - 400-200-400-200 - 150 - 400-200-400-200 - 50 watts,
which can be rewritten as
150 - 2x(2x(400 - 200) - 150) - 50

Thus this function has to
calculate 4 variables:
- startIndex is the beginning index of a repeating sequence
- endIndex is the index of the end of the repeating sequence
- blockLength comprises 2^n items with n being at least 1 and at most 2^n =
intervals.size()
- repeatCount is at least 2

- The algorithm should maximize the number of blockLength * repeatCount
- Repeating sequences can be nested
*/

  if (intervals.size () <= 1)
    {
      return intervals;
    }

  Block bestBlock;
  using IntervalIt = std::vector<Interval>::iterator;

  // Start with blockLength close to half of the whole sequence
  // if it is not 2^n, skip to next number equal to 2^n
  for (auto blockLength{ intervals.size () / 2 }; blockLength >= 1;
       blockLength >>= 1)
    {

      // Skip non-powers of 2
      if ((blockLength & (blockLength - 1)) != 0)
        {
          continue;
        }

      // From where to start the detection of repeating sequences
      // Initially this will be the start of intervals and then this will go up
      // to the point where at least one repeating sequence (2*blockLength)
      // will be possible
      for (size_t compareFromIndex{ 0 };
           compareFromIndex + (blockLength * 2) <= intervals.size ();
           ++compareFromIndex)
        {
          IntervalIt compareFromStart{ intervals.begin () + compareFromIndex };
          IntervalIt compareFromEnd{ compareFromStart + blockLength };

          uint16_t repeatCount{ 1 };

          // Slide a comparison window of blockLength from compareFromIndex to
          // end of sequence and detect longest repeating sequence
          for (size_t compareToIndex = compareFromIndex + blockLength;
               compareToIndex + blockLength <= intervals.size ();
               compareToIndex += blockLength)
            {
              auto compareToRange{ intervals.begin () + compareToIndex };

              // If the range we compare from is equal to the range we compare
              // to then a repeating sequence has been found and repeatCount is
              // at least 2.
              // Now continue the search, if another repeating sequence
              // is found, then increment again here.
              // If the compareTo range is different from the compareTo range,
              // then exit this detection loop
              if (std::equal (compareFromStart, compareFromEnd,
                              compareToRange))
                {
                  ++repeatCount;
                }
              else
                {
                  break;
                }
            }

          // If a repeating sequence has been found previously (i.e.
          // repeatCount is at least 2) then save the found sequence
          //
          // TODO: use a vector and check if the found sequence overlaps, maybe
          // implement recursion to look for nested sequences
          if (repeatCount >= 2 && blockLength * repeatCount > bestBlock.score)
            {
              bestBlock = Block{
                .startIndex = static_cast<std::size_t> (
                    std::distance (intervals.begin (), compareFromStart)),
                .endIndex = static_cast<std::size_t> (
                    std::distance (intervals.begin (), compareFromEnd)),
                .blockLength = blockLength,
                .repeatCount = repeatCount,
                .score = blockLength * repeatCount
              };
            }
        }
    }
  return removeDuplicates (intervals, bestBlock);
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
