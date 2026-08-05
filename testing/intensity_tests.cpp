#include <gtest/gtest.h>

import std;
import intensity;
import fitmodule;
import common;

using namespace Workouts;
static constexpr uint16_t ftp{ 300 };
static constexpr uint8_t maxHR{ 180 };
static constexpr uint16_t powerLow{ 200 };
static constexpr uint16_t powerHigh{ 250 };
static constexpr IntensityPair powerPair{ powerLow, powerHigh };
static constexpr uint8_t hrLow{ 120 };
static constexpr uint8_t hrHigh{ 160 };
static constexpr IntensityPair hrPair{ hrLow, hrHigh };

TEST (Intensity, PowerTests)
{
  Intensity intensity{ powerLow, IntensityUnit::Watts, ftp, Level::Low };
  EXPECT_EQ (intensity.getTarget (Level::Low), powerLow);
  EXPECT_EQ (intensity.getTarget (Level::High), powerLow);
  EXPECT_EQ (intensity.getUnitStr (), "watts");
  EXPECT_EQ (intensity.getType (), IntensityUnit::Watts);
  EXPECT_EQ (intensity.getTarget (), powerLow);
}

TEST (Intensity, PowerSetTargetTests)
{
  Intensity intensity;
  intensity.setTarget (powerLow, IntensityUnit::Watts, Level::Low);
  intensity.setTarget (powerHigh, IntensityUnit::Watts, Level::High);
  EXPECT_EQ (intensity.getTarget (Level::Low), powerLow);
  EXPECT_EQ (intensity.getTarget (Level::High), powerHigh);
}
TEST (Intensity, PowerSetTargetPairTests)
{
  Intensity intensity;
  intensity.setTarget (powerPair);
  EXPECT_EQ (intensity.getTarget (Level::Low), powerLow);
  EXPECT_EQ (intensity.getTarget (Level::High), powerHigh);
}

TEST (Intensity, HeartRateTests)
{
  Intensity intensity{ hrLow, IntensityUnit::HeartRateBPM, maxHR, Level::Low };
  EXPECT_EQ (intensity.getTarget (Level::Low), hrLow);
  EXPECT_EQ (intensity.getTarget (Level::High), hrLow);
  EXPECT_EQ (intensity.getUnitStr (), "bpm");
  EXPECT_EQ (intensity.getType (), IntensityUnit::HeartRateBPM);
  EXPECT_EQ (intensity.getTarget (), hrLow);
}

TEST (Intensity, HeartRateSetTargetTests)
{
  Intensity intensity;
  intensity.setTarget (hrLow, IntensityUnit::HeartRateBPM, Level::Low);
  intensity.setTarget (hrHigh, IntensityUnit::HeartRateBPM, Level::High);
  EXPECT_EQ (intensity.getTarget (Level::Low), hrLow);
  EXPECT_EQ (intensity.getTarget (Level::High), hrHigh);
}

TEST (Intensity, HeartRateSetTargetPairTests)
{
  Intensity intensity;
  intensity.setTarget (hrPair);
  EXPECT_EQ (intensity.getTarget (Level::Low), hrLow);
  EXPECT_EQ (intensity.getTarget (Level::High), hrHigh);
}

// getWatts() tests
TEST (Intensity, WattsUnitTests)
{
  Intensity intensityWatts{ powerPair, IntensityUnit::Watts, ftp };
  EXPECT_EQ (intensityWatts.getWatts (Level::Low), powerLow);
  EXPECT_EQ (intensityWatts.getWatts (Level::High), powerHigh);
}

TEST (Intensity, Percent2WattsTests)
{
  // Create Intensity with 50% and 75% of FTP
  constexpr uint16_t ftpLow{ 50 };
  constexpr uint16_t ftpHigh{ 75 };
  constexpr uint16_t wattsLow{ 150 };  // 50% of 300
  constexpr uint16_t wattsHigh{ 225 }; // 75% of 300

  Intensity intensityPercent{ { ftpLow, ftpHigh },
                              IntensityUnit::PercentFTP,
                              ftp };
  EXPECT_EQ (intensityPercent.getWatts (Level::Low), wattsLow);
  EXPECT_EQ (intensityPercent.getWatts (Level::High), wattsHigh);
}

TEST (Intensity, PowerZoneUnitTests)
{
  // Intensity with PowerZone 4
  constexpr PWZ zone{ PWZ::P4 };
  constexpr uint16_t wattsLow{ 273 };  // Lower bound of P4 = 91% of 300
  constexpr uint16_t wattsHigh{ 315 }; // Upper bound of P4 = 105% of 300

  Intensity intensityZone{ zone, IntensityUnit::PowerZone, ftp };
  EXPECT_EQ (intensityZone.getWatts (Level::Low), wattsLow);
  EXPECT_EQ (intensityZone.getWatts (Level::High), wattsHigh);
}

