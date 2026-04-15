module workoutlib;

namespace Workouts
{

void writeToStream (std::iostream &file, std::string_view key,
                    std::string_view value, std::string_view tagSeparator)
{ file << key << " " << tagSeparator << " " << value << '\n'; }

void writeWorkout (std::iostream &file, const TextFileFormat &fileformat,
                   Workout &workout)
{
  file << fileformat.headerStart;

  writeToStream (file, fileformat.nameTag, workout.getName (),
                 fileformat.headerSeparator);
  if (!fileformat.headerDuration.empty ())
    {
      long workoutDuration{};
      for (const auto &interval : workout)
        {
          workoutDuration += interval.getDuration ().count ();
        }
      writeToStream (file, fileformat.headerDuration,
                     std::to_string (workoutDuration).append ("\n"),
                     fileformat.headerSeparator);
    }
  if (!fileformat.headerSpec.empty ())
    {
      file << fileformat.headerSpec;
    }
  writeToStream (file, fileformat.noteTag, workout.getNotes (),
                 fileformat.headerSeparator);
  if (!fileformat.intensityUnitTag.empty ())
    {
      writeToStream (file, fileformat.intensityUnitTag,
                     std::to_string (workout.getFtp ()),
                     fileformat.headerSeparator);
    }

  file << fileformat.headerEnd;
  double startTime{};
  for (const auto &interval : workout)
    {
      startTime += writeIntensityDuration (file, fileformat, interval,
                                           fileformat.type, startTime);
    }
}

double writeIntensityDuration (std::iostream &file,
                               const TextFileFormat &fileFormat,
                               const Interval &interval, IntensityType type,
                               double startTime)
{
  double endTime{ startTime
                  + std::chrono::duration<double, std::ratio<60>> (
                        interval.getDuration ())
                        .count () };
  auto typeValue{ std::to_underlying (type) };
  std::uint16_t intensityLo{};
  std::uint16_t intensityHi{};

  if (type == IntensityType::PowerAbsHigh
      || type == IntensityType::PowerRelHigh)
    {
      intensityLo = interval.getIntensity (
          static_cast<IntensityType> (std::to_underlying (type) - 1));
      intensityHi = interval.getIntensity (type);
    }
  else
    {
      intensityLo = interval.getIntensity (type);
      intensityHi = interval.getIntensity (
          static_cast<IntensityType> (std::to_underlying (type) + 1));
    }
  file << std::fixed << std::setprecision (3) << startTime << "\t"
       << intensityLo << "\n";
  file << std::fixed << std::setprecision (3) << endTime << "\t" << intensityHi
       << "\n";
  return endTime;
}

void writeIntensityTime (std::iostream &file, const TextFileFormat &fileFormat,
                         const Interval &interval, IntensityType type)
{
  file << fileFormat.intervalTag << "\n\n";

  if (type == IntensityType::PowerAbsHigh
      || type == IntensityType::PowerAbsLow)
    {
      file << fileFormat.intervalIntensityAbsLoTag
           << fileFormat.intervalSeparator << interval.getIntensity (type)
           << '\n';
      file << fileFormat.intervalIntensityAbsHiTag
           << fileFormat.intervalSeparator << interval.getIntensity (type)
           << '\n';
    }
  else if (type == IntensityType::PowerRelHigh
           || type == IntensityType::PowerRelLow)
    {
      file << fileFormat.intervalIntensityRelLoTag
           << fileFormat.intervalSeparator << interval.getIntensity (type)
           << '\n';
      file << fileFormat.intervalIntensityRelHiTag
           << fileFormat.intervalSeparator << interval.getIntensity (type)
           << '\n';
    }
  file << fileFormat.intervalDurationTag << fileFormat.intervalSeparator
       << interval.getDuration ().count () << "?EXIT\n";
}

namespace fitFiles
{

void writeCommon (Duration &duration, uint16_t index)
{
  fit::WorkoutStepMesg workoutStepMsg;
  workoutStepMsg.SetMessageIndex (index);
  workoutStepMsg.SetIntensity (FIT_INTENSITY_ACTIVE);
  workoutStepMsg.SetDurationType (FIT_WKT_STEP_DURATION_TIME);
  workoutStepMsg.SetDurationValue (
      ((duration.Minutes * secInMinute) + duration.Seconds) * msecInSec);
}
void writeAbsoluteWatt (fit::Encode &encoder,
                        fit::WorkoutStepMesg &workoutStepMsg,
                        ValueRange &value)
{
  workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  if (value.To > 0)
    {
      // Power range
      workoutStepMsg.SetCustomTargetPowerLow (value.From
                                              + AbsolutePowerOffset);
      workoutStepMsg.SetCustomTargetPowerHigh (value.To + AbsolutePowerOffset);
    }
  else
    {
      // Power zone
      workoutStepMsg.SetTargetPowerZone (value.From);
    }
  encoder.Write (workoutStepMsg);
}
void writePercentFTP (fit::Encode &encoder,
                      fit::WorkoutStepMesg &workoutStepMsg, ValueRange &value)
{
  workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
  if (value.To > 0)
    {
      // power range
      workoutStepMsg.SetCustomTargetPowerLow (value.From);
      workoutStepMsg.SetCustomTargetPowerHigh (value.To);
    }
  else
    {
      // power zone
      workoutStepMsg.SetTargetPowerZone (value.From);
    }
  encoder.Write (workoutStepMsg);
}
void writeAbsoluteHeartRate (fit::Encode &encoder,
                             fit::WorkoutStepMesg &workoutStepMsg,
                             ValueRange &value)
{
  workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  if (value.To > 0)
    {
      // heart rate range
      workoutStepMsg.SetCustomTargetHeartRateLow (value.From
                                                  + AbsoluteHeartRateOffset);
      workoutStepMsg.SetCustomTargetHeartRateHigh (value.To
                                                   + AbsoluteHeartRateOffset);
    }
  else
    {
      // heart rate zone
      workoutStepMsg.SetTargetHrZone (value.From);
    }
  encoder.Write (workoutStepMsg);
}
void writePercentMaxHeartRate (fit::Encode &encoder,
                               fit::WorkoutStepMesg &workoutStepMsg,
                               ValueRange &value)
{
  workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
  if (value.To > 0)
    {
      // heart rate range

      workoutStepMsg.SetCustomTargetHeartRateLow (value.From);
      workoutStepMsg.SetCustomTargetHeartRateHigh (value.To);
    }
  else
    {
      // heart rate zone
      workoutStepMsg.SetTargetHrZone (value.From);
    }
  encoder.Write (workoutStepMsg);
}
void writeWorkout (std::iostream &file, std::string_view workoutName,
                   uint16_t intervalSteps)
{
  auto m_encoder = std::make_unique<fit::Encode> (fit::ProtocolVersion::V20);
  m_encoder->Open (file);

  fit::FileIdMesg fileIDMesg;
  fileIDMesg.SetType (FIT_FILE_WORKOUT);
  fileIDMesg.SetManufacturer (FIT_MANUFACTURER_DEVELOPMENT);
  fileIDMesg.SetProduct (1);

  fit::DateTime startTime (std::time (0));
  fileIDMesg.SetTimeCreated (startTime.GetTimeStamp ());

  // Create unique serial number
  srand ((unsigned int)time (nullptr));
  constexpr auto seed{ 10000U };
  fileIDMesg.SetSerialNumber ((rand () % seed) + 1);
  m_encoder->Write (fileIDMesg);

  // Workout Message
  fit::WorkoutMesg workoutMsg;
  workoutMsg.SetWktName (
      std::wstring (workoutName.begin (), workoutName.end ()));
  workoutMsg.SetSport (FIT_SPORT_CYCLING);
  workoutMsg.SetNumValidSteps (intervalSteps);
  m_encoder->Write (workoutMsg);
}
void writeWorkout (std::iostream &file, Workout &workout)
{
  auto workoutName{ workout.getName () };
  auto intervalSteps{ workout.intervalCount () };
  writeWorkout (file, workoutName, intervalSteps);
}
struct Listener : public fit::MesgListener
{
  void OnMesg (fit::Mesg &mesg) override
  {
    auto mesgName = mesg.GetName ();
    std::println ("Name: {}", mesgName);
    if (mesgName == "file_id")
      {
        fit::FileIdMesg fileIdMesg (mesg);
        std::println ("File ID: {}", fileIdMesg.GetSerialNumber ());
        std::println ("Type: {}", fileIdMesg.GetType ());
      }
    else if (mesgName == "workout")
      {
        fit::WorkoutMesg workoutMesg (mesg);
        auto w_workoutName = workoutMesg.GetWktName ();
        std::string workoutName{ w_workoutName.begin (),
                                 w_workoutName.end () };
        std::println ("Workout Name: {}", workoutName);
      }
    else if (mesgName == "workout_step")
      {
        fit::WorkoutStepMesg workoutStepMsg (mesg);
        auto targetType{ workoutStepMsg.GetTargetType () };
        std::println ("Target Type: {}", targetType);

        // TODO: Get the real values
        CapacityValues capValues{ .maxHeartRate = 180, .ftp = 300 };

        if (auto interval{ getFitInterval (mesg, capValues) }; interval)
          {
            m_workout.createInterval (*interval);
          }
      }
  }

  Workout getWorkout () { return std::move (m_workout); }
  Workout m_workout;
};
std::expected<Workout, std::string> readWorkout (std::istream &file)
{
  fit::Decode decoder;
  Listener listener;
  decoder.Read (file, listener);
  Workout workout{ listener.getWorkout () };
  return workout;
}
// readIntervals does not exist for fit files. It has to be done in
// readWorkout for fit files.
}

std::string wrapDescription (std::string_view stringview)
{
  std::string string (stringview);
  constexpr uint8_t lineLength{ 80 };
  constexpr uint8_t descriptionLength{ 12 };
  for (std::size_t i = 0; i < string.size (); ++i)
    {
      if (i > 0 && (i % lineLength) == 0)
        {
          string.insert (i, "\nDESCRIPTION=");
          i += descriptionLength;
        }
    }
  return string;
}

} // namespace Workouts