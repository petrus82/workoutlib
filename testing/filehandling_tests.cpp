#include <gtest/gtest.h>

import std;
import workoutlib;
import fitmodule;
import fitfiles;
import filehandling;

std::filesystem::path unreadableFile ()
{
  auto path = std::filesystem::temp_directory_path () / "unreadable.txt";
  std::ofstream file (path);
  file.close ();
  std::filesystem::permissions (path, std::filesystem::perms::owner_write);
  return path;
}

namespace Workouts
{

class FileWriteTests : public testing::Test
{
public:
  void SetUp () override
  {
    constexpr const uint16_t ftp{ 300 };
    using namespace std::chrono_literals;
    workout.createInterval (
        Interval{ Intensity{ 120, IntensityUnit::PercentFTP, ftp }, 5min });
    workout.createInterval (
        Interval{ Intensity{ 65, IntensityUnit::PercentFTP, ftp }, 300s });
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
  EXPECT_TRUE (fitFiles::isValidFit (testfile, decoder));
  EXPECT_FALSE (fitFiles::isValidFit (unreadableFile (), decoder));
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
}; // namespace Workouts