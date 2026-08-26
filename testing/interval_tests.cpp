#include <climits>
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
  auto it{ m_interval->begin () };
  EXPECT_EQ (*it->getIntensity ().getWatts (), powerLow);
  EXPECT_EQ (it->getDuration (), duration);
  ++it;
  EXPECT_EQ (*it->getIntensity ().getWatts (), powerLow2);
  EXPECT_EQ (it->getDuration (), duration2);
  it++;
}

TEST_F (IntervalTest, IteratorCountTest)
{
  m_interval->addSubInterval (Interval{
      Intensity{ powerLow2, IntensityUnit::Watts, ftp }, duration2 });
  constexpr const int repeats{ 2 };
  constexpr const int intervals{ 2 };

  m_interval->addRepeat (Repeat{ .begin = -1, .end = 0, .times = 2 });
  int repeated{};
  for (const auto &it : *m_interval)
    {
      ++repeated;
      if (repeated > 4)
        {
          FAIL () << "Endless loop!";
        }
    }
  EXPECT_EQ (repeated, repeats * intervals);
}

TEST_F (IntervalTest, IteratorThrowTest)
{
  m_interval->addSubInterval (Interval{
      Intensity{ powerLow2, IntensityUnit::Watts, ftp }, duration2 });
  constexpr const int repeats{ 2 };
  constexpr const int intervals{ 2 };
  m_interval->addRepeat (Repeat{ .begin = -1, .end = 0, .times = 2 });
  auto it{ m_interval->begin () };
  int index{ 1 };
  for (; index <= (repeats); ++index)
    {
      // it points to parent
      EXPECT_NO_THROW (it->getDuration ());
      ++it;
      // now it points to subInterval
      EXPECT_NO_THROW (it->getDuration ());
      // increment to next sequence
      ++it;
    }

  // Now we are out of range
  EXPECT_EQ (index, repeats + 1);
  EXPECT_THROW (it->getDuration (), std::out_of_range);
}

TEST_F (IntervalTest, IteratorLevel2Test)
{
  m_interval->addSubInterval (Interval{
      Intensity{ 1, IntensityUnit::Watts, ftp }, std::chrono::seconds (1) });
  m_interval->addSubInterval (Interval{
      Intensity{ 2, IntensityUnit::Watts, ftp }, std::chrono::seconds (2) });
  // Sequence has to be (-1, 0)
  m_interval->addRepeat (Repeat{ .begin = -1, .end = 0, .times = 1 });
  // Sequence (1, -1, 0, 1)
  m_interval->addRepeat (Repeat{ .begin = 1, .end = 1, .times = 2 });
  EXPECT_EQ (m_interval->count (), 6);
  auto intervalIt{ m_interval->begin () };

  // First repeat, 1 x from parent to subInterval 1
  // parent Interval
  EXPECT_NO_THROW (intervalIt->getIntensity ());
  EXPECT_EQ (*intervalIt->getIntensity ().getWatts (), powerLow);
  EXPECT_EQ (intervalIt->getDuration ().count (), duration.count ());

  // subInterval 1
  ++intervalIt;
  EXPECT_NO_THROW (intervalIt->getIntensity ());
  EXPECT_EQ (*intervalIt->getIntensity ().getWatts (), 1);
  EXPECT_EQ (intervalIt->getDuration ().count (), 1);

  // Second repeat: first repeat sequence and then parent to subInterval 2
  // subInterval 2
  ++intervalIt;
  EXPECT_NO_THROW (intervalIt->getIntensity ());
  EXPECT_EQ (*intervalIt->getIntensity ().getWatts (), 2);
  EXPECT_EQ (intervalIt->getDuration ().count (), 2);

  // parent
  ++intervalIt;
  EXPECT_NO_THROW (intervalIt->getIntensity ());
  EXPECT_EQ (*intervalIt->getIntensity ().getWatts (), powerLow);
  EXPECT_EQ (intervalIt->getDuration ().count (), duration.count ());

  // subInterval 1
  ++intervalIt;
  EXPECT_NO_THROW (intervalIt->getIntensity ());
  EXPECT_EQ (*intervalIt->getIntensity ().getWatts (), 1);
  EXPECT_EQ (intervalIt->getDuration ().count (), 1);

  // subInterval 2
  ++intervalIt;
  EXPECT_NO_THROW (intervalIt->getIntensity ());
  EXPECT_EQ (*intervalIt->getIntensity ().getWatts (), 2);
  EXPECT_EQ (intervalIt->getDuration ().count (), 2);

  // Sentinel
  ++intervalIt;
  EXPECT_EQ (intervalIt, m_interval->end ());
  EXPECT_THROW (intervalIt->getIntensity (), std::out_of_range);
}

