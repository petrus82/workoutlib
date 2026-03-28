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

int
main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}