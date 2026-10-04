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
static constexpr int maxLineLength{ 80 }; // Maximum line length for text files

std::string insertLineBreaks (std::string input, std::string_view tagSeparator,
                              std::size_t lineLength = maxLineLength)
{
  std::string output;
  std::size_t pos{ 0 };

  // Substitute all previous linebreaks from the input string with a space
  std::replace (input.begin (), input.end (), '\n', ' ');

  // Insert a line break at the last space character before the line length
  // limit
  while (pos < input.size ())
    {
      std::size_t endPos{ pos + lineLength };
      endPos = std::min (endPos, input.size ());

      // Find the last space character before the end position
      std::size_t lastSpacePos{ input.rfind (' ', endPos) };

      if (lastSpacePos != std::string_view::npos && lastSpacePos > pos)
        {
          endPos = lastSpacePos;
        }

      // remove the leading space character from the next line
      std::string newLine = input.substr (pos, endPos - pos);
      if (newLine.starts_with (' '))
        {
          newLine.erase (0, 1);
        }

      output.append (std::format ("{}{}\n", tagSeparator, newLine));
      pos = endPos;
    }
  return output;
}

std::string writeDuration (std::span<Interval> intervals)
{
  std::string output;
  unsigned int totalDuration{};
  for (const auto &interval : intervals)
    {
      for (auto subInterval{ interval.begin () };
           subInterval != interval.end (); ++subInterval)
        {
          totalDuration += subInterval->getDuration ().count ();
        }
    }
  output.append (std::format ("DURATION={}\n", totalDuration));
  return output;
}

export class TextHandler
{
public:
  explicit TextHandler (const std::filesystem::path &file)
      : m_file (file), m_inputstream (m_file)
  {}
  virtual ~TextHandler () = default;
  TextHandler (const TextHandler &) = delete;
  TextHandler &operator= (const TextHandler &) = delete;
  TextHandler (TextHandler &&) = delete;
  TextHandler &&operator= (TextHandler &&) = delete;

  // ReadFileC
  const auto &getWorkoutName () const { return m_workoutName; }
  const auto &getWorkoutNotes () const { return m_workoutNotes; }
  Intervals getIntervals () { return std::move (m_intervals); }

  // WriteFileC
  void setWorkoutName (std::string_view name) { m_workoutName = name; }
  void setWorkoutNotes (std::string_view notes) { m_workoutNotes = notes; }
  voidReturn writeFile (const std::filesystem::path &file,
                        std::string_view workoutName, std::string_view notes,
                        std::span<Interval> intervals)
  {
    std::ofstream outputStream (file);
    if (!outputStream.is_open ())
      {
        return std::unexpected (std::format (
            "Cannot open file {} for writing.", file.filename ().string ()));
      }
    outputStream << fileFormat.headerStart << '\n';
    outputStream << fileFormat.workoutNameToken << fileFormat.headerSeparator
                 << workoutName << '\n';
    outputStream << insertLineBreaks (std::string (notes),
                                      std::string (fileFormat.workoutNoteToken)
                                          .append (fileFormat.headerSeparator))
                 << '\n';
    if (fileFormat.hasDuration)
      {
        outputStream << writeDuration (intervals) << '\n';
      }
    outputStream << fileFormat.headerEnd << '\n';
    outputStream << writeIntervals (intervals);
    return {};
  }

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
                             auto workoutSection{
                               getTokenSection (m_fileContent,
                                                fileFormat.headerStart,
                                                fileFormat.headerEnd),
                             };
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
                             auto intervals{
                               getTokenSection (m_fileContent,
                                                fileFormat.intervalTokenBegin,
                                                fileFormat.intervalTokenEnd),
                             };
                             if (!intervals)
                               {
                                 return std::unexpected (intervals.error ());
                               }

