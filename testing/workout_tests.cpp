
#include <gtest/gtest.h>

import std;
import workoutlib;
import fitmodule;
import common;
import interval;

void convertCSV (std::string_view file)
{
  const std::string FitCSVTool{ "/usr/lib/garminfit/FitCSVTool.jar" };
  std::string fitCSVCommand{
    std::string ("java -jar ").append (FitCSVTool).append (" ")
  };
  std::system (fitCSVCommand.append (file).c_str ());
}

using namespace std::string_literals;
namespace Workouts
{
class TestFileHandler
{
public:
  TestFileHandler (bool fileCheckPass) : m_fileCheckPass (fileCheckPass) {}

  voidReturn checkFile ()
  {
    if (m_fileCheckPass)
      {
        return {};
      }
    return std::unexpected ("Error");
  }
  stringReturn getWorkoutName () { return "TestName"; }
  stringReturn getWorkoutNotes () { return "TestNotes"; }
  auto getIntervals ()
  {
    Intervals intervals;
    intervals.emplace_back (
        Interval{ Intensity{ 1, IntensityUnit::Watts, 200 },
                  std::chrono::seconds (300) });
    intervals.emplace_back (
        Interval{ Intensity{ 2, IntensityUnit::Watts, 200 },
                  std::chrono::seconds (400) });
    return intervals;
  }

  bool m_fileCheckPass{ true };
};

TEST (WorkoutTests, readFileCheckTest)
{
  auto retVal{ readFile (TestFileHandler{ false }) };
  EXPECT_FALSE (retVal);
  EXPECT_EQ (retVal.error (), std::string ("Error"));
  EXPECT_TRUE (readFile (TestFileHandler{ true }));
}
TEST (WorkoutTests, readFileStringsTest)
{
  auto retVal{ readFile (TestFileHandler{ true }) };
  EXPECT_EQ (retVal->getName (), "TestName");
  EXPECT_EQ (retVal->getNotes (), "TestNotes");
}
TEST (WorkoutTests, readFileIntervalTest)
{
  auto retVal{ readFile (TestFileHandler{ true }) };
  auto intervalIt{ retVal->begin () };
  EXPECT_EQ (intervalIt->getDuration (), std::chrono::seconds (300));
  ++intervalIt;
  EXPECT_EQ (intervalIt->getDuration (), std::chrono::seconds (400));
  ++intervalIt;
  EXPECT_EQ (intervalIt, retVal->end ());
}

}; // namespace Workouts
int main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}