#include <gtest/gtest.h>

import std;
import intensity;
import fitmodule;
import common;

using namespace Workouts;

class IntensityTest : public ::testing::Test
{
protected:
  // Moved constants from original file
  static constexpr uint16_t ftp{ 300 };
  static constexpr uint8_t maxHR{ 180 };
  static constexpr uint16_t powerLow{ 200 };
  static constexpr uint16_t powerHigh{ 250 };
  static constexpr IntensityPair powerPair{ powerLow, powerHigh };
  static constexpr uint8_t hrLow{ 120 };
  static constexpr uint8_t hrHigh{ 160 };
  static constexpr IntensityPair hrPair{ hrLow, hrHigh };

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
  // Create Intensity with 50% and 75% of FTP
  constexpr uint16_t ftpLow{ 50 };
  constexpr uint16_t ftpHigh{ 75 };
  constexpr uint16_t wattsLow{ 150 };  // 50% of 300
  constexpr uint16_t wattsHigh{ 225 }; // 75% of 300

  m_intensity = std::make_unique<Intensity> (IntensityPair{ ftpLow, ftpHigh },
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getWatts (Level::Low), wattsLow);
  EXPECT_EQ (m_intensity->getWatts (Level::High), wattsHigh);
}

TEST_F (IntensityTest, PowerZoneUnitTests)
{
  // Intensity with PowerZone 4
  constexpr PWZ zone{ PWZ::P4 };
  constexpr uint16_t wattsLow{ 273 };  // Lower bound of P4 = 91% of 300
  constexpr uint16_t wattsHigh{ 315 }; // Upper bound of P4 = 105% of 300

  m_intensity
      = std::make_unique<Intensity> (zone, IntensityUnit::PowerZone, ftp);
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
  // Create Intensity with 50% and 75% of FTP
  constexpr uint16_t ftpLow{ 50 };
  constexpr uint16_t ftpHigh{ 75 };
  constexpr uint16_t wattsLow{ 150 };  // 50% of 300
  constexpr uint16_t wattsHigh{ 225 }; // 75% of 300

  m_intensity = std::make_unique<Intensity> (IntensityPair{ ftpLow, ftpHigh },
                                             IntensityUnit::PercentFTP, ftp);
  EXPECT_EQ (m_intensity->getWatts (Level::Low), wattsLow);
  EXPECT_EQ (m_intensity->getWatts (Level::High), wattsHigh);
}

TEST_F (IntensityTest, AbsoluteFromPercentFTPTests)
{
  // Create Intensity with 50% and 75% of FTP
  constexpr uint16_t ftpLow{ 50 };
  constexpr uint16_t ftpHigh{ 75 };
  constexpr uint16_t wattsLow{ 150 };  // 50% of 300
  constexpr uint16_t wattsHigh{ 225 }; // 75% of 300

  m_intensity = std::make_unique<Intensity> (
      IntensityPair{ wattsLow, wattsHigh }, IntensityUnit::Watts, ftp);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::Low), ftpLow);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::High), ftpHigh);
}

TEST_F (IntensityTest, PercentFTPFailTest)
{
  m_intensity = std::make_unique<Intensity> ();
  auto retVal{ m_intensity->getPercentFTP (Level::Low) };
  EXPECT_FALSE (retVal.has_value ());
}

TEST_F (IntensityTest, PowerZone2PercentFTPTests)
{
  // Intensity with PowerZone 4
  constexpr PWZ zone{ PWZ::P4 };

  m_intensity = std::make_unique<Intensity> (zone, ftp);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::Low), pwZone.Z4.first);
  EXPECT_EQ (m_intensity->getPercentFTP (Level::High), pwZone.Z4.second);
}

TEST_F (IntensityTest, PowerZoneAbsoluteTests)
{
  m_intensity
      = std::make_unique<Intensity> (powerPair, IntensityUnit::Watts, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low),
             PWZ::P2); // 200 watts is Z2 if FTP is 300
  EXPECT_EQ (m_intensity->getPowerZone (Level::High),
             PWZ::P3); // 250 watts is Z3 if FTP is 300
}

TEST_F (IntensityTest, PowerZoneFromPercentFTPTests)
{
  // Start with edge cases
  constexpr uint16_t ftpLow{ 0 };    // 0% FTP = P1
  constexpr uint16_t ftpHigh{ 250 }; // 250% FTP = P7

  m_intensity = std::make_unique<Intensity> (IntensityPair{ ftpLow, ftpHigh },
                                             IntensityUnit::PercentFTP, ftp);
  auto retVal{ m_intensity->getPowerZone (Level::Low) };
  EXPECT_EQ (retVal, PWZ::P1);
  retVal = m_intensity->getPowerZone (Level::High);
  EXPECT_EQ (retVal, PWZ::P7);

  // Check Zone values
  m_intensity->setTarget (PWZ::P4, IntensityUnit::PowerZone, Level::Low);
  m_intensity->setTarget (PWZ::P4, IntensityUnit::PowerZone, Level::High);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low), PWZ::P4);
  EXPECT_EQ (m_intensity->getPowerZone (Level::High), PWZ::P4);

  // Check absolute Values to Zones
  m_intensity = std::make_unique<Intensity> (
      IntensityPair{ powerLow, powerHigh }, IntensityUnit::Watts, ftp);
  EXPECT_EQ (m_intensity->getPowerZone (Level::Low),
             PWZ::P2); // 200 watts is Z2 if FTP is 300
  EXPECT_EQ (m_intensity->getPowerZone (Level::High),
             PWZ::P3); // 250 watts is Z3 if FTP is 300
}