                             return { intervals };
                           })
                     .and_then (
                         [this] (
                             std::string_view intervalSection) -> voidReturn
                           {
                             if (intervalSection.empty ())
                               {
                                 return std::unexpected (
                                     "No interval sections found.");
                               }

                             auto tokens{
                               getTokens (intervalSection,
                                          fileFormat.intervalSeparator),
                             };
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

  struct TextFileFormat
  {
    std::string_view headerStart;
    std::string_view headerEnd;
    std::string_view workoutNameToken;
    std::string_view workoutNoteToken;
    bool hasDuration{ false };
    std::string_view intervalTokenBegin;
    std::string_view intervalTokenEnd;
    std::string_view intensityUnitTag;
    std::string_view headerSeparator;
    std::string_view intervalToken;
    std::string_view intervalSeparator;
    IntensityUnit type;
    // Having fileFormat as publicly visible avoids a bunch of getter/setter
    // functions
    // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  } fileFormat{};

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
                           constexpr int firstPrintableChar{ 33 };
                           const auto start{
                             std::find_if (string.begin (), string.end (),
                                           [] (unsigned char character)
                                             {
                                               return character
                                                      >= firstPrintableChar;
                                             }),
                           };
                           const auto end{
                             std::find_if (
                                 string.rbegin (), string.rend (),
                                 [] (unsigned char character)
                                   { return character >= firstPrintableChar; })
                                 .base (),
                           };
                           return std::string (start, end);
                         };

                       const std::string &key{ trim (line.substr (0, pos)) };
                       const std::string &value{
                         trim (line.substr (pos + tagSeparator.size ())),
                       };
                       return Token{ key, value };
                     }
                   return Token{ std::string (line), std::string () };
                 })
           // Convert to std::vector<Token>
           | std::ranges::to<Tokens> ();
  }

protected:
  virtual std::string writeIntervals (std::span<Interval> intervals) = 0;

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
    fileFormat.headerSeparator = " = ";
    fileFormat.headerEnd = "=STREAM=";
    fileFormat.intervalTokenBegin = "=INTERVAL=";
    fileFormat.intervalTokenEnd = "=INTERVAL=";
    fileFormat.workoutNameToken = "NAME";
    fileFormat.workoutNoteToken = "DESCRIPTION";
    fileFormat.hasDuration = true;
  }
  ~PlanHandler () override = default;
  PlanHandler (const PlanHandler &) = delete;
  PlanHandler &operator= (const PlanHandler &) = delete;
  PlanHandler (PlanHandler &&) = delete;
  PlanHandler &&operator= (PlanHandler &&) = delete;

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
        Interval interval;
        Intensity intensity;
        bool isParent{ true };
        std::ptrdiff_t subIntervalCount{};
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
                        std::stoi (value.substr (0, value.find ('?'))));
                    interval.setIntensity (Intensity (intensity));
                    interval.setDuration (duration);
                    if (isParent)
                      {
                        intervals.emplace_back (Interval (interval));
                        isParent = false;
                        interval = Interval{};
                        intensity = Intensity{};
                      }
                    else
                      {
                        intervals.back ().addSubInterval (Interval (interval));
                        const auto lastRepeat{
                          intervals.back ().getRepeats ().size () - 1,
                        };
                        intervals.back ().getRepeatAt (lastRepeat).end
                            = subIntervalCount++;
                        interval = Interval{};
                        intensity = Intensity{};
                      }
                  }
                else if (key == "REPEAT")
                  {
                    interval.addRepeat (Repeat{
                        .begin = -1,
                        .end = -1,
                        .times = static_cast<unsigned int> (std::stoi (value)),
                    });
                  }
                else if (value == "INTERVAL=")
                  {
                    isParent = true;
                  }
              }
          }
        catch (std::exception e)
          {
            return std::unexpected (
                std::format ("Error converting {} into numbers.", tokens));
          }
      }
    return intervals;
  }

