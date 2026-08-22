#include <gtest/gtest.h>

import std;
import workoutlib;
import fitmodule;
import fitfiles;

namespace Workouts
{
namespace fitFiles
{
class FitReadTester : public ::testing::Test
{
protected:
  std::filesystem::path testfile{ "Workout.fit" };
  FitHandler m_handler{ testfile };

private:
};
TEST_F (FitReadTester, ReadFileTest)
{
  if (auto retVal (m_handler.checkFile ()); !retVal)
    {
      FAIL () << retVal.error ();
    }
  if (auto retVal{ m_handler.readFile () }; !retVal)
    {
      FAIL () << retVal.error ();
    }
  EXPECT_EQ (m_handler.getWorkoutName (), "4x4 VO2Max Cycling");
  EXPECT_EQ (m_handler.getWorkoutNotes (),
             "10 min Z2 warmup, 4x(4 min VO2max / 5 min recovery)");
  auto intervals{ m_handler.getIntervals () };
  EXPECT_EQ (*intervals.at (0).getIntensity ().getPercentFTP (Level::Low), 60);
  EXPECT_EQ (*intervals.at (0).getIntensity ().getPercentFTP (Level::High),
             75);
  EXPECT_EQ (*intervals.at (1).getIntensity ().getPercentFTP (Level::Low),
             110);
  EXPECT_EQ (*intervals.at (1).getIntensity ().getPercentFTP (Level::High),
             120);
  EXPECT_EQ (
      *intervals.at (1).subIntervalAt (0).getIntensity ().getPercentFTP (
          Level::Low),
      50);
  EXPECT_EQ (
      *intervals.at (1).subIntervalAt (0).getIntensity ().getPercentFTP (
          Level::High),
      60);
  EXPECT_EQ (intervals.at (1).getRepeats (), 4);
}

class FitWriteTester : public ::testing::Test
{
protected:
  std::filesystem::path testfile{ "Test.fit" };
  FitHandler m_handler{ testfile };
  Workout workout{ "TestWorkout", "TestNotes." };
  static const constexpr uint16_t ftp{ 365 };

  void SetUp () override
  {
    // Warm Up @ 50 - 60% FTP for 10 min.
    workout.addInterval (Interval{
        Intensity{ IntensityPair{ 50, 60 }, IntensityUnit::PercentFTP, ftp },
        std::chrono::seconds (600) });

    // 4 min. VO2Max @ 105 - 110% FTP
    workout.addInterval (Interval{
        Intensity{ IntensityPair{ 105, 110 }, IntensityUnit::PercentFTP, ftp },
        std::chrono::seconds (240) });

    // Recovery 5 min. @ 50 - 60% FTP
    workout.addInterval (Interval{
        Intensity{ IntensityPair{ 50, 60 }, IntensityUnit::PercentFTP, ftp },
        std::chrono::seconds (300) });

    // 12 x 30/30 @ 115 - 130 % FTP
    Interval HIIT{ Intensity{ IntensityPair{ 115, 130 },
                              IntensityUnit::PercentFTP, ftp },
                   std::chrono::seconds (30) };
    HIIT.addSubInterval (
        Interval{ Intensity{ 50, IntensityUnit::PercentFTP, ftp },
                  std::chrono::seconds (30) });
    HIIT.setRepeats (12);
    workout.addInterval (std::move (HIIT));

    // 10 min. Recovery @ 50 - 60% FTP
    workout.addInterval (Interval{
        Intensity{ IntensityPair{ 50, 60 }, IntensityUnit::PercentFTP, ftp },
        std::chrono::seconds (600) });

    // 10 min. Sweet Spot @ 85 - 95% FTP
    workout.addInterval (Interval{
        Intensity{ IntensityPair{ 85, 95 }, IntensityUnit::PercentFTP, ftp },
        std::chrono::seconds (600) });

    // Cool down 5 min. @ 50 - 60% FTP
    workout.addInterval (Interval{
        Intensity{ IntensityPair{ 50, 60 }, IntensityUnit::PercentFTP, ftp },
        std::chrono::seconds (300) });
  }
};
TEST_F (FitWriteTester, WriteTest)
{
  if (auto retVal{ workout.writeFile (m_handler) }; !retVal)
    {
      FAIL () << retVal.error ();
    }
}
}; // namespace fitFiles
}; // namespace Workouts