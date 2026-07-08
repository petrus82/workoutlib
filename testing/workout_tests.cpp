
#include <gtest/gtest.h>

import std;
import workoutlib;
import fitmodule;

void convertCSV (std::string_view file)
{
  const std::string FitCSVTool{ "/usr/lib/garminfit/FitCSVTool.jar" };
  std::string fitCSVCommand{
    std::string ("java -jar ").append (FitCSVTool).append (" ")
  };
  std::system (fitCSVCommand.append (file).c_str ());
}

using namespace std::string_literals;
namespace Workouts
{

TEST (RepeatTests, RepeatIntervalTest)
{
  constexpr uint16_t ftp{ 300 };
  constexpr uint16_t power1{ 400 };
  constexpr std::chrono::seconds duration{ 300 };
  constexpr uint16_t power2{ 200 };
  constexpr std::array expected{ power1, power2, power1, power2 };
  /*   Interval interval{ *Interval::create (
        AbsolutePower{ power2, ftp }, IntensityType::PowerAbsLow, duration) };
    interval.addSubInterval (*Interval::create (
        AbsolutePower{ power2, ftp }, IntensityType::PowerAbsLow, duration));
    interval.setRepeats (2);
    auto intensities{
      interval.getIntervalsExpanded ()
      | std::views::transform (
          [] (const Interval &interval)
            { return interval.getIntensity (IntensityType::PowerAbsLow); })
    };
    EXPECT_TRUE (std::ranges::equal (intensities, expected)); */
}

TEST (RemoveSubIntervalTest, RemoveSubInterval)
{
  constexpr uint16_t ftp{ 300 };
  constexpr uint16_t power1{ 400 };
  constexpr std::chrono::seconds duration{ 300 };
  constexpr uint16_t power2{ 200 };
  std::vector expected{ power1, power2, power1 };
  /*   Interval interval{ *Interval::create (
        AbsolutePower{ power1, ftp }, IntensityType::PowerAbsLow, duration) };
    interval.addSubInterval (*Interval::create (
        AbsolutePower{ power2, ftp }, IntensityType::PowerAbsLow, duration));
    interval.addSubInterval (*Interval::create (
        AbsolutePower{ power1, ftp }, IntensityType::PowerAbsLow, duration));
    auto before{
      interval.getIntervalsExpanded ()
      | std::views::transform (
          [] (const Interval &interval)
            { return interval.getIntensity (IntensityType::PowerAbsLow); })
    };
    EXPECT_TRUE (std::ranges::equal (before, expected));
    interval.removeSubInterval (0);
    expected.erase (expected.begin () + 1);
    auto after{
      interval.getIntervalsExpanded ()
      | std::views::transform (
          [] (const Interval &interval)
            { return interval.getIntensity (IntensityType::PowerAbsLow); })
    };
    EXPECT_TRUE (std::ranges::equal (after, expected));
    EXPECT_THROW (interval.removeSubInterval (1), std::out_of_range); */
}
}; // namespace Workouts
int main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}