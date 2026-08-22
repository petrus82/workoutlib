
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
  std::string getWorkoutName () { return "TestName"; }
  std::string getWorkoutNotes () { return "TestNotes"; }

  void setWorkoutName (std::string_view name)
  {
    if (name != "TestName")
      {
        FAIL () << "Got " << name << ", not TestName";
      }
  }

  void setWorkoutNotes (std::string_view notes)
  {
    if (notes != "TestNotes")
      {
        FAIL () << "Got " << notes << ", not TestNotes";
      }
  }

  voidReturn writeFile (std::span<Interval> intervals) { return {}; }
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

  voidReturn writeIntervals (std::span<Interval> intervals)
  {
    auto iterator{ intervals.begin () };
    if (iterator == intervals.end ())
      {
        return std::unexpected ("No intervals.");
      }
    auto intensity1{ *intervals.begin ()->getIntensity ().getWatts () };
    int duration1{ static_cast<int> (
        intervals.begin ()->getDuration ().count ()) };
    auto intensity2{ *intervals.back ().getIntensity ().getWatts () };
    int duration2{ static_cast<int> (
        intervals.back ().getDuration ().count ()) };
    if (intensity1 == 1 && duration1 == 300 && intensity2 == 2
        && duration2 == 400)
      {
        return {};
      }
    return std::unexpected (std::format (
        "Test failed with intensity1: {}, duration1: {}, intensity2: {}, "
        "duration2: {}",
        intensity1, std::to_string (duration1), intensity2,
        std::to_string (duration2)));
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

class WorkoutWriteTest : public testing::Test
{
  void SetUp () override
  {
    workout.addInterval (Interval{ Intensity{ 1, IntensityUnit::Watts, 200 },
                                   std::chrono::seconds (300) });
    workout.addInterval (Interval{ Intensity{ 2, IntensityUnit::Watts, 200 },
                                   std::chrono::seconds (400) });
  }

protected:
  Workout workout{ "TestName", "TestNotes" };
};
TEST_F (WorkoutWriteTest, writeFileTest)
{
  TestFileHandler handler{ true };
  if (auto retVal{ workout.writeFile (handler) }; retVal)
    {
      SUCCEED ();
    }
  else
    {
      FAIL () << retVal.error ();
    }
}

}; // namespace Workouts
int main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}