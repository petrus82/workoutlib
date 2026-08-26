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
  void SetUp () override
  {
    m_wktStep.SetMessageIndex (0);
    m_wktStep.SetIntensity (FIT_INTENSITY_ACTIVE);
    m_wktStep.SetDurationType (FIT_WKT_STEP_DURATION_TIME);
    m_wktStep.SetDurationValue (1000);
  }

protected:
  std::filesystem::path testfile{ "Workout.fit" };
  std::filesystem::path non_existent{ "No_file.fit" };

  fit::WorkoutStepMesg m_wktStep;
  FitHandler m_handler{ testfile };
  FitHandler::Listener m_listener{ &m_handler };

private:
};
/* TEST_F (FitReadTester, ReadFileTest)
{
  FitHandler handler{ testfile };
  if (auto retVal (handler.checkFile ()); !retVal)
    {
      FAIL () << retVal.error ();
    }
  if (auto retVal{ handler.readFile () }; !retVal)
    {
      FAIL () << retVal.error ();
    }
  EXPECT_EQ (handler.getWorkoutName (), "4x4 VO2Max Cycling");
  EXPECT_EQ (handler.getWorkoutNotes (),
             "10 min Z2 warmup, 4x(4 min VO2max / 5 min recovery)");
  auto intervals{ handler.getIntervals () };
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
} */

TEST_F (FitReadTester, InvalidFilesTest)
{
  FitHandler invalid{ non_existent };
  EXPECT_FALSE (invalid.checkFile ().has_value ());
  EXPECT_FALSE (invalid.readFile ().has_value ());
  FitHandler activity{ std::filesystem::path ("Activity.fit") };
  EXPECT_TRUE (activity.checkFile ());
  EXPECT_FALSE (activity.readFile ().has_value ());
}
TEST_F (FitReadTester, WorkoutStepWattsTester)
{
  m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  m_wktStep.SetCustomTargetPowerLow (1100);
  m_wktStep.SetCustomTargetPowerHigh (1200);
  auto retVal{ m_listener.getFitInterval (m_wktStep) };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (), "watts");
  EXPECT_EQ (*retVal->getIntensity ().getWatts (Level::Low), 100);
  EXPECT_EQ (*retVal->getIntensity ().getWatts (Level::High), 200);
}
TEST_F (FitReadTester, WorkoutStepFtpTester)
{
  m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  m_wktStep.SetCustomTargetPowerLow (70);
  m_wktStep.SetCustomTargetPowerHigh (250);
  auto retVal{ m_listener.getFitInterval (m_wktStep) };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (), "\%FTP");
  EXPECT_EQ (*retVal->getIntensity ().getPercentFTP (Level::Low), 70);
  EXPECT_EQ (*retVal->getIntensity ().getPercentFTP (Level::High), 250);
}
TEST_F (FitReadTester, WorkoutStepPwrZoneTester)
{
  m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  m_wktStep.SetTargetPowerZone (4);
  auto retVal{ m_listener.getFitInterval (m_wktStep) };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (), "power zone");
  EXPECT_EQ (*retVal->getIntensity ().getPowerZone (), 4);
}
TEST_F (FitReadTester, WorkoutStepHrBPMTester)
{
  m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  m_wktStep.SetCustomTargetHeartRateLow (250);
  m_wktStep.SetCustomTargetHeartRateHigh (280);
  auto retVal{ m_listener.getFitInterval (m_wktStep) };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (), "bpm");
  EXPECT_EQ (*retVal->getIntensity ().getHeartRateBPM (Level::Low), 150);
  EXPECT_EQ (*retVal->getIntensity ().getHeartRateBPM (Level::High), 180);
}
TEST_F (FitReadTester, WorkoutStepHrPercentTester)
{
  m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  m_wktStep.SetCustomTargetHeartRateLow (80);
  m_wktStep.SetCustomTargetHeartRateHigh (95);
  auto retVal{ m_listener.getFitInterval (m_wktStep) };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (), "\%max heart rate");
  EXPECT_EQ (*retVal->getIntensity ().getPercentMaxHR (Level::Low), 80);
  EXPECT_EQ (*retVal->getIntensity ().getPercentMaxHR (Level::High), 95);
}
TEST_F (FitReadTester, WorkoutStepHrZoneTester)
{
  m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  m_wktStep.SetTargetHrZone (4);
  auto retVal{ m_listener.getFitInterval (m_wktStep) };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (), "heart rate zone");
  EXPECT_EQ (*retVal->getIntensity ().getHeartRateZone (), 4);
}

// TODO: Check SubIntervals

class FitWriteTester : public ::testing::Test
{
protected:
  std::filesystem::path testfile{ "Workout.fit" };
  FitHandler m_handler{ testfile };
  Workout workout{
    "HIT Workout",
    "HIT Interval mit 4 min. VO2Max, 12x30/30 und Sweet Spot Interval."
  };
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
  if (auto retVal{ workout.writeFile (m_handler, testfile) }; !retVal)
    {
      FAIL () << retVal.error ();
    }
}
}; // namespace fitFiles
}; // namespace Workouts