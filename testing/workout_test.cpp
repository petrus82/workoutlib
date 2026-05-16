#include "gmock/gmock.h"
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

std::filesystem::path unreadableFile ()
{
  auto path = std::filesystem::temp_directory_path () / "unreadable.txt";
  std::ofstream file (path);
  file.close ();
  std::filesystem::permissions (path, std::filesystem::perms::owner_write);
  return path;
}
using namespace std::string_literals;
namespace Workouts
{
using testing::HasSubstr;

TEST (WorkoutTests, GetTagsTest)
{
  std::string_view testData{ "FIRST TAG=this is the content. \n"
                             "This belongs to the first tag.\n"
                             "SECOND TAG=And this is also content.\n" };
  std::string_view tagSeparator{ "=" };
  auto result{ getTags (testData, tagSeparator) };
  EXPECT_EQ (result.at (0).first, "FIRST TAG");
  EXPECT_EQ (result.at (0).second,
             "this is the content. This belongs to the first tag.");
  EXPECT_EQ (result.at (1).first, "SECOND TAG");
}

class FileWriteTests : public testing::Test
{
public:
  void SetUp () override
  {
    constexpr const uint16_t ftp{ 300 };
    constexpr const IntensityType intervalType{ IntensityType::PowerAbsLow };
    Intervals intervals{ Interval{ 200, 300s, intervalType, ftp },
                         Interval{ 150, 400s, intervalType, ftp } };
    workout.setIntervals (intervals);
  }
  void TearDown () override {}

private:
  Workout workout{ "Test", "Notes" };
  std::string fileContent;
};

TEST_F (FileWriteTests, ContentTest) {}

TEST (WorkoutTests, ReadableTest)
{
  EXPECT_FALSE (Workouts::getFileStream (unreadableFile ()));
  std::filesystem::path testfile{ "Workout.fit" };
  EXPECT_TRUE (Workouts::getFileStream (testfile));
}
TEST (WorkoutTests, FitTest)
{
  std::filesystem::path testfile{ "Workout.fit" };
  fit::Decode decoder;
  EXPECT_TRUE (Workouts::isValidFit (testfile, decoder));
  EXPECT_FALSE (Workouts::isValidFit (unreadableFile (), decoder));
}
TEST (FileReadTests, ReadContentTest)
{
  std::filesystem::path testFile{ std::filesystem::temp_directory_path ()
                                  / "Testfile" };
  std::ofstream testStream (testFile, std::ios::out);
  std::string_view testContent{ "TestContent" };
  testStream << testContent;
  testStream.close ();

  EXPECT_TRUE (readFileContent (testFile));
  EXPECT_EQ (testContent, readFileContent (testFile).value ());
  std::filesystem::path nonexistent ("/tmp/nonexistent.file");
  EXPECT_FALSE (readFileContent (nonexistent));
  EXPECT_EQ (readFileContent (nonexistent).error (), "Cannot open file.");
  EXPECT_FALSE (readFileContent (unreadableFile ()));
  EXPECT_EQ (readFileContent (unreadableFile ()).error (),
             "Cannot open file.");
}
TEST (ErgTests, WorkoutReadTest)
{
  using namespace textFiles;
  std::string_view testfile{
    "[COURSE HEADER]\nVERSION = 2\nUNITS = METRIC\n"
    "DESCRIPTION = Notes\nFILE NAME = Workout\nFTP = 300\n"
    "MINUTES WATTS\n[END COURSE HEADER]\n[COURSE DATA]\n"
    "0.000\t100\n5.000\t100\n5.000\t200\n11.667\t200\n"
  };
  auto returnPair{ processContent (testfile, ergFile) };
  auto tags{ getTags (returnPair.first, ergFile.headerSeparator) };
  auto workout = getWorkout (returnPair.first, ergFile);
  EXPECT_EQ (workout.getNotes (), "Notes");
  EXPECT_EQ (workout.getFtp (), 300);
}
TEST (ErgTests, WorkoutWriteTest)
{
  using namespace textFiles;
  std::stringstream stream;
  constexpr const std::uint16_t ftp{ 300 };
  Workout workout{ "Workout", "Notes" };
  workout.setFtp (ftp);
  writeWorkout (stream, ergFile, workout);
  std::array expected{ "[COURSE HEADER]\n",     "VERSION = 2\n",
                       "UNITS = METRIC\n",      "DESCRIPTION = Notes\n",
                       "FILE NAME = Workout\n", "FTP = 300\n",
                       "MINUTES WATTS\n",       "[END COURSE HEADER]\n",
                       "[COURSE DATA]\n" };
  for (const auto &check : expected)
    {
      EXPECT_TRUE (stream.str ().contains (check));
    }
}
TEST (ErgTests, IntervalWriteTest)
{
  using namespace textFiles;
  std::stringstream stream;
  constexpr const std::uint16_t ftp{ 200 };
  Interval first{ 100, std::chrono::seconds (300), IntensityType::PowerAbsHigh,
                  ftp };
  Interval second{ 200, std::chrono::seconds (400),
                   IntensityType::PowerAbsHigh, ftp };
  writeIntensityDuration (stream, ergFile, first, IntensityType::PowerAbsHigh,
                          0);
  std::string expected{ "0.000\t100\n5.000\t100\n5.000\t200\n11.667\t200\n" };
  writeIntensityDuration (stream, ergFile, second, IntensityType::PowerAbsHigh,
                          5);
  EXPECT_EQ (stream.str (), expected);
}
TEST (ErgTests, IntervalReadTest)
{
  using namespace textFiles;
  std::string_view testfile{
    "[COURSE HEADER]\nVERSION = 2\nUNITS = METRIC\n"
    "DESCRIPTION = Notes\nFILE NAME = Workout\nFTP = 300\n"
    "MINUTES WATTS\n[END COURSE HEADER]\n[COURSE DATA]\n"
    "0.000\t100\n5.000\t100\n5.000\t200\n11.667\t200\n"
  };
  auto returnPair{ processContent (testfile, ergFile) };
  auto intervals = getTextIntervals (returnPair.second, ergFile,
                                     IntensityType::PowerAbsHigh, 300);
  EXPECT_TRUE (intervals);
  EXPECT_EQ (intervals->front ().getIntensity (IntensityType::PowerAbsLow),
             100);
  EXPECT_EQ (intervals->front ().getDuration (), std::chrono::seconds (300));
  EXPECT_EQ (intervals->back ().getIntensity (IntensityType::PowerAbsLow),
             200);
  EXPECT_EQ (intervals->back ().getDuration (), std::chrono::seconds (400));
}

TEST (MrcTests, IntervalWriteTest)
{
  using namespace textFiles;
  std::stringstream stream;
  constexpr const std::uint16_t ftp{ 200 };
  constexpr const std::uint16_t relPowerLow{ 50 };
  constexpr const std::uint16_t relPowerHigh{ 75 };
  constexpr const long shortDuration{ 300 };
  constexpr const long longDuration{ 400 };
  Interval first{ relPowerLow, std::chrono::seconds (shortDuration),
                  IntensityType::PowerRelHigh, ftp };
  Interval second{ relPowerHigh, std::chrono::seconds (longDuration),
                   IntensityType::PowerRelHigh, ftp };
  writeIntensityDuration (stream, mrcFile, first, IntensityType::PowerRelHigh,
                          0);
  std::string expected{ "0.000\t50\n5.000\t50\n" };
  EXPECT_EQ (stream.str (), expected);
  expected += "5.000\t75\n11.667\t75\n";
  writeIntensityDuration (stream, mrcFile, second, IntensityType::PowerRelHigh,
                          5);
  EXPECT_EQ (stream.str (), expected);
}
TEST (MrcTests, IntervalReadTest)
{
  using namespace textFiles;
  std::string_view testfile{
    "[COURSE HEADER]\nVERSION = 2\nUNITS = METRIC\n"
    "DESCRIPTION = Notes\nFILE NAME = Workout\n"
    "MINUTES PERCENT\n[END COURSE HEADER]\n[COURSE DATA]\n"
    "0.000\t50\n5.000\t50\n5.000\t75\n11.667\t75\n"
  };
  auto returnPair{ processContent (testfile, mrcFile) };
  auto intervals{ getTextIntervals (returnPair.second, mrcFile,
                                    IntensityType::PowerRelHigh, 300) };
  EXPECT_TRUE (intervals);
  EXPECT_EQ (intervals->front ().getIntensity (IntensityType::PowerAbsLow),
             150);
  EXPECT_EQ (intervals->front ().getDuration (), std::chrono::seconds (300));
  EXPECT_EQ (intervals->back ().getIntensity (IntensityType::PowerRelLow), 75);
  EXPECT_EQ (intervals->back ().getDuration (), std::chrono::seconds (400));
}
TEST (MrcTests, WorkoutWriteTest)
{
  using namespace textFiles;
  std::stringstream stream;
  Workout workout{ "Workout", "Notes" };
  writeWorkout (stream, mrcFile, workout);
  std::array expected{ "[COURSE HEADER]\n",     "VERSION = 2\n",
                       "UNITS = METRIC\n",      "DESCRIPTION = Notes\n",
                       "FILE NAME = Workout\n", "MINUTES PERCENT\n",
                       "[END COURSE HEADER]\n", "[COURSE DATA]\n" };
  for (const auto &check : expected)
    {
      EXPECT_TRUE (stream.str ().contains (check));
    }
}
TEST (MrcTests, WorkoutReadTest)
{
  using namespace textFiles;
  std::string_view file{
    "[COURSE HEADER]\nVERSION = 2\nUNITS = METRIC\n"
    "DESCRIPTION = Notes\nFILE NAME = Workout\n"
    "MINUTES PERCENT\n[END COURSE HEADER]\n[COURSE DATA]\n"
  };
  auto returnPair{ processContent (file, mrcFile) };
  auto workout{ getWorkout (returnPair.first, mrcFile) };
  EXPECT_EQ (workout.getName (), "Workout");
  EXPECT_EQ (workout.getNotes (), "Notes");
}

TEST (PlanTests, WorkoutWriteTest)
{
  using namespace planFiles;
  std::stringstream stream;
  Workout workout{ "Workout", "Notes" };
  writeWorkout (stream, planFile, workout);
  std::array expected{ "=HEADER=\n\n",       "NAME = Workout\n",
                       "DURATION = 0\n\n",   "PLAN_TYPE = 0\n",
                       "WORKOUT_TYPE = 0\n", "DESCRIPTION = Notes\n",
                       "=STREAM=\n\n" };
  for (const auto &check : expected)
    {
      EXPECT_PRED1 ([&] (std::string_view string)
                      { return stream.str ().contains (string); }, check);
    }
}
TEST (PlanTests, WorkoutReadTest)
{
  using namespace planFiles;
  std::string_view file{ "=HEADER=\n\n"
                         "NAME=Workout\n\n"
                         "DURATION=0\n"
                         "PLAN_TYPE=0\n"
                         "WORKOUT_TYPE=0\n"
                         "DESCRIPTION=Notes \n"
                         "DESCRIPTION=Second Line\n"
                         "=STREAM=\n\n" };
  auto returnPair{ processContent (file, planFile) };
  auto tags{ getTags (returnPair.first, planFile.headerSeparator) };
  auto workout{ getWorkout (returnPair.first, planFile) };
  EXPECT_EQ (workout.getName (), "Workout");
  EXPECT_EQ (workout.getNotes (), "Notes Second Line");
}
TEST (PlanTests, IntervalWriteTest)
{
  using namespace planFiles;
  std::stringstream stream;
  constexpr const std::uint16_t ftp{ 200 };
  constexpr const std::uint16_t relPower{ 75 };
  constexpr const long shortDuration{ 300 };
  constexpr const long longDuration{ 400 };
  std::array expected{
    "=INTERVAL=\n\n",      "PWR_LO=150\n",
    "PWR_HI=150\n",        "MESG_DURATION_SEC>=300?EXIT\n",
    "=INTERVAL=\n\n",      "PERCENT_FTP_LO=75\n",
    "PERCENT_FTP_HI=75\n", "MESG_DURATION_SEC>=400?EXIT\n"
  };
  Interval first{ relPower, std::chrono::seconds (shortDuration),
                  IntensityType::PowerRelHigh, ftp };
  Interval second{ relPower, std::chrono::seconds (longDuration),
                   IntensityType::PowerRelHigh, ftp };
  writeIntensityTime (stream, planFile, first, IntensityType::PowerAbsHigh);
  writeIntensityTime (stream, planFile, second, IntensityType::PowerRelHigh);
  for (const auto &check : expected)
    {
      EXPECT_THAT (stream.str (), HasSubstr (check));
    }
}

TEST (PlanTests, IntervalReadTest)
{
  using namespace planFiles;
  std::string_view testfile{ "=HEADER=\n\n"
                             "NAME=Workout\n\n"
                             "DURATION=0\n"
                             "DESCRIPTION=Notes\n\n"
                             "PLAN_TYPE=0\n"
                             "WORKOUT_TYPE=0\n"
                             "=STREAM=\n\n"
                             "=INTERVAL=\n\n"
                             "PWR_LO=150\nPWR_HI=150\n"
                             "MESG_DURATION_SEC>=300?EXIT\n"
                             "=INTERVAL=\n\n"
                             "PWR_LO=75\nPWR_HI=75\n"
                             "MESG_DURATION_SEC>=400?EXIT\n"
                             "=INTERVAL=\n\n"
                             "PERCENT_FTP_LO=75\nPERCENT_FTP_HI=75\n"
                             "MESG_DURATION_SEC>=400?EXIT\n" };
  auto returnPair{ splitPlanContent (testfile) };
  auto retVal{ getPlanIntervals (returnPair.second, 300) };
  EXPECT_TRUE (retVal);
  const auto &intervals{ *retVal };
  EXPECT_EQ (intervals.front ().getIntensity (IntensityType::PowerAbsHigh),
             150);
  EXPECT_EQ (intervals.front ().getDuration (), std::chrono::seconds (300));
  EXPECT_EQ (intervals.back ().getIntensity (IntensityType::PowerRelHigh), 75);
  EXPECT_EQ (intervals.back ().getDuration (), std::chrono::seconds (400));
}

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
  fit::WorkoutStepMesg fitData;
  CapacityValues capValues{ .maxHeartRate = maxHeartRate, .ftp = ftp };
};
TEST_F (FitTests, AbsPowIntervalReadTest)
{
  fitData.SetMessageIndex (0);
  fitData.SetDurationValue (intervalDur1);
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  fitData.SetCustomTargetPowerLow (powerLow + AbsolutePowerOffset);
  fitData.SetCustomTargetPowerHigh (powerHigh + AbsolutePowerOffset);
  auto interval{ Workouts::fitFiles::getFitInterval (fitData, capValues) };
  EXPECT_TRUE (interval);
  EXPECT_EQ (interval->getDuration (), std::chrono::seconds (intervalDur1));
  EXPECT_EQ (interval->getIntensity (IntensityType::PowerAbsHigh), powerHigh);
  EXPECT_EQ (interval->getIntensity (IntensityType::PowerAbsLow), powerLow);
}
TEST_F (FitTests, RelPowIntervalReadTest)
{
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  fitData.SetDurationValue (intervalDur2);
  fitData.SetCustomTargetPowerLow (powerLow);
  fitData.SetCustomTargetPowerHigh (powerHigh);
  auto interval{ getFitInterval (fitData, capValues) };
  EXPECT_TRUE (interval);
  EXPECT_EQ (interval->getDuration (), std::chrono::seconds (intervalDur2));
  EXPECT_EQ (interval->getIntensity (IntensityType::PowerAbsHigh),
             powerRelHighFtp);
  EXPECT_EQ (interval->getIntensity (IntensityType::PowerAbsLow), powerLowFtp);
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
  auto interval{ getFitInterval (fitData, capValues) };
  EXPECT_TRUE (interval);
  EXPECT_EQ (interval->getDuration (), std::chrono::seconds (intervalDur1));
  EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateAbsLow),
             heartRateLow);
  EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateAbsHigh),
             heartRateHigh);
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
  auto interval{ getFitInterval (fitData, capValues) };
  EXPECT_TRUE (interval);
  EXPECT_EQ (interval->getDuration (), std::chrono::seconds (intervalDur2));
  EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateAbsLow),
             heartRateAbsLow);
  EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateAbsHigh),
             heartRateAbsHigh);
}
TEST_F (FitTests, PowerZoneReadTest)
{
  constexpr uint8_t pwrZone{ 2 };
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  fitData.SetDurationValue (intervalDur1);
  fitData.SetTargetPowerZone (pwrZone);
  auto interval{ getFitInterval (fitData, capValues) };
  EXPECT_TRUE (interval);
  EXPECT_EQ (interval->getIntensity (IntensityType::PowerZone), pwrZone);
  EXPECT_EQ (interval->getIntensity (IntensityType::PowerRelLow),
             pwZone.Z2.first);
  EXPECT_EQ (interval->getIntensity (IntensityType::PowerRelHigh),
             pwZone.Z2.second);
}

