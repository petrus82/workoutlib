module planworkout;

using namespace Workouts;
void
PlanInterval::writeCommon ()
{
  m_file << "=INTERVAL=" << std::endl << std::endl;
}
void
PlanInterval::writeAbsoluteWatt ()
{
  m_file << "PWR_LO=" << m_value.From << std::endl;
  m_file << "PWR_HI=" << m_value.To << std::endl;
  m_file << "MESG_DURATION_SEC>="
         << m_duration.Minutes * 60 + m_duration.Seconds << "?EXIT"
         << std::endl;
}
void
PlanInterval::writePercentFTP ()
{
  m_file << "PERCENT_FTP_LO=" << m_value.From << std::endl;
  m_file << "PERCENT_FTP_HI=" << m_value.To << std::endl;
  m_file << "MESG_DURATION_SEC>="
         << m_duration.Minutes * 60 + m_duration.Seconds << "?EXIT"
         << std::endl;
}

std::size_t
PlanWorkout::createInterval (WorkoutType type, ValueRange value,
                             Duration duration)
{
  m_intervals.emplace_back (
      std::make_shared<PlanInterval> (m_file, type, value, duration));
  m_duration += duration.Minutes * 60 + duration.Seconds;
  return m_intervals.size () - 1;
}

void
PlanWorkout::writeWorkout ()
{
  m_file << "=HEADER=" << std::endl << std::endl;
  m_file << "NAME=" << m_workoutName << std::endl << std::endl;
  m_file << "DURATION=" << std::to_string (m_duration) << std::endl;
  m_file << "PLAN_TYPE=0" << std::endl;
  m_file << "WORKOUT_TYPE=0" << std::endl;
  m_file << "DESCRIPTION=" << m_notes << std::endl << std::endl;
  m_file << "=STREAM=" << std::endl << std::endl;
}

void
PlanWorkout::wrapDescription (std::string &string)
{
  for (std::size_t i = 0; i < string.size (); ++i)
    {
      if (i > 0 && (i % 80) == 0)
        {
          string.insert (i, "\nDESCRIPTION=");
          i += 12;
        }
    }
}