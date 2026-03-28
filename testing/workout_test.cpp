#include <gtest/gtest.h>

import std;
import workoutlib;
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

TEST (WorkoutTests, UnreadableTest)
{
  EXPECT_FALSE (Workouts::getFileStream (unreadableFile ()));
}

int
main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}