// getPercentFTP() tests
TEST (Intensity, PercentFTPTests)
{
  Intensity intensityPercent{ powerPair, IntensityUnit::PercentFTP, ftp };
  EXPECT_EQ (intensityPercent.getPercentFTP (Level::Low), powerLow);
  EXPECT_EQ (intensityPercent.getPercentFTP (Level::High), powerHigh);
}
TEST (Intensity, PercentFTP2WattsTests)
{
  // Create Intensity with 50% and 75% of FTP
  constexpr uint16_t ftpLow{ 50 };
  constexpr uint16_t ftpHigh{ 75 };
  constexpr uint16_t wattsLow{ 150 };  // 50% of 300
  constexpr uint16_t wattsHigh{ 225 }; // 75% of 300

  Intensity intensityPercent{ { ftpLow, ftpHigh },
                              IntensityUnit::PercentFTP,
                              ftp };
  EXPECT_EQ (intensityPercent.getWatts (Level::Low), wattsLow);
  EXPECT_EQ (intensityPercent.getWatts (Level::High), wattsHigh);
}

TEST (Intensity, AbsoluteFromPercentFTPTests)
{
  // Create Intensity with 50% and 75% of FTP
  constexpr uint16_t ftpLow{ 50 };
  constexpr uint16_t ftpHigh{ 75 };
  constexpr uint16_t wattsLow{ 150 };  // 50% of 300
  constexpr uint16_t wattsHigh{ 225 }; // 75% of 300

  Intensity intensityPercent{ { wattsLow, wattsHigh },
                              IntensityUnit::Watts,
                              ftp };
  EXPECT_EQ (intensityPercent.getPercentFTP (Level::Low), ftpLow);
  EXPECT_EQ (intensityPercent.getPercentFTP (Level::High), ftpHigh);
}

TEST (Intensity, PercentFTPFailTest)
{
  Intensity intensity;
  auto retVal{ intensity.getPercentFTP (Level::Low) };
  EXPECT_FALSE (retVal.has_value ());
}

TEST (Intensity, PowerZone2PercentFTPTests)
{
  // Intensity with PowerZone 4
  constexpr PWZ zone{ PWZ::P4 };

  Intensity intensityZone{ zone, ftp };
  EXPECT_EQ (intensityZone.getPercentFTP (Level::Low), pwZone.Z4.first);
  EXPECT_EQ (intensityZone.getPercentFTP (Level::High), pwZone.Z4.second);
}

TEST (Intensity, PowerZoneAbsoluteTests)
{
  Intensity intensity{ powerPair, IntensityUnit::Watts, ftp };
  EXPECT_EQ (intensity.getPowerZone (Level::Low),
             PWZ::P2); // 200 watts is Z2 if FTP is 300
  EXPECT_EQ (intensity.getPowerZone (Level::High),
             PWZ::P3); // 250 watts is Z3 if FTP is 300
}

TEST (Intensity, PowerZoneFromPercentFTPTests)
{
  // Start with edge cases
  constexpr uint16_t ftpLow{ 0 };    // 0% FTP = P1
  constexpr uint16_t ftpHigh{ 250 }; // 250% FTP = P7

  Intensity intensityPercent{ { ftpLow, ftpHigh },
                              IntensityUnit::PercentFTP,
                              ftp };
  auto retVal{ intensityPercent.getPowerZone (Level::Low) };
  EXPECT_EQ (retVal, PWZ::P1);
  retVal = intensityPercent.getPowerZone (Level::High);
  EXPECT_EQ (retVal, PWZ::P7);

  // Check Zone values
  intensityPercent.setTarget (PWZ::P4, IntensityUnit::PowerZone, Level::Low);
  intensityPercent.setTarget (PWZ::P4, IntensityUnit::PowerZone, Level::High);
  EXPECT_EQ (intensityPercent.getPowerZone (Level::Low), PWZ::P4);
  EXPECT_EQ (intensityPercent.getPowerZone (Level::High), PWZ::P4);

  // Check absolute Values to Zones
  Intensity intensityAbsolute{ { powerLow, powerHigh },
                               IntensityUnit::Watts,
                               ftp };
  EXPECT_EQ (intensityAbsolute.getPowerZone (Level::Low),
             PWZ::P2); // 200 watts is Z2 if FTP is 300
  EXPECT_EQ (intensityAbsolute.getPowerZone (Level::High),
             PWZ::P3); // 250 watts is Z3 if FTP is 300
}