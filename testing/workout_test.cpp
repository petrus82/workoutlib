#include <gtest/gtest.h>

import std;
import workoutlib;
import fitmodule;
void
convertCSV (std::string_view file)
{
  const std::string FitCSVTool{ "/usr/lib/garminfit/FitCSVTool.jar" };
  std::string fitCSVCommand{
    std::string ("java -jar ").append (FitCSVTool).append (" ")
  };
  std::system (fitCSVCommand.append (file).c_str ());
}

std::filesystem::path
unreadableFile ()
{
  auto path = std::filesystem::temp_directory_path () / "unreadable.txt";
  std::ofstream file (path);
  file.close ();
  std::filesystem::permissions (path, std::filesystem::perms::owner_write);
  return path;
}

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
TEST (ErgTests, WorkoutWriteTest)
{
  using namespace Workouts;
  using namespace ErgFile;
  std::stringstream stream;
  Workout workout{ "Workout", "Notes" };
  workout.setFtp (300);
  writeWorkout (stream, workout);
  std::string expected{
    "[COURSE HEADER]\nVERSION = 2\nUNITS = GERMAN\n"
    "DESCRIPTION = Notes\nFILE NAME = Workout\nFTP = 300\n"
    "MINUTES WATTS\n[END COURSE HEADER]\n[COURSE DATA]\n"
  };
  EXPECT_EQ (stream.str (), expected);
}
TEST (ErgTests, WorkoutReadTest)
{
  std::stringstream stream;
  stream << "[COURSE HEADER]\nVERSION = 2\nUNITS = GERMAN\n"
         << "DESCRIPTION = Notes\nFILE NAME = Workout\nFTP = 300\n"
         << "MINUTES WATTS\n[END COURSE HEADER]\n[COURSE DATA]\n";
  auto retVal{ Workouts::ErgFile::readWorkout (stream) };
  EXPECT_TRUE (retVal);
  auto workout{ retVal.value () };
  EXPECT_EQ (workout.getName (), "Workout");
  EXPECT_EQ (workout.getNotes (), "Notes");
  EXPECT_EQ (workout.getFtp (), 300);
}
TEST (ErgTests, IntervalWriteTest)
{
  using namespace Workouts;
  using namespace ErgFile;
  std::stringstream stream;
  Interval first{ 100, WorkoutType::AbsoluteWatt, std::chrono::seconds (300) };
  Interval second{ 200, WorkoutType::AbsoluteWatt,
                   std::chrono::seconds (400) };
  writeInterval (stream, first, WorkoutType::AbsoluteWatt);
  std::string expected{ "0.00\t100\n0.00\t5.00\n" };
  EXPECT_EQ (stream.str (), expected);
  expected += "5.00\t200\n5.00\t11.67\n";
  writeInterval (stream, second, WorkoutType::AbsoluteWatt);
  EXPECT_EQ (stream.str (), expected);
}
TEST (ErgTests, IntervalReadTest)
{
  using namespace Workouts;
  using namespace ErgFile;
  std::stringstream stream;
  stream << "[COURSE HEADER]\nVERSION = 2\nUNITS = GERMAN\n"
         << "DESCRIPTION = Notes\nFILE NAME = Workout\nFTP = 300\n"
         << "MINUTES WATTS\n[END COURSE HEADER]\n[COURSE DATA]\n";
  stream << "0.00\t100\n0.00\t5.00\n5.00\t200\n5.00\t11.67\n";
  auto retVal{ readIntervals (stream) };
  EXPECT_TRUE (retVal);
  auto intervals{ retVal.value () };
  EXPECT_EQ (intervals.front ().getIntensity (), 100);
  EXPECT_EQ (intervals.front ().getDuration (), std::chrono::seconds (300));
  EXPECT_EQ (intervals.back ().getIntensity (), 200);
  EXPECT_EQ (intervals.back ().getDuration (), std::chrono::seconds (400));
}

TEST (MrcTests, IntervalWriteTest)
{
  using namespace Workouts;
  using namespace MrcFile;
  std::stringstream stream;
  Interval first{ 100, WorkoutType::AbsoluteWatt, std::chrono::seconds (300) };
  writeInterval (stream, first, WorkoutType::PercentFTP, 200);
  std::string expected{ "0.00\t50\n5.00\t50\n" };
  EXPECT_EQ (stream.str (), expected);

  Interval second{ 75, WorkoutType::PercentFTP, std::chrono::seconds (400) };
  expected += "5.00\t75\n11.67\t75\n";
  writeInterval (stream, second, WorkoutType::PercentFTP, 200);
  EXPECT_EQ (stream.str (), expected);
}
TEST (MrcTests, IntervalReadTest)
{
  using namespace Workouts;
  using namespace MrcFile;
  std::stringstream stream;
  stream << "[COURSE HEADER]\nVERSION = 2\nUNITS = GERMAN\n"
         << "DESCRIPTION = Notes\nFILE NAME = Workout\n"
         << "MINUTES PERCENT\n[END COURSE HEADER]\n[COURSE DATA]\n";
  stream << "0.00\t50\n5.00\t50\n5.00\t75\n11.67\t75\n";
  auto retVal{ readIntervals (stream) };
  EXPECT_TRUE (retVal);
  auto intervals{ retVal.value () };
  EXPECT_EQ (intervals.front ().getIntensity (200, WorkoutType::AbsoluteWatt),
             100);
  EXPECT_EQ (intervals.front ().getDuration (), std::chrono::seconds (300));
  EXPECT_EQ (intervals.back ().getIntensity (200, WorkoutType::PercentFTP),
             75);
  EXPECT_EQ (intervals.back ().getDuration (), std::chrono::seconds (400));
}
TEST (MrcTests, WorkoutWriteTest)
{
  using namespace Workouts;
  using namespace MrcFile;
  std::stringstream stream;
  Workout workout{ "Workout", "Notes" };
  writeWorkout (stream, workout);
  std::string expected{
    "[COURSE HEADER]\nVERSION = 2\nUNITS = GERMAN\n"
    "DESCRIPTION = Notes\nFILE NAME = Workout\n"
    "MINUTES PERCENT\n[END COURSE HEADER]\n[COURSE DATA]\n"
  };
  EXPECT_EQ (stream.str (), expected);
}
TEST (MrcTests, WorkoutReadTest)
{
  using namespace Workouts;
  using namespace MrcFile;
  std::stringstream stream;
  stream << "[COURSE HEADER]\nVERSION = 2\nUNITS = GERMAN\n"
         << "DESCRIPTION = Notes\nFILE NAME = Workout\n"
         << "MINUTES PERCENT\n[END COURSE HEADER]\n[COURSE DATA]\n";
  auto retVal{ readWorkout (stream) };
  EXPECT_TRUE (retVal);
  auto workout{ retVal.value () };
  EXPECT_EQ (workout.getName (), "Workout");
  EXPECT_EQ (workout.getNotes (), "Notes");
}
int
main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}