private:
  std::string writeIntervals (std::span<Interval> intervals) override
  {
    std::string output;
    for (const auto &interval : intervals)
      {
        auto writeInterval
            = [&output] (const Interval &interval, bool isParentEntry = true)
          {
            isParentEntry ? output.append ("\n=INTERVAL=\n")
                          : output.append ("\n=SUBINTERVAL=\n");
            switch (interval.getIntensity ().getType ())
              {
              case IntensityUnit::PowerZone: [[fallthrough]];
              case IntensityUnit::Watts:
                output.append (std::format (
                    "PWR_LO={}\n",
                    *interval.getIntensity ().getWatts (Level::Low)));
                output.append (std::format (
                    "PWR_HI={}\n",
                    *interval.getIntensity ().getWatts (Level::High)));
                break;
              case IntensityUnit::PercentFTP:
                output.append (std::format (
                    "PERCENT_FTP_LO={}\n",
                    *interval.getIntensity ().getPercentFTP (Level::Low)));
                output.append (std::format (
                    "PERCENT_FTP_HI={}\n",
                    *interval.getIntensity ().getPercentFTP (Level::High)));
                break;
              case IntensityUnit::PercentMaxHR: [[fallthrough]];
              case IntensityUnit::HeartRateZone: [[fallthrough]];
              case IntensityUnit::HeartRateBPM:
                output.append (std::format (
                    "HR_LO={}\n",
                    *interval.getIntensity ().getHeartRateBPM (Level::Low)));
                output.append (std::format (
                    "HR_HI={}\n",
                    *interval.getIntensity ().getHeartRateBPM (Level::High)));
                break;
              }
            output.append (std::format ("MESG_DURATION_SEC>={}?EXIT\n",
                                        interval.getDuration ().count ()));
          };
        if (!interval.getSubIntervals ().empty ())
          {
            output.append (
                std::format ("\n{}\n", fileFormat.intervalTokenBegin));
            output.append (
                std::format ("REPEAT={}\n", interval.getRepeatCount ()));
            writeInterval (interval, false);
            std::ranges::for_each (
                interval.getSubIntervals (),
                [&writeInterval] (const Interval &subInterval)
                  { writeInterval (subInterval, false); });
            continue;
          }
        writeInterval (interval);
      }
    return output;
  }
};
}; // namespace planFiles

export constexpr auto generateBlock (std::ranges::range auto &&intervals)
{
  // blockSize is the number of elements in a comparison of
  // sourceRange and targetRange.
  // If sourceRange == targetRange a repetition is found

  constexpr const std::size_t minimalLength{ 1 };

  // blockSizeSentinel is 1 number above the target blockSize because
  // Iota takes a sentinel value
  const auto blockSizeSentinel{ (intervals.size () / 2) + 1 };

  auto block = [] (std::size_t minimalLength, std::size_t blockSizeSentinel)
    { return std::views::iota (minimalLength, blockSizeSentinel); };
  if (blockSizeSentinel == 0)
    {
      return block (blockSizeSentinel, blockSizeSentinel);
    }
  return block (minimalLength, blockSizeSentinel);
}

std::vector<Interval> &
compress (std::vector<Interval> &intervals,
          const std::ranges::view auto &subIntervals,
          std::vector<Interval>::iterator parentInterval,
          std::ptrdiff_t repeatLength, const Repeat &repeat)
{
  std::ranges::for_each (subIntervals,
                         [&parentInterval] (auto &&subInterval)
                           {
                             parentInterval->addSubInterval (
                                 std::forward<Interval> (subInterval));
                           });
  parentInterval->addRepeat (repeat);

  const auto firstErase{ std::next (parentInterval) };

  // The sentinel of the erase range. Delete up to, but not including this:
  const auto endErase{ std::next (firstErase, repeatLength - 1) };
  intervals.erase (firstErase, endErase);
  return intervals;
};

