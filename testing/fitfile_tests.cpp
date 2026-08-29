#include <gtest/gtest.h>

import std;
import workoutlib;
import fitmodule;
import fitfiles;
import sha256;

// Test Workout.fit file generated using xxd -i Workout.fit
constexpr std::array WorkoutFile{
  0x0e, 0x20, 0xa6, 0x52, 0x64, 0x01, 0x00, 0x00, 0x2e, 0x46, 0x49, 0x54, 0x88,
  0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x02, 0x84, 0x00, 0x01, 0x00,
  0x02, 0x02, 0x84, 0x04, 0x04, 0x86, 0x00, 0xff, 0x00, 0x05, 0x01, 0x00, 0xd1,
  0xb0, 0xf4, 0x44, 0x40, 0x00, 0x00, 0x1a, 0x00, 0x04, 0x04, 0x01, 0x00, 0x08,
  0x0c, 0x07, 0x11, 0x42, 0x07, 0x06, 0x02, 0x84, 0x00, 0x02, 0x48, 0x49, 0x54,
  0x20, 0x57, 0x6f, 0x72, 0x6b, 0x6f, 0x75, 0x74, 0x00, 0x48, 0x49, 0x54, 0x20,
  0x49, 0x6e, 0x74, 0x65, 0x72, 0x76, 0x61, 0x6c, 0x20, 0x6d, 0x69, 0x74, 0x20,
  0x34, 0x20, 0x6d, 0x69, 0x6e, 0x2e, 0x20, 0x56, 0x4f, 0x32, 0x4d, 0x61, 0x78,
  0x2c, 0x20, 0x31, 0x32, 0x78, 0x33, 0x30, 0x2f, 0x33, 0x30, 0x20, 0x75, 0x6e,
  0x64, 0x20, 0x53, 0x77, 0x65, 0x65, 0x74, 0x20, 0x53, 0x70, 0x6f, 0x74, 0x20,
  0x49, 0x6e, 0x74, 0x65, 0x72, 0x76, 0x61, 0x6c, 0x2e, 0x00, 0x09, 0x00, 0x40,
  0x00, 0x00, 0x1b, 0x00, 0x07, 0x07, 0x01, 0x00, 0x01, 0x01, 0x00, 0x02, 0x04,
  0x86, 0x03, 0x01, 0x00, 0x05, 0x04, 0x86, 0x06, 0x04, 0x86, 0xfe, 0x02, 0x84,
  0x00, 0x00, 0x00, 0xc0, 0x27, 0x09, 0x00, 0x04, 0x32, 0x00, 0x00, 0x00, 0x3c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xa9, 0x03, 0x00, 0x04,
  0x69, 0x00, 0x00, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  0xe0, 0x93, 0x04, 0x00, 0x04, 0x32, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x30, 0x75, 0x00, 0x00, 0x04, 0x73, 0x00, 0x00,
  0x00, 0x82, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x30, 0x75, 0x00,
  0x00, 0x04, 0x32, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x04, 0x00, 0x40,
  0x00, 0x00, 0x1b, 0x00, 0x04, 0x01, 0x01, 0x00, 0x02, 0x04, 0x86, 0x04, 0x04,
  0x86, 0xfe, 0x02, 0x84, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
  0x00, 0x05, 0x00, 0x40, 0x00, 0x00, 0x1b, 0x00, 0x07, 0x07, 0x01, 0x00, 0x01,
  0x01, 0x00, 0x02, 0x04, 0x86, 0x03, 0x01, 0x00, 0x05, 0x04, 0x86, 0x06, 0x04,
  0x86, 0xfe, 0x02, 0x84, 0x00, 0x00, 0x00, 0xc0, 0x27, 0x09, 0x00, 0x04, 0x32,
  0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0xc0,
  0x27, 0x09, 0x00, 0x04, 0x55, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x07,
  0x00, 0x00, 0x00, 0x00, 0xe0, 0x93, 0x04, 0x00, 0x04, 0x32, 0x00, 0x00, 0x00,
  0x3c, 0x00, 0x00, 0x00, 0x08, 0x00, 0xca, 0x6f
};

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
    m_wktStep.SetDurationTime (1);
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
TEST_F (FitReadTester, WorkoutStepSubIntervalTester)
{
  fit::WorkoutStepMesg parentMsg = m_wktStep;
  fit::WorkoutStepMesg subIntervalMsg = m_wktStep;
  fit::WorkoutStepMesg repeatMsg = m_wktStep;
  constexpr const uint16_t parentLoInt{ 88 };
  constexpr const uint16_t parentHiInt{ 93 };
  constexpr const std::chrono::seconds parentDur{ 1 };

  constexpr const uint16_t subLoInt{ 50 };
  constexpr const uint16_t subHiInt{ 65 };
  constexpr const std::chrono::seconds subDur{ 2 };

  // parent interval
  parentMsg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  parentMsg.SetCustomTargetPowerLow (parentLoInt);
  parentMsg.SetCustomTargetPowerHigh (parentHiInt);
  parentMsg.SetDurationTime (parentDur.count ());
  auto parent{ m_listener.getFitInterval (parentMsg) };
  EXPECT_TRUE (parent);
  m_handler.addInterval (std::move (*parent));

  // subInterval
  subIntervalMsg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  subIntervalMsg.SetCustomTargetPowerLow (subLoInt);
  subIntervalMsg.SetCustomTargetPowerHigh (subHiInt);
  subIntervalMsg.SetDurationTime (subDur.count ());
  auto subInterval{ m_listener.getFitInterval (subIntervalMsg) };
  EXPECT_TRUE (subInterval);
  m_handler.addInterval (std::move (*subInterval));

  // repeat message
  repeatMsg.SetDurationType (FIT_WKT_STEP_DURATION_REPEAT_UNTIL_STEPS_CMPLT);
  // repeat 2 times
  repeatMsg.SetTargetValue (2);

  // illegal index above number of subIntervals
  repeatMsg.SetDurationValue (2);

  // Also check the unexpected chain
  fit::Mesg msg (repeatMsg);
  m_listener.OnMesg (msg);
  EXPECT_EQ (m_listener.errMesg,
             "Invalid repeat message. No interval at index 2");

  // legal index (Repeat from parent, this is the smallest possible number)
  repeatMsg.SetDurationValue (0);
  auto repeat{ m_listener.getFitInterval (repeatMsg) };
  EXPECT_EQ (repeat.error (), "Workout repeat step.");

  auto intervals{ m_handler.getIntervals () };
  EXPECT_EQ (intervals.at (0).count (), 4);

  auto intervalIt{ intervals.at (0).begin () };

  // First step should be the parent interval
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::Low),
             parentLoInt);
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::High),
             parentHiInt);
  EXPECT_EQ (intervalIt->getDuration (), parentDur);

  // Second step subInterval
  ++intervalIt;
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::Low),
             subLoInt);
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::High),
             subHiInt);
  EXPECT_EQ (intervalIt->getDuration (), subDur);

  // Third step parent interval
  ++intervalIt;
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::Low),
             parentLoInt);
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::High),
             parentHiInt);
  EXPECT_EQ (intervalIt->getDuration (), parentDur);

  // Fourth step subInterval
  ++intervalIt;
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::Low),
             subLoInt);
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::High),
             subHiInt);
  EXPECT_EQ (intervalIt->getDuration (), subDur);

  // Now it should be the sentinel
  ++intervalIt;
  EXPECT_EQ (intervalIt, intervals.at (0).end ());
}

TEST_F (FitReadTester, WorkoutMsgTester)
{
  fit::WorkoutMesg workoutMsg;
  std::string_view workoutName{ "Workout" };
  std::string_view workoutNotes{ "ÄÖÜßäöü" };
  workoutMsg.SetWktName (sv2wstring (workoutName));
  workoutMsg.SetWktDescription (sv2wstring (workoutNotes));

  fit::Mesg msg (workoutMsg);
  m_listener.OnMesg (msg);
  EXPECT_EQ (m_handler.getWorkoutName (), workoutName);
  EXPECT_EQ (m_handler.getWorkoutNotes (), workoutNotes);
}

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

TEST (SHA256, HashTest)
{
  auto result{ sha256sum ("Activity.fit") };
  std::println ("SHA256sum: {}", result);
}

}; // namespace fitFiles
}; // namespace Workouts