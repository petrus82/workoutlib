module mrcworkout;

using namespace Workouts;
void
MrcInterval::writePercentFTP ()
{
  m_file << std::fixed << std::setprecision (2) << m_startTime << "\t"
         << m_value.From << std::endl;
  m_file << std::fixed << std::setprecision (2)
         << m_startTime + m_duration.Minutes + 1 / 60 * m_duration.Seconds
         << "\t" << m_value.To << std::endl;
}

std::size_t
MrcWorkout::createInterval (WorkoutType type, ValueRange value,
                            Duration duration)
{
  if (type == WorkoutType::AbsoluteWatt)
    {
      throw std::runtime_error ("No absolute watt target possible for a MRC "
                                "workout. Please provide \% ftp targets.");
    }
  else if (type == WorkoutType::AbsoluteHeartRate
           || type == WorkoutType::PercentMaxHeartRate)
    {
      throw std::runtime_error ("No heart rate target possible for a MRC "
                                "Workout. Please provide \% ftp targets.");
    }
  m_intervals.emplace_back (
      std::make_unique<MrcInterval> (m_file, type, value, duration));
  return m_intervals.size () - 1;
}

void
MrcWorkout::writeWorkout ()
{
  m_file << "[COURSE HEADER]" << std::endl;
  m_file << "VERSION = 2" << std::endl;
  m_file << "UNITS = GERMAN" << std::endl;
  m_file << "DESCRIPTION = " << m_notes << std::endl;
  m_file << "FILE NAME = " << m_workoutName << std::endl;
  m_file << "MINUTES PERCENT" << std::endl;
  m_file << "[END COURSE HEADER]" << std::endl;
  m_file << "[COURSE DATA]" << std::endl;
}