export auto blockEncode (std::vector<Interval> &intervals)
{
  const auto blockRange{ generateBlock (intervals) };

  // Iterate over intervals with a blockSize
  std::ranges::for_each (
      blockRange,
      [&intervals] (const std::ptrdiff_t blockSize)
        {
          // A comparison window has a sequenceLength of 2*blockSize because it
          // contains a source and a target range
          const auto sequenceLength{ (2 * blockSize) };

          // The comparison has to be done in an index controlled loop
          // because if a repeating sequence has been found, the detection loop
          // will run over this blockLength again from the start to find
          // repeating sequences that have been hidden by the repeats before,
          // like in A | B | C | B | C | D | B | C | B | C | D
          // which becomes A | B | D | B | D after one iteration of blockLength
          // 2
          std::ptrdiff_t startIndex{};
          bool hasRepeats{ false };
          while (true)
            {
              // Slide a comparison window from right to left
              // over intervals to prevent iterator invalidation
              // if items will be removed
              const auto windows{
                intervals | std::views::reverse
                    | std::views::slide (sequenceLength),
              };

              bool isEndOfLoop{ startIndex >= std::ssize (windows) };

              if (isEndOfLoop && !hasRepeats)
                {
                  break;
                }
              if (isEndOfLoop && hasRepeats)
                {
                  hasRepeats = false;
                  startIndex = 0;
                }

              // split the comparison window in half and
              // compare
              //
              // This starts at end of interval and moves to beginning
              const auto window{ windows.begin () + startIndex };
              const auto source{ *window | std::views::take (blockSize) };
              const auto target{ *window | std::views::drop (blockSize) };
              if (std::ranges::equal (source, target))
                {
                  hasRepeats = true;
                  // get all possible following repeating
                  // sequences
                  const auto intervalsReverse{
                    intervals | std::views::reverse,
                  };
                  const auto comparisonPattern{
                    std::views::repeat (source) | std::views::join,
                  };
                  const auto repeatBegin{
                    std::ranges::next (intervalsReverse.begin (), startIndex),
                  };

                  const auto repeatEnd{
                    std::ranges::mismatch (
                        repeatBegin, intervalsReverse.end (),
                        comparisonPattern.begin (), comparisonPattern.end ())
                        .in1
                  };

                  const auto allRepeats{
                    std::ranges::subrange (repeatBegin, repeatEnd),
                  };

                  const auto repeatLength{
                    std::ranges::distance (repeatBegin, repeatEnd),
                  };

                  const auto parentInterval{
                    intervals.begin ()
                        + (std::ssize (intervals) - startIndex - repeatLength),
                  };

                  const auto lastRepeat{
                    intervals.begin ()
                        + (std::ssize (intervals) - startIndex - 1),
                  };

                  const auto times{
                    static_cast<unsigned int> (repeatLength / blockSize),
                  };

                  const auto subIntervals{
                    std::ranges::subrange (source.begin (),
                                           std::ranges::prev (source.end ())),
                  };

                  Repeat repeat{};
                  if (const auto previousRepeats{
                          std::ssize (parentInterval->getRepeats ()),
                      };
                      previousRepeats > 0)
                    {
                      repeat.begin = previousRepeats;
                      repeat.end = previousRepeats;
                    }
                  else if (std::ranges::size (subIntervals) > 0)
                    {
                      repeat.begin = -1;
                      repeat.end = previousRepeats;
                    }
                  else
                    {
                      repeat.begin = -1;
                      repeat.end = -1;
                    }
                  repeat.times = times;

                  intervals = compress (intervals, subIntervals,
                                        parentInterval, repeatLength, repeat);
                  continue;
                }
              ++startIndex;
            }
        });
  return intervals;
}

export class ErgMrcHandler : public TextHandler
{
public:
  explicit ErgMrcHandler (const std::filesystem::path &file)
      : TextHandler (file)
  {
    fileFormat.headerStart = "[COURSE HEADER]";
    fileFormat.headerSeparator = " = ";
    fileFormat.headerEnd = "[END COURSE HEADER]";
    fileFormat.workoutNameToken = "FILE NAME";
    fileFormat.workoutNoteToken = "DESCRIPTION";
    fileFormat.intervalTokenBegin = "[COURSE DATA]";
    fileFormat.intervalTokenEnd = "[END COURSE DATA]";
    fileFormat.intervalSeparator = "\t";
  }
  ~ErgMrcHandler () override = default;
  ErgMrcHandler (const ErgMrcHandler &) = delete;
  ErgMrcHandler &operator= (const ErgMrcHandler &) = delete;
  ErgMrcHandler (ErgMrcHandler &&) = delete;
  ErgMrcHandler &&operator= (ErgMrcHandler &&) = delete;

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

