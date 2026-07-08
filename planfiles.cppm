export module planfiles;

import config;
import common;
import interval;
import filehandling;
import std;

namespace Workouts
{
export using Tag = std::pair<std::string, std::string>;
export using Tags = std::vector<Tag>;

export namespace planFiles
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
  std::size_t previousPos = 0;
  std::size_t intervalPos = intervals.find (planFile.intervalTag);

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

export constexpr Tags getTags (std::string_view data,
                               std::string_view tagSeparator)
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
  interval.getIntensity ().setFTP (ftp);

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
          interval.getIntensity ().setTarget (*retVal, type, level);
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

}; // namespace Workouts