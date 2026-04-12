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
using namespace Workouts;

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
    std::list<Interval> intervals{ Interval{ 200, 300s, intervalType, ftp },
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
  std::array expected{ "=HEADER=\n\n",   "NAME=Workout\n",
                       "DURATION=0\n\n", "DESCRIPTION=Notes\n",
                       "PLAN_TYPE=0\n",  "WORKOUT_TYPE=0\n",
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
      EXPECT_TRUE (stream.str ().contains (check));
    }
}

TEST (PlanTests, IntervalReadTest)
{
  using namespace planFiles;
  std::string_view testfile{ "=HEADER=\n\n"
                             "NAME=Workout\n\n"
                             "DURATION=0\n"
                             "PLAN_TYPE=0\n"
                             "WORKOUT_TYPE=0\n"
                             "DESCRIPTION=Notes\n\n"
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
  auto test{ processContent (testfile, planFile) };
  auto tags{ getTags (test.second, planFile.intervalSeparator) };
  auto returnPair{ splitPlanContent (testfile) };
  auto retVal{ getPlanIntervals (returnPair.second, 300) };
  EXPECT_TRUE (retVal);
  const auto &intervals{ retVal.value () };
  EXPECT_EQ (intervals.front ().getIntensity (IntensityType::PowerAbsHigh),
             150);
  EXPECT_EQ (intervals.front ().getDuration (), std::chrono::seconds (300));
  EXPECT_EQ (intervals.back ().getIntensity (IntensityType::PowerRelHigh), 75);
  EXPECT_EQ (intervals.back ().getDuration (), std::chrono::seconds (400));
}

TEST (FitTests, WorkoutReadTest)
{
  using namespace Workouts;
  std::ifstream file ("WorkoutCustomTargetValues.fit",
                      std::ios::in | std::ios::binary);
}
int main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}