  std::expected<Intervals, std::string>
  getIntervalStrings (std::string_view intervalSectionString) override
  {
    // vector of std::pair with second being intensities. On odd indexes there
    // are start times, on even indexes endtimes.
    Intervals intervals;
    auto intervalTokens{
      getTokens (intervalSectionString, fileFormat.intervalSeparator),
    };
    bool isStart{ true };
    std::string intervalString;
    std::chrono::seconds duration;
    std::chrono::seconds startTime{};
    try
      {
        std::ranges::for_each (
            intervalTokens,
            [&] (const auto &intervalString)
              {
                if (isStart)
                  {
                    startTime
                        = std::chrono::duration_cast<std::chrono::seconds> (
                            std::chrono::duration<double,
                                                  std::ratio<secInMinute>>{
                                std::stof (intervalString.first),
                            });
                    isStart = false;
                  }
                else
                  {
                    auto endTime{
                      std::chrono::duration_cast<std::chrono::seconds> (
                          std::chrono::duration<double,
                                                std::ratio<secInMinute>>{
                              std::stod (intervalString.first),
                          }),
                    };
                    duration = endTime - startTime;
                    auto intensity{ std::stoi (intervalString.second) };
                    intervals.emplace_back (
                        *getInterval (intensity, duration));
                    isStart = true;
                  }
              });
      }
    catch (std::exception e)
      {
        // Conversion failed, continue
      }
    auto retVal{ blockEncode (intervals) };
    return intervals;
  }

  virtual intervalReturn getInterval (uint16_t intensity,
                                      std::chrono::seconds duration) = 0;

private:
  std::string writeIntervals (std::span<Interval> intervals) override
  {
    double startTime{ 0.0F };
    std::stringstream output;

    output << fileFormat.intervalTokenBegin << "\n";
    std::ranges::for_each (
        intervals,
        [&output, &startTime] (const Interval &interval)
          {
            // Expand interval because Erg/Mrc files don't
            // support subintervals
            std::ranges::for_each (
                interval,
                [&output, &startTime] (const Interval &subInterval)
                  {
                    const double endTime{
                      startTime
                          + std::chrono::duration<double,
                                                  std::ratio<secInMinute>> (
                                subInterval.getDuration ())
                                .count (),
                    };
                    const auto intensity{
                      subInterval.getIntensity ().getTarget (Level::Low),
                    };
                    output << std::fixed << std::setprecision (2) << startTime
                           << "\t" << intensity << "\n";
                    output << std::fixed << std::setprecision (2) << endTime
                           << "\t"
                           << subInterval.getIntensity ().getTarget (
                                  Level::High)
                           << "\n";
                    startTime = endTime;
                  });
          });
    output << fileFormat.intervalTokenEnd;
    return output.str ();
  }

private:
  uint16_t m_ftp{};
};

export namespace ergFiles
{
class ErgHandler : public ErgMrcHandler
{
public:
  explicit ErgHandler (const std::filesystem::path &file)
      : ErgMrcHandler (file)
  {}
  ~ErgHandler () override = default;
  ErgHandler (const ErgHandler &) = delete;
  ErgHandler &operator= (const ErgHandler &) = delete;
  ErgHandler (ErgHandler &&) = delete;
  ErgHandler &&operator= (ErgHandler &&) = delete;

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
  explicit MrcHandler (const std::filesystem::path &file)
      : ErgMrcHandler (file)
  {}
  ~MrcHandler () override = default;
  MrcHandler (const MrcHandler &) = delete;
  MrcHandler &operator= (const MrcHandler &) = delete;
  MrcHandler (MrcHandler &&) = delete;
  MrcHandler &&operator= (MrcHandler &&) = delete;

  intervalReturn getInterval (uint16_t intensity,
                              std::chrono::seconds duration) override
  {
    return Interval{ Intensity{ intensity, IntensityUnit::PercentFTP, 0 },
                     duration };
  }
};

}; // namespace mrcFiles
}; // namespace textFiles
}; // namespace Workouts
