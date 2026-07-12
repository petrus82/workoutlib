#include "gmock/gmock.h"
#include <gtest/gtest.h>

import std;
import workoutlib;
import textfiles;
import planfiles;

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
      EXPECT_TRUE (stream.str ().contains (check))
          << "Expected string not found: " << check;
    }
}
TEST (ErgTests, IntervalWriteTest)
{
  using namespace textFiles;
  std::stringstream stream;
  constexpr const std::uint16_t ftp{ 200 };
  /*   Interval first{ *Interval::create (AbsolutePower{ 100, ftp },
                                       IntensityType::PowerAbsHigh, 300s) };
    Interval second{ *Interval::create (AbsolutePower{ 200, ftp },
                                        IntensityType::PowerAbsHigh, 400s) };
    writeIntensityDuration (stream, ergFile, first,
    IntensityType::PowerAbsHigh, 0); std::string expected{
    "0.000\t100\n5.000\t100\n5.000\t200\n11.667\t200\n" };
    writeIntensityDuration (stream, ergFile, second,
    IntensityType::PowerAbsHigh, 5); EXPECT_EQ (stream.str (), expected); */
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
  /*   auto intervals = getTextIntervals (returnPair.second, ergFile,
                                       IntensityType::PowerAbsHigh, 300);
    EXPECT_TRUE (intervals);
    EXPECT_EQ (intervals->front ().getIntensity (IntensityType::PowerAbsLow),
               100);
    EXPECT_EQ (intervals->front ().getDuration (), std::chrono::seconds (300));
    EXPECT_EQ (intervals->back ().getIntensity (IntensityType::PowerAbsLow),
               200);
    EXPECT_EQ (intervals->back ().getDuration (), std::chrono::seconds (400));
  */
}

TEST (MrcTests, IntervalWriteTest)
{
  using namespace textFiles;
  std::stringstream stream;
  constexpr const std::uint16_t ftp{ 200 };
  constexpr const std::uint16_t relPowerLow{ 50 };
  constexpr const std::uint16_t relPowerHigh{ 75 };
  constexpr const std::chrono::seconds shortDuration{ 300 };
  constexpr const std::chrono::seconds longDuration{ 400 };
  /*   Interval first{ *Interval::create (RelativePower{ relPowerLow, ftp },
                                       IntensityType::PowerRelHigh,
                                       shortDuration) };
    Interval second{ *Interval::create (RelativePower{ relPowerHigh, ftp },
                                        IntensityType::PowerRelHigh,
                                        longDuration) };
    writeIntensityDuration (stream, mrcFile, first,
    IntensityType::PowerRelHigh, 0); std::string expected{
    "0.000\t50\n5.000\t50\n" }; EXPECT_EQ (stream.str (), expected); expected
    += "5.000\t75\n11.667\t75\n"; writeIntensityDuration (stream, mrcFile,
    second, IntensityType::PowerRelHigh, 5); EXPECT_EQ (stream.str (),
    expected); */
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
  /*   auto intervals{ getTextIntervals (returnPair.second, mrcFile,
                                      IntensityType::PowerRelHigh, 300) };
    EXPECT_TRUE (intervals);
    EXPECT_EQ (intervals->front ().getIntensity (IntensityType::PowerAbsLow),
               150);
    EXPECT_EQ (intervals->front ().getDuration (), std::chrono::seconds (300));
    EXPECT_EQ (intervals->back ().getIntensity (IntensityType::PowerRelLow),
    75); EXPECT_EQ (intervals->back ().getDuration (), std::chrono::seconds
    (400)); */
}
TEST (MrcTests, WorkoutWriteTest)
{
  using namespace textFiles;
  std::stringstream stream;
  Workout workout{ "Workout", "Notes" };
  /*   writeWorkout (stream, mrcFile, workout);
    std::array expected{ "[COURSE HEADER]\n",     "VERSION = 2\n",
                         "UNITS = METRIC\n",      "DESCRIPTION = Notes\n",
                         "FILE NAME = Workout\n", "MINUTES PERCENT\n",
                         "[END COURSE HEADER]\n", "[COURSE DATA]\n" };
    for (const auto &check : expected)
      {
        EXPECT_TRUE (stream.str ().contains (check));
      } */
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

}; // namespace Workouts
