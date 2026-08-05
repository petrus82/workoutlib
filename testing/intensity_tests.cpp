#include <gtest/gtest.h>

import std;
import intensity;
import fitmodule;
import common;

using namespace Workouts;

TEST (Intensity, PowerTests)
{
  // Power Data
  // Constructor
  Intensity intensity{ 100, IntensityUnit::Watts, 200, Level::Low };
  EXPECT_EQ (intensity.getTarget (Level::Low), 100);
  EXPECT_EQ (intensity.getTarget (Level::High), 100);
  EXPECT_EQ (intensity.getUnitStr (), "watts");
  EXPECT_EQ (intensity.getType (), IntensityUnit::Watts);
  EXPECT_EQ (intensity.getTarget (), 100);

  // setTarget
  intensity.setTarget (150, IntensityUnit::Watts, Level::High);
  EXPECT_EQ (intensity.getTarget (Level::Low), 100);
  EXPECT_EQ (intensity.getTarget (Level::High), 150);
  intensity.setTarget (IntensityPair{ 200, 250 });
  EXPECT_EQ (intensity.getTarget (Level::Low), 200);
  EXPECT_EQ (intensity.getTarget (Level::High), 250);
}

TEST (Intensity, HeartRateTests)
{
  // Heart Rate Data
  Intensity intensity{ 150, IntensityUnit::HeartRateBPM, 180, Level::Low };
  EXPECT_EQ (intensity.getTarget (Level::Low), 150);
  EXPECT_EQ (intensity.getTarget (Level::High), 150);
  EXPECT_EQ (intensity.getUnitStr (), "bpm");
  EXPECT_EQ (intensity.getType (), IntensityUnit::HeartRateBPM);
  EXPECT_EQ (intensity.getTarget (), 150);

  // setTarget
  intensity.setTarget (160, IntensityUnit::HeartRateBPM, Level::High);
  EXPECT_EQ (intensity.getTarget (Level::Low), 150);
  EXPECT_EQ (intensity.getTarget (Level::High), 160);
  intensity.setTarget (IntensityPair{ 170, 180 });
  EXPECT_EQ (intensity.getTarget (Level::Low), 170);
  EXPECT_EQ (intensity.getTarget (Level::High), 180);
}