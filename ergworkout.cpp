module ergworkout;

using namespace Workouts;
void
ErgInterval::writeAbsoluteWatt ()
{
  m_file << std::fixed << std::setprecision (2) << m_startTime << "\t"
         << m_value.From << std::endl;
  m_file << std::fixed << std::setprecision (2)
         << m_startTime + m_duration.Minutes + 1 / 60 * m_duration.Seconds
         << "\t" << m_value.To << std::endl;
}

std::size_t
ErgWorkout::createInterval (WorkoutType type, ValueRange value,
                            Duration duration)
{
  if (type == WorkoutType::PercentFTP)
    {
      throw std::runtime_error (
          "No relative power possible for an ERG Workout. Please provide "
          "absolute watt targets.");
    }
  else if (type == WorkoutType::AbsoluteHeartRate
           || type == WorkoutType::PercentMaxHeartRate)
    {
      throw std::runtime_error (
          "No heart rate target possible for an ERG Workout. Please provide "
          "absolute watt targets.");
    }
  m_intervals.emplace_back (
      std::make_shared<ErgInterval> (m_file, type, value, duration));
  return m_intervals.size () - 1;
}

void
ErgWorkout::setFtp (uint16_t ftp)
{
  if (ftp > 0 && ftp < 2000)
    {
      m_ftp = ftp;
    }
  else
    {
      throw std::runtime_error (
          "ftp must be an integer number above 0 and below 2000.");
    }
}

void
ErgWorkout::writeWorkout ()
{
  m_file << "[COURSE HEADER]" << std::endl;
  m_file << "VERSION = 2" << std::endl;
  m_file << "UNITS = GERMAN" << std::endl;
  m_file << "DESCRIPTION = " << m_notes << std::endl;
  m_file << "FILE NAME = " << m_workoutName << std::endl;
  m_file << "FTP = " << m_ftp << std::endl;
  m_file << "MINUTES WATTS" << std::endl;
  m_file << "[END COURSE HEADER]" << std::endl;
  m_file << "[COURSE DATA]" << std::endl;
}