TEST_F (FitTests, HeartRateZoneReadTest)
{
  constexpr uint8_t zone{ 3 };
  fitData.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  fitData.SetDurationValue (intervalDur2);
  fitData.SetTargetHrZone (zone);
  auto interval{ getFitInterval (fitData, capValues) };
  EXPECT_TRUE (interval);
  EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateZone), zone);
  EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateRelLow),
             hrZone.Z3.first);
  EXPECT_EQ (interval->getIntensity (IntensityType::HeartRateRelHigh),
             hrZone.Z3.second);
}

TEST_F (FitTests, IntervalWriteAbsPowerTest)
{
  Interval interval{ powerLow, std::chrono::seconds (intervalDur1),
                     IntensityType::PowerAbsLow, ftp };
  auto workoutStepMsg{ fitFiles::writeFitInterval (interval) };
  EXPECT_TRUE (workoutStepMsg.IsCustomTargetPowerLowValid ());
  EXPECT_EQ (workoutStepMsg.GetCustomTargetPowerLow (),
             powerLow + AbsolutePowerOffset);
  EXPECT_TRUE (workoutStepMsg.IsDurationTimeValid ());
  EXPECT_EQ (workoutStepMsg.GetDurationValue (), intervalDur1 * msecInSec);
}
TEST_F (FitTests, IntervalWriteRelPowerTest)
{
  Interval interval{ powerHigh, std::chrono::seconds (intervalDur1),
                     IntensityType::PowerRelHigh, ftp };
  auto workoutStepMsg{ fitFiles::writeFitInterval (interval) };
  EXPECT_TRUE (workoutStepMsg.IsCustomTargetPowerHighValid ());
  EXPECT_EQ (workoutStepMsg.GetCustomTargetPowerHigh (), powerHigh);
  EXPECT_TRUE (workoutStepMsg.IsDurationTimeValid ());
  EXPECT_EQ (workoutStepMsg.GetDurationValue (), intervalDur1 * msecInSec);
}
TEST_F (FitTests, IntervalWritePowerZoneTest)
{
  Interval interval{ 3, std::chrono::seconds (intervalDur1),
                     IntensityType::PowerZone, ftp };
  auto workoutStepMsg{ fitFiles::writeFitInterval (interval) };
  EXPECT_TRUE (workoutStepMsg.IsTargetPowerZoneValid ());
  EXPECT_EQ (workoutStepMsg.GetTargetPowerZone (), 3);
}
TEST_F (FitTests, IntervalWriteHrZoneTest)
{
  Interval interval{ 3, std::chrono::seconds (intervalDur1),
                     IntensityType::HeartRateZone, maxHeartRate };
  auto workoutStepMsg{ fitFiles::writeFitInterval (interval) };
  EXPECT_TRUE (workoutStepMsg.IsTargetHrZoneValid ());
  EXPECT_EQ (workoutStepMsg.GetTargetHrZone (), 3);
}
}; // namespace fitFiles

