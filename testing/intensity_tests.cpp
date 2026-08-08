#include <gtest/gtest.h>

import std;
import intensity;
import fitmodule;
import common;

using namespace Workouts;

class IntensityTest : public ::testing::Test
{
protected:
  static constexpr const uint16_t ftp{ 300 };
  static constexpr const uint8_t maxHR{ 180 };

  // Power
  static constexpr const uint16_t powerLow{ 150 };
  static constexpr const uint16_t powerHigh{ 225 };
  static constexpr const uint16_t zero{ 0 };
  static constexpr const uint16_t relPowLow{ 50 };
  static constexpr const uint16_t relPowHigh{ 75 };
  static constexpr const uint16_t highRelPower{ 250 }; // 250% FTP = P7
  static constexpr const PWZ powerZone{ PWZ::P4 };
  static constexpr const IntensityPair powerPair{ powerLow, powerHigh };

  // Heart Rate
  static constexpr const uint8_t hrLow{ 120 };
  static constexpr const uint8_t hrHigh{ 160 };
  static constexpr const IntensityPair hrPair{ hrLow, hrHigh };

  std::unique_ptr<Intensity> m_intensity;

  void SetUp () override
  {
    // Setup code if needed
  }
};

TEST_F (IntensityTest, PowerTests)
{
  m_intensity = std::make_unique<Intensity> (powerLow, IntensityUnit::Watts,
                                             ftp, Level::Low);
  EXPECT_EQ (m_intensity->getTarget (Level::Low), powerLow);
  EXPECT_EQ (m_intensity->getTarget (Level::High), powerLow);
  EXPECT_EQ (m_intensity->getUnitStr (), "watts");
  EXPECT_EQ (m_intensity->getType (), IntensityUnit::Watts);
  EXPECT_EQ (m_intensity->getTarget (), powerLow);
}

TEST_F (IntensityTest, PowerSetTargetTests)
{
  m_intensity = std::make_unique<Intensity> ();
  m_intensity->setTarget (powerLow, IntensityUnit::Watts, Level::Low);
  m_intensity->setTarget (powerHigh, IntensityUnit::Watts, Level::High);
  EXPECT_EQ (m_intensity->getTarget (Level::Low), powerLow);
  EXPECT_EQ (m_intensity->getTarget (Level::High), powerHigh);
}

TEST_F (IntensityTest, PowerSetTargetPairTests)
{
  m_intensity = std::make_unique<Intensity> ();
  m_intensity->setTarget (powerPair);
  EXPECT_EQ (m_intensity->getTarget (Level::Low), powerLow);
  EXPECT_EQ (m_intensity->getTarget (Level::High), powerHigh);
}

TEST_F (IntensityTest, HeartRateTests)
{
  m_intensity = std::make_unique<Intensity> (
      hrLow, IntensityUnit::HeartRateBPM, maxHR, Level::Low);
  EXPECT_EQ (m_intensity->getTarget (Level::Low), hrLow);
  EXPECT_EQ (m_intensity->getTarget (Level::High), hrLow);
  EXPECT_EQ (m_intensity->getUnitStr (), "bpm");
  EXPECT_EQ (m_intensity->getType (), IntensityUnit::HeartRateBPM);
  EXPECT_EQ (m_intensity->getTarget (), hrLow);
}

TEST_F (IntensityTest, HeartRateSetTargetTests)
{
  m_intensity = std::make_unique<Intensity> ();
  m_intensity->setTarget (hrLow, IntensityUnit::HeartRateBPM, Level::Low);
  m_intensity->setTarget (hrHigh, IntensityUnit::HeartRateBPM, Level::High);
  EXPECT_EQ (m_intensity->getTarget (Level::Low), hrLow);
  EXPECT_EQ (m_intensity->getTarget (Level::High), hrHigh);
}

TEST_F (IntensityTest, HeartRateSetTargetPairTests)
{
  m_intensity = std::make_unique<Intensity> ();
  m_intensity->setTarget (hrPair);
  EXPECT_EQ (m_intensity->getTarget (Level::Low), hrLow);
  EXPECT_EQ (m_intensity->getTarget (Level::High), hrHigh);
}

// getWatts() tests
TEST_F (IntensityTest, WattsUnitTests)
{
  m_intensity
      = std::make_unique<Intensity> (powerPair, IntensityUnit::Watts, ftp);
  EXPECT_EQ (m_intensity->getWatts (Level::Low), powerLow);
  EXPECT_EQ (m_intensity->getWatts (Level::High), powerHigh);
}

TEST_F (IntensityTest, Percent2WattsTests)
{
  m_intensity = std::make_unique<Intensity> (
      IntensityPair{ relPowLow, relPowHigh }, IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getWatts (Level::Low), powerLow);
  EXPECT_EQ (m_intensity->getWatts (Level::High), powerHigh);
}

TEST_F (IntensityTest, PowerZoneUnitTests)
{
  // Intensity with PowerZone 4
  constexpr PWZ powerZone{ PWZ::P4 };
  constexpr uint16_t wattsLow{ 273 };  // Lower bound of P4 = 91% of 300
  constexpr uint16_t wattsHigh{ 315 }; // Upper bound of P4 = 105% of 300

  m_intensity
      = std::make_unique<Intensity> (powerZone, IntensityUnit::PowerZone, ftp);
  EXPECT_EQ (m_intensity->getWatts (Level::Low), wattsLow);
  EXPECT_EQ (m_intensity->getWatts (Level::High), wattsHigh);
}

