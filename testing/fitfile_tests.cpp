#include <gtest/gtest.h>

import std;
import workoutlib;
import fitmodule;
import fitfiles;

namespace Workouts
{
namespace fitFiles
{
class FitTests : public testing::Test
{
public:
  void SetUp () override
  {
    fitData.SetIntensity (FIT_INTENSITY_ACTIVE);
    fitData.SetDurationType (FIT_WKT_STEP_DURATION_TIME);
    fitData.SetMessageIndex (0);
  }
  void TearDown () override {}

  static constexpr uint8_t maxHeartRate{ 180 };
  static constexpr uint16_t ftp{ 300 };
  static constexpr uint16_t intervalDur1{ 300 };
  static constexpr uint16_t intervalDur2{ 400 };
  static constexpr uint16_t powerLow{ 75 };
  static constexpr uint16_t powerLowFtp{ 225 };
  static constexpr uint16_t powerHigh{ 110 };
  static constexpr uint16_t powerRelHighFtp{ 330 };
  static constexpr uint16_t AbsoluteHeartRateOffset{ 100 };
  static constexpr uint16_t AbsolutePowerOffset{ 1000 };
  fit::WorkoutStepMesg fitData;
  /*   CapacityValues capValues{ .maxHeartRate = maxHeartRate, .ftp = ftp }; */
};
TEST_F (FitTests, AbsPowIntervalReadTest)
{
  fitData.SetMessageIndex (0);
  fitData.SetDurationValue (intervalDur1);
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  fitData.SetCustomTargetPowerLow (powerLow + AbsolutePowerOffset);
  fitData.SetCustomTargetPowerHigh (powerHigh + AbsolutePowerOffset);
  /*   auto interval{ Workouts::fitFiles::getFitInterval (fitData, capValues)
    }; EXPECT_TRUE (interval); EXPECT_EQ (interval->getDuration (),
    std::chrono::seconds (intervalDur1)); EXPECT_EQ (interval->getIntensity
    (IntensityType::PowerAbsHigh), powerHigh); EXPECT_EQ
    (interval->getIntensity (IntensityType::PowerAbsLow), powerLow); */
}
TEST_F (FitTests, RelPowIntervalReadTest)
{
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  fitData.SetDurationValue (intervalDur2);
  fitData.SetCustomTargetPowerLow (powerLow);
  fitData.SetCustomTargetPowerHigh (powerHigh);
  /*   auto interval{ getFitInterval (fitData, capValues) };
    EXPECT_TRUE (interval);
    EXPECT_EQ (interval->getDuration (), std::chrono::seconds (intervalDur2));
    EXPECT_EQ (interval->getIntensity (IntensityType::PowerAbsHigh),
               powerRelHighFtp);
    EXPECT_EQ (interval->getIntensity (IntensityType::PowerAbsLow),
    powerLowFtp); */
}
TEST_F (FitTests, AbsHRIntervalReadTest)
{
  constexpr uint8_t heartRateLow{ 120 };
  constexpr uint8_t heartRateHigh{ 150 };
  // Absolute HeartRate
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  fitData.SetDurationValue (intervalDur1);
  fitData.SetCustomTargetHeartRateLow (heartRateLow + AbsoluteHeartRateOffset);
  fitData.SetCustomTargetHeartRateHigh (heartRateHigh
                                        + AbsoluteHeartRateOffset);
  /*   auto interval{ getFitInterval (fitData, capValues) };
    EXPECT_TRUE (interval);
    EXPECT_EQ (interval->getDuration (), std::chrono::seconds (intervalDur1));
    EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateAbsLow),
               heartRateLow);
    EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateAbsHigh),
               heartRateHigh); */
}
TEST_F (FitTests, RelHRIntervalReadTest)
{
  constexpr uint8_t heartRateAbsLow{ 117 };
  constexpr uint8_t heartRateRelLow{ 65 };
  constexpr uint8_t heartRateAbsHigh{ 162 };
  constexpr uint8_t heartRateRelHigh{ 90 };
  // Absolute HeartRate
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  fitData.SetDurationValue (intervalDur2);
  fitData.SetCustomTargetHeartRateLow (heartRateRelLow);
  fitData.SetCustomTargetHeartRateHigh (heartRateRelHigh);
  /*   auto interval{ getFitInterval (fitData, capValues) };
    EXPECT_TRUE (interval);
    EXPECT_EQ (interval->getDuration (), std::chrono::seconds (intervalDur2));
    EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateAbsLow),
               heartRateAbsLow);
    EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateAbsHigh),
               heartRateAbsHigh); */
}
TEST_F (FitTests, PowerZoneReadTest)
{
  constexpr uint8_t pwrZone{ 2 };
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  fitData.SetDurationValue (intervalDur1);
  fitData.SetTargetPowerZone (pwrZone);
  /*   auto interval{ getFitInterval (fitData, capValues) };
    EXPECT_TRUE (interval);
    EXPECT_EQ (interval->getIntensity (IntensityType::PowerZone), pwrZone);
    EXPECT_EQ (interval->getIntensity (IntensityType::PowerRelLow),
               pwZone.Z2.first);
    EXPECT_EQ (interval->getIntensity (IntensityType::PowerRelHigh),
               pwZone.Z2.second); */
}

TEST_F (FitTests, HeartRateZoneReadTest)
{
  constexpr uint8_t zone{ 3 };
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  fitData.SetDurationValue (intervalDur2);
  fitData.SetTargetHrZone (zone);
  /*   auto interval{ getFitInterval (fitData, capValues) };
    EXPECT_TRUE (interval);
    EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateZone), zone);
    EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateRelLow),
               hrZone.Z3.first);
    EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateRelHigh),
               hrZone.Z3.second); */
}

TEST_F (FitTests, IntervalWriteAbsPowerTest)
{
  /*   Interval interval{ *Interval::create (AbsolutePower{ powerLow, ftp },
                                          IntensityType::PowerAbsLow,
                                          std::chrono::seconds (intervalDur1))
    }; auto workoutStepMsg{ fitFiles::writeFitInterval (interval) };
    EXPECT_TRUE (workoutStepMsg.IsCustomTargetPowerLowValid ());
    EXPECT_EQ (workoutStepMsg.GetCustomTargetPowerLow (),
               powerLow + AbsolutePowerOffset);
    EXPECT_TRUE (workoutStepMsg.IsDurationTimeValid ());
    EXPECT_EQ (workoutStepMsg.GetDurationValue (), intervalDur1 * msecInSec);
  */
}
TEST_F (FitTests, IntervalWriteRelPowerTest)
{
  /*   Interval interval{ *Interval::create (RelativePower{ powerHigh, ftp },
                                          IntensityType::PowerRelHigh,
                                          std::chrono::seconds (intervalDur1))
    }; auto workoutStepMsg{ fitFiles::writeFitInterval (interval) };
    EXPECT_TRUE (workoutStepMsg.IsCustomTargetPowerHighValid ());
    EXPECT_EQ (workoutStepMsg.GetCustomTargetPowerHigh (), powerHigh);
    EXPECT_TRUE (workoutStepMsg.IsDurationTimeValid ());
    EXPECT_EQ (workoutStepMsg.GetDurationValue (), intervalDur1 * msecInSec);
  */
}
TEST_F (FitTests, IntervalWritePowerZoneTest)
{
  /*   Interval interval{ *Interval::create (PowerZone{ PWZ::P3, ftp },
                                          IntensityType::PowerZone,
                                          std::chrono::seconds (intervalDur1))
    }; auto workoutStepMsg{ fitFiles::writeFitInterval (interval) };
    EXPECT_TRUE (workoutStepMsg.IsTargetPowerZoneValid ());
    EXPECT_EQ (workoutStepMsg.GetTargetPowerZone (), 3); */
}
TEST_F (FitTests, IntervalWriteHrZoneTest)
{
  /*   Interval interval{ *Interval::create (HeartRateZone{ HRZ::H3,
    maxHeartRate }, IntensityType::HeartRateZone, std::chrono::seconds
    (intervalDur1)) }; auto workoutStepMsg{ fitFiles::writeFitInterval
    (interval) }; EXPECT_TRUE (workoutStepMsg.IsTargetHrZoneValid ());
    EXPECT_EQ (workoutStepMsg.GetTargetHrZone (), 3); */
}

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

}; // namespace fitFiles
};