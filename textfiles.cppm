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

using TokenSections = std::vector<std::string_view>;

// Key / Value pair
using Token = std::pair<std::string, std::string>;
using Tokens = std::vector<Token>;

export std::string_view getWorkoutSection (std::string_view fileData,
                                           std::string_view workoutTag,
                                           std::string_view intervalTag)
{
  // Remove the workoutTag (Header)
  fileData.remove_prefix (workoutTag.length ());

  // Return everything up but not including to the first intervalTag
  auto intervalPos{ fileData.find (intervalTag) };
  return fileData.substr (0, intervalPos);
}

export TokenSections getIntervalTokenSections (std::string_view fileData,
                                               std::string_view intervalTag)
{
  // split into intervals
  return fileData | std::views::split (intervalTag)
         | std::views::transform ([] (auto interval)
                                    { return std::string_view (interval); })
         | std::views::transform (
             [&intervalTag] (auto interval)
               {
                 // Remove intervalTag
                 auto pos{ interval.find (intervalTag) };
                 return interval.substr (pos + intervalTag.size ());
               })
         | std::ranges::to<TokenSections> ();
}

export Tokens getTokens (std::string_view tokenSection,
                         std::string_view tagSeparator)
{
  return tokenSection
         // Split into lines using newline character
         | std::views::split ('\n')
         // Convert const char* to std::string_view
         | std::views::transform ([] (auto line)
                                    { return std::string_view (line); })
         // Split into key / value pairs using tagSeparator
         | std::views::transform (
             [tagSeparator] (auto line)
               {
                 auto pos{ line.find (tagSeparator) };
                 if (pos != std::string_view::npos)
                   {
                     std::string key{ line.substr (0, pos) };
                     std::string value{ line.substr (pos
                                                     + tagSeparator.size ()) };
                     return Token{ key, value };
                   }
                 return Token{ std::string (line), std::string () };
               })
         // Convert to std::vector<Token>
         | std::ranges::to<Tokens> ();
}

static constexpr int MaxFileSize{ 1024 * 1024 }; // 1 MB in bytes

export class TextHandler
{
public:
  explicit TextHandler (const std::filesystem::path &file)
      : m_file (file), m_inputstream (m_file)
  {
  }
  // ReadFileC
  const auto &getWorkoutName () const { return m_workoutName; }
  const auto &getWorkoutNotes () const { return m_workoutNotes; }
  Intervals getIntervals () {}

  // WriteFileC
  void setWorkoutName (std::string_view name) {}
  void setWorkoutNotes (std::string_view notes) {}
  void writeFile (std::filesystem::path file, std::string_view workoutName,
                  std::string_view notes, std::span<Interval> intervals)
  {
  }

  // TestAdapterC
  voidReturn checkFile ()
  {
    if (std::filesystem::file_size (m_file) > MaxFileSize)
      {
        return std::unexpected (
            std::format ("The filesize of {} is above the filesize limit of 1 "
                         "MB ({} bytes).",
                         m_file.filename ().string (), MaxFileSize));
      }
    if (!m_inputstream.is_open ())
      {
        return std::unexpected (std::format ("Cannot open file {} to read.",
                                             m_file.filename ().string ()));
      }
    return {};
  }

  voidReturn readFile ()
  {
    m_fileContent = { std::istreambuf_iterator<char> (m_inputstream),
                      std::istreambuf_iterator<char> () };
    if (m_fileContent.empty ())
      {
        return std::unexpected (std::format ("Cannot read file {}.",
                                             m_file.filename ().string ()));
      }
    auto workoutSection
        = getWorkoutSection (m_fileContent, workoutToken, intervalToken);
    processWorkoutSection (workoutSection);

    m_intervalSections
        = getIntervalTokenSections (m_fileContent, intervalToken);
    return {};
  }

  void addInterval (Interval &&interval) {}
  std::string_view getErrMsg () const {}

private:
  void processWorkoutSection (std::string_view workoutSection)
  {
    auto tokens{ getTokens (workoutSection, "=") };
    for (const auto &[key, value] : tokens)
      {
        if (key == workoutNameToken)
          {
            m_workoutName = value;
          }
        else if (key == workoutNotesToken)
          {
            m_workoutNotes.append (value);
          }
      }
  }

private:
  // Format definitions
  static constexpr std::string_view workoutToken{ "=HEADER=" };
  static constexpr std::string_view workoutNameToken{ "Name" };
  static constexpr std::string_view workoutNotesToken{ "DESCRIPTION" };
  static constexpr std::string_view intervalToken{ "=INTERVAL=" };

private:
  std::filesystem::path m_file;
  std::ifstream m_inputstream;
  std::string m_fileContent;
  std::string_view m_workoutSection;
  std::string m_workoutName;
  std::string m_workoutNotes;
  TokenSections m_intervalSections;
};

export namespace planFiles
{
class PlanHandler : public TextHandler
{
public:
  explicit PlanHandler (const std::filesystem::path &file) : TextHandler (file)
  {
  }
};
}; // namespace planFiles

export namespace ergFiles
{
class ErgHandler : public TextHandler
{
public:
  explicit ErgHandler (const std::filesystem::path &file) : TextHandler (file)
  {
  }

public:
};
}; // namespace ergFiles

export namespace mrcFiles
{
class MrcHandler : public TextHandler
{
public:
  explicit MrcHandler (const std::filesystem::path &file) : TextHandler (file)
  {
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
