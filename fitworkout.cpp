module fitworkout;

using namespace Workouts;
void
FitInterval::writeCommon ()
{
  m_workoutStepMesg = std::make_unique<fit::WorkoutStepMesg> ();
  m_workoutStepMesg->SetMessageIndex (m_index);
  m_workoutStepMesg->SetIntensity (FIT_INTENSITY_ACTIVE);
  m_workoutStepMesg->SetDurationType (FIT_WKT_STEP_DURATION_TIME);
  m_workoutStepMesg->SetDurationValue (
      (m_duration.Minutes * 60 + m_duration.Seconds) * 1000);
}

void
FitInterval::writeAbsoluteWatt ()
{
  m_workoutStepMesg->SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  if (m_value.To > 0)
    {
      // Power range
      m_workoutStepMesg->SetCustomTargetPowerLow (m_value.From
                                                  + AbsolutePowerOffset);
      m_workoutStepMesg->SetCustomTargetPowerHigh (m_value.To
                                                   + AbsolutePowerOffset);
    }
  else
    {
      // Power zone
      m_workoutStepMesg->SetTargetPowerZone (m_value.From);
    }
  m_encoder->Write (*m_workoutStepMesg);
}

void
FitInterval::writePercentFTP ()
{
  m_workoutStepMesg->SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  if (m_value.To > 0)
    {
      // power range
      m_workoutStepMesg->SetCustomTargetPowerLow (m_value.From);
      m_workoutStepMesg->SetCustomTargetPowerHigh (m_value.To);
    }
  else
    {
      // power zone
      m_workoutStepMesg->SetTargetPowerZone (m_value.From);
    }
  m_encoder->Write (*m_workoutStepMesg);
}

void
FitInterval::writeAbsoluteHeartRate ()
{
  m_workoutStepMesg->SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  if (m_value.To > 0)
    {
      // heart rate range
      m_workoutStepMesg->SetCustomTargetHeartRateLow (
          m_value.From + AbsoluteHeartRateOffset);
      m_workoutStepMesg->SetCustomTargetHeartRateHigh (
          m_value.To + AbsoluteHeartRateOffset);
    }
  else
    {
      // heart rate zone
      m_workoutStepMesg->SetTargetHrZone (m_value.From);
    }
  m_encoder->Write (*m_workoutStepMesg);
}

void
FitInterval::writePercentMaxHeartRate ()
{
  m_workoutStepMesg->SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  if (m_value.To > 0)
    {
      // heart rate range

      m_workoutStepMesg->SetCustomTargetHeartRateLow (m_value.From);
      m_workoutStepMesg->SetCustomTargetHeartRateHigh (m_value.To);
    }
  else
    {
      // heart rate zone
      m_workoutStepMesg->SetTargetHrZone (m_value.From);
    }
  m_encoder->Write (*m_workoutStepMesg);
}

void
FitWorkout::writeWorkout ()
{
  m_encoder = std::make_unique<fit::Encode> (fit::ProtocolVersion::V20);
  m_encoder->Open (m_file);

  fit::FileIdMesg fileIDMesg;
  fileIDMesg.SetType (FIT_FILE_WORKOUT);
  fileIDMesg.SetManufacturer (FIT_MANUFACTURER_DEVELOPMENT);
  fileIDMesg.SetProduct (1);

  fit::DateTime startTime (std::time (0));
  fileIDMesg.SetTimeCreated (startTime.GetTimeStamp ());

  // Create unique serial number
  srand ((unsigned int)time (nullptr));
  fileIDMesg.SetSerialNumber (rand () % 10000 + 1);
  m_encoder->Write (fileIDMesg);

  // Workout Message
  fit::WorkoutMesg workoutMsg;
  workoutMsg.SetWktName (
      std::wstring (m_workoutName.begin (), m_workoutName.end ()));
  workoutMsg.SetSport (FIT_SPORT_CYCLING);
  workoutMsg.SetNumValidSteps (m_intervals.size ());
  m_encoder->Write (workoutMsg);
}