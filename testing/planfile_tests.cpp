#include <gtest/gtest.h>

import std;
import workoutlib;
import planfiles;

namespace Workouts
{
constexpr const std::uint16_t ftp{ 200 };
constexpr const std::uint16_t relPower{ 75 };

TEST (PlanTests, WorkoutWriteTest)
{
  using namespace planFiles;
  std::stringstream stream;
  Workout workout{ "Workout", "Notes" };
  /*   writeWorkout (stream, planFile, workout);
    std::array expected{ "=HEADER=\n\n",       "NAME = Workout\n",
                         "DURATION = 0\n\n",   "PLAN_TYPE = 0\n",
                         "WORKOUT_TYPE = 0\n", "DESCRIPTION = Notes\n",
                         "=STREAM=\n\n" };
    for (const auto &check : expected)
      {
        EXPECT_PRED1 ([&] (std::string_view string)
                        { return stream.str ().contains (string); }, check);
      } */
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
  /*   auto tags{ getTags (returnPair.first, planFile.headerSeparator) };
    auto workout{ getWorkout (returnPair.first, planFile) };
    EXPECT_EQ (workout.getName (), "Workout");
    EXPECT_EQ (workout.getNotes (), "Notes Second Line"); */
}
TEST (PlanTests, IntervalWriteTest)
{
  using namespace planFiles;
  std::stringstream stream;

  constexpr const std::chrono::seconds shortDuration{ 300 };
  constexpr const std::chrono::seconds longDuration{ 400 };
  std::array expected{
    "=INTERVAL=\n\n",      "PWR_LO=150\n",
    "PWR_HI=150\n",        "MESG_DURATION_SEC>=300?EXIT\n",
    "=INTERVAL=\n\n",      "PERCENT_FTP_LO=75\n",
    "PERCENT_FTP_HI=75\n", "MESG_DURATION_SEC>=400?EXIT\n"
  };
  /*   Interval first{ *Interval::create (RelativePower{ relPower, ftp },
                                       IntensityType::PowerRelHigh,
                                       shortDuration) };
    Interval second{ *Interval::create (RelativePower{ relPower, ftp },
                                        IntensityType::PowerRelHigh,
                                        longDuration) };
    writeIntensityTime (stream, planFile, first, IntensityType::PowerAbsHigh);
    writeIntensityTime (stream, planFile, second, IntensityType::PowerRelHigh);
    for (const auto &check : expected)
      {
        EXPECT_THAT (stream.str (), HasSubstr (check));
      } */
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
  auto retVal{ getPlanIntervals (returnPair.second, ftp) };
  EXPECT_TRUE (retVal);
  const auto &intervals{ *retVal };
  /*   EXPECT_EQ (intervals.front ().getIntensity
    (IntensityType::PowerAbsHigh), 150); EXPECT_EQ (intervals.front
    ().getDuration (), std::chrono::seconds (300)); EXPECT_EQ (intervals.back
    ().getIntensity (IntensityType::PowerRelHigh), 75); EXPECT_EQ
    (intervals.back ().getDuration (), std::chrono::seconds (400)); */
}

}; // namespace Workouts