TEST_F (IntervalTest, addRepeatThrowTests)
{
  EXPECT_THROW (
      m_interval->addRepeat (Repeat{ .begin = -1, .end = 0, .times = 1 }),
      std::runtime_error);
  EXPECT_THROW (
      m_interval->addRepeat (Repeat{ .begin = 0, .end = 0, .times = 1 }),
      std::runtime_error);
  EXPECT_THROW (
      m_interval->addRepeat (Repeat{ .begin = -2, .end = -1, .times = 1 }),
      std::runtime_error);
  EXPECT_THROW (
      m_interval->addRepeat (Repeat{ .begin = 1, .end = 0, .times = 1 }),
      std::runtime_error);
  m_interval->addSubInterval (Interval{
      Intensity{ 1, IntensityUnit::Watts, ftp }, std::chrono::seconds (1) });
  EXPECT_THROW (
      m_interval->addRepeat (Repeat{ .begin = -1, .end = 1, .times = 1 }),
      std::runtime_error);
  EXPECT_THROW (
      m_interval->addRepeat (Repeat{ .begin = -1, .end = 0, .times = 0 }),
      std::runtime_error);
  EXPECT_THROW (
      m_interval->addRepeat (Repeat{ .begin = 0, .end = -2, .times = 1 }),
      std::runtime_error);
}

TEST_F (IntervalTest, removeRepeatTests)
{
  m_interval->addSubInterval (Interval{
      Intensity{ 1, IntensityUnit::Watts, ftp }, std::chrono::seconds (1) });
  m_interval->addRepeat (Repeat{ .begin = -1, .end = 0, .times = 1 });
  EXPECT_THROW (m_interval->removeRepeat (-1), std::runtime_error);
  EXPECT_THROW (m_interval->removeRepeat (1), std::runtime_error);
  EXPECT_NO_THROW (m_interval->removeRepeat (0));
  EXPECT_THROW (m_interval->removeRepeat (0), std::runtime_error);
}

TEST_F (IntervalTest, NoRepeatTest)
{
  EXPECT_EQ (m_interval->count (), 1);
  EXPECT_EQ (*m_interval->begin ()->getIntensity ().getWatts (), powerLow);
  EXPECT_EQ (m_interval->begin ()->getDuration ().count (), duration.count ());
  m_interval->addSubInterval (Interval{
      Intensity{ 1, IntensityUnit::Watts, ftp }, std::chrono::seconds (1) });
  EXPECT_EQ (m_interval->count (), 2);
  auto subIntervalIt{ m_interval->begin () };
  ++subIntervalIt;
  EXPECT_EQ (*subIntervalIt->getIntensity ().getWatts (), 1);
  EXPECT_EQ (subIntervalIt->getDuration ().count (), 1);
}

TEST_F (IntervalTest, RemoveSubIntervalTest)
{
  m_interval->addSubInterval (
      Interval{ Intensity{ 1, IntensityUnit::Watts, ftp }, duration });
  m_interval->addSubInterval (
      Interval{ Intensity{ 2, IntensityUnit::Watts, ftp }, duration });
  m_interval->addSubInterval (
      Interval{ Intensity{ 3, IntensityUnit::Watts, ftp }, duration });
  m_interval->addSubInterval (
      Interval{ Intensity{ 4, IntensityUnit::Watts, ftp }, duration });
  EXPECT_EQ (m_interval->count (), 5);
  EXPECT_EQ (*m_interval->subIntervalAt (0).getIntensity ().getWatts (), 1);
  EXPECT_EQ (*m_interval->subIntervalAt (1).getIntensity ().getWatts (), 2);
  EXPECT_EQ (*m_interval->subIntervalAt (2).getIntensity ().getWatts (), 3);
  EXPECT_EQ (*m_interval->subIntervalAt (3).getIntensity ().getWatts (), 4);

  // Remove first
  if (auto retVal{ m_interval->removeSubInterval (0) }; !retVal)
    {
      FAIL () << retVal.error ();
    }
  EXPECT_EQ (*m_interval->subIntervalAt (0).getIntensity ().getWatts (), 2);
  EXPECT_EQ (*m_interval->subIntervalAt (1).getIntensity ().getWatts (), 3);
  EXPECT_EQ (*m_interval->subIntervalAt (2).getIntensity ().getWatts (), 4);

  // Remove middle
  if (auto retVal{ m_interval->removeSubInterval (1) }; !retVal)
    {
      FAIL () << retVal.error ();
    }
  EXPECT_EQ (*m_interval->subIntervalAt (0).getIntensity ().getWatts (), 2);
  EXPECT_EQ (*m_interval->subIntervalAt (1).getIntensity ().getWatts (), 4);

  // Remove last
  if (auto retVal{ m_interval->removeSubInterval (1) }; !retVal)
    {
      FAIL () << retVal.error ();
    }
  EXPECT_EQ (*m_interval->subIntervalAt (0).getIntensity ().getWatts (), 2);

  // Remove remaining
  if (auto retVal{ m_interval->removeSubInterval (0) }; !retVal)
    {
      FAIL () << retVal.error ();
    }
  EXPECT_EQ (m_interval->count (), 1);

  // Remove missing
  auto retVal{ m_interval->removeSubInterval (LONG_MAX) };
  EXPECT_FALSE (retVal);
}