// getPercentFTP() tests
TEST_F (IntensityTest, PercentFTPTests)
{
  m_intensity = std::make_unique<Intensity> (powerPair,
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::Low), powerLow);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::High), powerHigh);
}

TEST_F (IntensityTest, PercentFTP2WattsTests)
{
  m_intensity = std::make_unique<Intensity> (
      IntensityPair{ relPowLow, relPowHigh }, IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getWatts (Level::Low), powerLow);
  EXPECT_EQ (m_intensity->getWatts (Level::High), powerHigh);
}

TEST_F (IntensityTest, AbsoluteFromPercentFTPTests)
{
  m_intensity = std::make_unique<Intensity> (
      IntensityPair{ powerLow, powerHigh }, IntensityUnit::Watts, ftp);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::Low), relPowLow);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::High), relPowHigh);
}

TEST_F (IntensityTest, PercentFTPFailTest)
{
  m_intensity = std::make_unique<Intensity> ();
  auto retVal{ m_intensity->getPercentFTP (Level::Low) };
  EXPECT_FALSE (retVal.has_value ());
}

TEST_F (IntensityTest, PowerZone2PercentFTPTests)
{
  m_intensity = std::make_unique<Intensity> (powerZone, ftp);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::Low), pwZone.Z4.first);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::High), pwZone.Z4.second);
}

TEST_F (IntensityTest, PowerZoneAbsoluteTests)
{
  m_intensity
      = std::make_unique<Intensity> (powerPair, IntensityUnit::Watts, ftp);
  EXPECT_TRUE (m_intensity->getPowerZone (Level::Low).has_value ());
  EXPECT_EQ (*m_intensity->getPowerZone (Level::Low),
             PWZ::P1); // 150 watts is Z1 if FTP is 300
  EXPECT_EQ (*m_intensity->getPowerZone (Level::High),
             PWZ::P2); // 225 watts is Z2 if FTP is 300
}

TEST_F (IntensityTest, PowerZoneFromPercentFTPTests)
{
  m_intensity = std::make_unique<Intensity> (
      IntensityPair{ zero, highRelPower }, IntensityUnit::PercentFTP, ftp);
  auto retVal{ m_intensity->getPowerZone (Level::Low) };
  EXPECT_EQ (*retVal, PWZ::P1);
  retVal = m_intensity->getPowerZone (Level::High);
  EXPECT_EQ (*retVal, PWZ::P7);

  // Check Zone values
  m_intensity->setTarget (PWZ::P4, IntensityUnit::PowerZone, Level::Low);
  m_intensity->setTarget (PWZ::P4, IntensityUnit::PowerZone, Level::High);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low), PWZ::P4);
  EXPECT_EQ (m_intensity->getPowerZone (Level::High), PWZ::P4);

  // Check absolute Values to Zones
  m_intensity = std::make_unique<Intensity> (
      IntensityPair{ powerLow, powerHigh }, IntensityUnit::Watts, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low),
             PWZ::P1); // 150 watts is Z1 if FTP is 300
  EXPECT_EQ (m_intensity->getPowerZone (Level::High),
             PWZ::P2); // 225 watts is Z2 if FTP is 300
}

TEST_F (IntensityTest, PowerZoneBoundaryTests)
{
  // P1
  m_intensity = std::make_unique<Intensity> (IntensityPair{ 0, 54 },
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low), PWZ::P1);
  EXPECT_EQ (m_intensity->getPowerZone (Level::High), PWZ::P1);

  // P2
  m_intensity = std::make_unique<Intensity> (IntensityPair{ 55, 75 },
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low), PWZ::P2);
  EXPECT_EQ (m_intensity->getPowerZone (Level::High), PWZ::P2);

  // P3
  m_intensity = std::make_unique<Intensity> (IntensityPair{ 76, 90 },
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low), PWZ::P3);
  EXPECT_EQ (m_intensity->getPowerZone (Level::High), PWZ::P3);

  // P4
  m_intensity = std::make_unique<Intensity> (IntensityPair{ 91, 105 },
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low), PWZ::P4);
  EXPECT_EQ (m_intensity->getPowerZone (Level::High), PWZ::P4);

  // P5
  m_intensity = std::make_unique<Intensity> (IntensityPair{ 106, 120 },
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low), PWZ::P5);
  EXPECT_EQ (m_intensity->getPowerZone (Level::High), PWZ::P5);

  // P6
  m_intensity = std::make_unique<Intensity> (IntensityPair{ 121, 150 },
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low), PWZ::P6);
  EXPECT_EQ (m_intensity->getPowerZone (Level::High), PWZ::P6);

  // P7
  m_intensity = std::make_unique<Intensity> (IntensityPair{ 151, 255 },
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low), PWZ::P7);
  EXPECT_EQ (m_intensity->getPowerZone (Level::High), PWZ::P7);
}