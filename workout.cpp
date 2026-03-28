module workoutlib;

using namespace Workouts;
[[nodiscard]] float
Interval::writeToFile (float startTime)
{
  m_startTime = startTime;
  writeCommon ();
  switch (m_type)
    {
    case WorkoutType::AbsoluteWatt:
      writeAbsoluteWatt ();
      break;
    case WorkoutType::PercentFTP:
      writePercentFTP ();
      break;
    case WorkoutType::AbsoluteHeartRate:
      writeAbsoluteHeartRate ();
      break;
    case WorkoutType::PercentMaxHeartRate:
      writePercentMaxHeartRate ();
      break;
    default:
      throw std::runtime_error ("No implementation for type "
                                + intervalTypeToString (m_type)
                                + "in FitInterval.");
    }
  // Return startTime of the next interval which is current startTime +
  // Duration of current interval
  float endTime{ m_startTime + (float)m_duration.Minutes
                 + (float)m_duration.Seconds * 1 / 60 };
  return endTime;
}

void
Workout::createRepeat (Repeat repeat)
{
  std::vector<std::shared_ptr<Interval>> repeatVector{};

  // Get pointers of intervalls to repeat
  std::vector<std::shared_ptr<Interval>>::iterator startPosition;
  for (std::size_t i = 0; i < m_intervals.size (); ++i)
    {
      if (i == repeat.From)
        {
          startPosition = m_intervals.begin () + i;
        }
      if (i >= repeat.From && i <= repeat.To)
        {
          repeatVector.emplace_back (m_intervals[i]);
        }
    }

  // Add repeat vector to intervalls repeat.Times
  for (uint8_t j = 0; j < repeat.Times; ++j)
    {
      startPosition = m_intervals.insert (startPosition, repeatVector.begin (),
                                          repeatVector.end ());
    }
}

void
Workout::writeToFile ()
{
  float startTime{};
  for (const auto &it : m_intervals)
    {
      startTime = it->writeToFile (startTime);
    }
}