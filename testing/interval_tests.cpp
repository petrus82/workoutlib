#include <gtest/gtest.h>

import std;
import interval;
import intensity;
import common;
import fitmodule;

using namespace Workouts;

class IntervalTest : public ::testing::Test
{
protected:
  static constexpr const uint16_t ftp{ 300 };
  static constexpr const uint16_t powerLow{ 200 };
  static constexpr const uint16_t powerLow2{ 220 };
  static constexpr const uint16_t powerHigh{ 250 };
  static constexpr const uint16_t powerHigh2{ 400 };
  static constexpr const std::chrono::seconds duration{ 300 };
  static constexpr const std::chrono::seconds duration2{ 400 };
  std::unique_ptr<Interval> m_interval = std::make_unique<Interval> (
      Intensity{ IntensityPair{ powerLow, powerHigh }, IntensityUnit::Watts,
                 ftp },
      duration);
};

TEST_F (IntervalTest, CopyCstrTest)
{
  Interval intervalCopy{ *m_interval };
  EXPECT_EQ (m_interval->getIntensity ().getWatts (), powerLow);
  EXPECT_EQ (m_interval->getIntensity ().getWatts (),
             intervalCopy.getIntensity ().getWatts ());
  EXPECT_EQ (m_interval->getDuration (), intervalCopy.getDuration ());
  EXPECT_EQ (m_interval->getRepeats (), intervalCopy.getRepeats ());
}

TEST_F (IntervalTest, CopyOpTest)
{
  Interval intervalCopy;
  intervalCopy = *m_interval;
  EXPECT_EQ (m_interval->getIntensity ().getWatts (), powerLow);
  EXPECT_EQ (m_interval->getIntensity ().getWatts (),
             intervalCopy.getIntensity ().getWatts ());
  EXPECT_EQ (m_interval->getDuration (), intervalCopy.getDuration ());
  EXPECT_EQ (m_interval->getRepeats (), intervalCopy.getRepeats ());
}

TEST_F (IntervalTest, IteratorSeqTest)
{
  m_interval->addSubInterval (Interval{
      Intensity{ powerLow2, IntensityUnit::Watts, ftp }, duration2 });
  for (const auto &it : *m_interval)
    {
      std::println ("Duration: {}", it.getDuration ());
    }
  auto it{ m_interval->begin () };
  EXPECT_EQ (*it->getIntensity ().getWatts (), powerLow);
  EXPECT_EQ (it->getDuration (), duration);
  ++it;
  EXPECT_EQ (*it->getIntensity ().getWatts (), powerLow2);
  EXPECT_EQ (it->getDuration (), duration2);
  it++;
  EXPECT_THROW (it->getDuration (), std::out_of_range);
}

TEST_F (IntervalTest, AddSubIntervalTest)
{
  m_interval->addSubInterval (
      Interval{ Intensity{ IntensityPair{ powerLow2, powerHigh2 },
                           IntensityUnit::Watts, ftp },
                duration2 });
}