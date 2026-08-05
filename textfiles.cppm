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
// Used for erg and mrc file content
export const constexpr TextFileFormat ergFile{
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
}

} // namespace textFiles

}; // namespace Workouts
