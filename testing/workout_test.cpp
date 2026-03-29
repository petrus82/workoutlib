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
TEST (ErgTests, WorkoutTest)
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
TEST (ErgTests, IntervalTest)
{
  using namespace Workouts;
  using namespace ErgFile;
  std::stringstream stream;
  Interval first{ 100, std::chrono::seconds (300) };
  Interval second{ 200, std::chrono::seconds (400) };
  writeInterval (stream, first, WorkoutType::AbsoluteWatt);
  std::string expected{ "0.00\t100\n0.00\t5.00\n" };
  EXPECT_EQ (stream.str (), expected);
  expected += "5.00\t200\n5.00\t11.67\n";
  writeInterval (stream, second, WorkoutType::AbsoluteWatt);
  EXPECT_EQ (stream.str (), expected);
}

int
main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}