TEST (RepeatTests, RepeatIntervalTest)
{
  constexpr uint16_t ftp{ 300 };
  constexpr uint16_t power1{ 400 };
  constexpr std::chrono::seconds duration{ 300 };
  constexpr uint16_t power2{ 200 };
  constexpr std::array expected{ power1, power2, power1, power2 };
  Interval interval{ power1, duration, IntensityType::PowerAbsLow, ftp };
  interval.addSubInterval (
      Interval (power2, duration, IntensityType::PowerAbsLow, ftp));
  interval.setRepeats (2);
  auto intensities{
    interval.getIntervalsExpanded ()
    | std::views::transform (
        [] (const Interval &interval)
          { return interval.getIntensity (IntensityType::PowerAbsLow); })
  };
  EXPECT_TRUE (std::ranges::equal (intensities, expected));
}

TEST (RemoveSubIntervalTest, RemoveSubInterval)
{
  constexpr uint16_t ftp{ 300 };
  constexpr uint16_t power1{ 400 };
  constexpr std::chrono::seconds duration{ 300 };
  constexpr uint16_t power2{ 200 };
  std::vector expected{ power1, power2, power1 };
  Interval interval{ power1, duration, IntensityType::PowerAbsLow, ftp };
  interval.addSubInterval (
      Interval{ power2, duration, IntensityType::PowerAbsLow, ftp });
  interval.addSubInterval (
      Interval{ power1, duration, IntensityType::PowerAbsLow, ftp });
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
  EXPECT_THROW (interval.removeSubInterval (1), std::out_of_range);
}
}; // namespace Workouts
int main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}