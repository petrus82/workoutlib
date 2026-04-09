module workoutlib;

namespace Workouts
{

std::expected<std::list<Interval>, std::string>
readIntervals (std::istream &file, const TextFileFormat &fileformat,
               uint16_t ftp)
{
  std::string line;
  std::list<Interval> intervals;
  const constexpr std::uint8_t secondsInMinute{ 60 };
  bool intervalsFound{ false };
  bool isIntervalEnd{ true };
  double startTime{ 0.0 };
  double endTime{ 0.0 };
  int intensityLow{};
  int intensityHigh{};
  std::chrono::seconds duration;
  while (std::getline (file, line))
    {
      if (line.empty ())
        {
          continue;
        }

      if (line == fileformat.intervalTag)
        {
          intervalsFound = true;
          continue;
        }

      if (intervalsFound && line == fileformat.intervalIntensityAbsLoTag)
        {
          auto charactersEnd{ fileformat.intervalIntensityAbsLoTag.size () };
          intensityLow = std::stoi (
              line.substr (charactersEnd, line.length () - charactersEnd));
        }
      else if (intervalsFound && line == fileformat.intervalIntensityAbsHiTag)
        {
          auto charactersEnd{ fileformat.intervalIntensityAbsHiTag.size () };
          intensityHigh = std::stoi (
              line.substr (charactersEnd, line.length () - charactersEnd));
        }
      else if (intervalsFound && line == fileformat.intervalDurationTag)
        {
          auto charactersEnd{ fileformat.intervalDurationTag.size () };
          long seconds{ std::stol (
              line.substr (charactersEnd, line.length () - charactersEnd)) };
          duration = std::chrono::duration<long, std::ratio<1>> (seconds);
        }
    }
  return intervals;
}

std::expected<Workout, std::string>
readWorkout (std::istream &file, const TextFileFormat &fileformat)
{
  std::string line;
  std::string notes;
  std::string workoutName;
  uint16_t ftp{ 0 };
  while (std::getline (file, line))
    {
      if (line.starts_with (fileformat.noteTag))
        {
          notes = line.substr (fileformat.noteTag.length ());
        }
      else if (line.starts_with (fileformat.nameTag))
        {
          workoutName = line.substr (fileformat.nameTag.length ());
          if (workoutName.empty ())
            {
              return std::unexpected ("Cannot read workout name.");
            }
        }
      else if (line.starts_with (fileformat.intensityUnitTag))
        {
          try
            {
              ftp = std::stoi (
                  line.substr (fileformat.intensityUnitTag.length ()));
            }
          catch (...)
            {
              return std::unexpected ("Cannot read FTP.");
            }
        }
    }
  Workout workout (workoutName);
  workout.setNotes (notes);
  workout.setFtp (ftp);
  auto intervals{ readIntervals (file, fileformat, workout.getFtp ()) };
  return workout;
}
void
writeWorkout (std::iostream &file, const TextFileFormat &fileformat,
              Workout &workout)
{
  file << fileformat.headerStart;
  file << fileformat.nameTag << workout.getName () << '\n';
  if (!fileformat.headerDuration.empty ())
    {
      long workoutDuration{};
      for (const auto &interval : workout)
        {
          workoutDuration += interval.getDuration ().count ();
        }
      file << fileformat.headerDuration << workoutDuration << "\n\n";
    }
  file << fileformat.noteTag << workout.getNotes () << '\n';
  if (!fileformat.intensityUnitTag.empty ())
    {
      file << fileformat.intensityUnitTag << workout.getFtp () << '\n';
    }

  file << fileformat.headerEnd;
  double startTime{};
  for (const auto &interval : workout)
    {
      startTime += writeIntensityDuration (file, fileformat, interval,
                                           fileformat.type, startTime);
    }
}
double
writeIntensityDuration (std::iostream &file, const TextFileFormat &fileFormat,
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

void
writeIntensityTime (std::iostream &file, const TextFileFormat &fileFormat,
                    const Interval &interval, IntensityType type)
{
  file << fileFormat.intervalTag << '\n';
  file << fileFormat.intervalIntensityAbsLoTag << interval.getIntensity (type)
       << '\n';
  file << fileFormat.intervalIntensityAbsHiTag << interval.getIntensity (type)
       << '\n';
  file << fileFormat.intervalDurationTag << interval.getDuration ().count ()
       << "?EXIT\n";
}

namespace FitFile
{
static const constexpr int AbsolutePowerOffset = 1000;
static const constexpr int AbsoluteHeartRateOffset = 100;
// convert from minutes::seconds to msec.
static constexpr const auto msecInSec{ 1000U };
static constexpr const auto secInMinute{
  60U
}; // convert from minutes::seconds to msec.
void
writeCommon (Duration &duration, uint16_t index)
{
  fit::WorkoutStepMesg workoutStepMsg;
  workoutStepMsg.SetMessageIndex (index);
  workoutStepMsg.SetIntensity (FIT_INTENSITY_ACTIVE);
  workoutStepMsg.SetDurationType (FIT_WKT_STEP_DURATION_TIME);
  workoutStepMsg.SetDurationValue (
      ((duration.Minutes * secInMinute) + duration.Seconds) * msecInSec);
}
void
writeAbsoluteWatt (fit::Encode &encoder, fit::WorkoutStepMesg &workoutStepMsg,
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
void
writePercentFTP (fit::Encode &encoder, fit::WorkoutStepMesg &workoutStepMsg,
                 ValueRange &value)
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
void
writeAbsoluteHeartRate (fit::Encode &encoder,
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
void
writePercentMaxHeartRate (fit::Encode &encoder,
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
void
writeWorkout (std::iostream &file, std::string_view workoutName,
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
void
writeWorkout (std::iostream &file, Workout &workout)
{
  auto workoutName{ workout.getName () };
  auto intervalSteps{ workout.intervalCount () };
  writeWorkout (file, workoutName, intervalSteps);
}
struct Listener : public fit::MesgListener
{
  void
  OnMesg (fit::Mesg &mesg) override
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
        Interval interval{};
        IntensityType type{};

        // That should be replaced with GetTargetPowerZone,
        // GetCustomTargetPowerLow/High or GetCustomTargetHeartRateLow/High,
        // GetTargetHrZone
        uint16_t intensity{};
        if (workoutStepMsg.IsTargetHrZoneValid () != 0U)
          {
            intensity = workoutStepMsg.GetTargetHrZone ();
            interval.setIntensity (intensity, IntensityType::HeartRateZoneLow);
          }
        else if (workoutStepMsg.IsCustomTargetHeartRateLowValid () != 0U)
          {
            intensity = workoutStepMsg.GetCustomTargetHeartRateLow ();
            interval.setIntensity (intensity, IntensityType::HeartRateAbsLow);
          }
        else if (workoutStepMsg.IsCustomTargetPowerLowValid () != 0U)
          {
            intensity = workoutStepMsg.GetCustomTargetPowerLow ();
            interval.setIntensity (intensity, IntensityType::PowerAbsLow);
          }
        long duration{ workoutStepMsg.GetDurationValue () };
        duration = duration / msecInSec;
        std::println ("Duration: {} sec", duration);
        interval.setDuration (
            std::chrono::duration<long, std::ratio<1>> (duration));
        m_workout.createInterval (interval);
      }
  }

  Workout
  getWorkout ()
  {
    return std::move (m_workout);
  }
  Workout m_workout;
};
std::expected<Workout, std::string>
readWorkout (std::istream &file)
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

/*
std::expected<std::list<Interval>, std::string>
readIntervals (std::istream &file)
{
  std::list<Interval> intervals;
  std::string line;
  bool intervalsFound{ false };
  long seconds{ 0 };
  uint16_t startWatts{ 0 };
  uint16_t endWatts{ 0 };
  std::chrono::seconds duration;
  WorkoutType type{};
  constexpr const uint8_t charWatt{ 7 };
  constexpr const uint8_t charFtp{ 15 };
  constexpr const uint8_t charDuration{ 19 };
  using namespace std::literals;
  while (std::getline (file, line))
    {
      if (line.starts_with ("=INTERVAL="sv))
        {
          intervalsFound = true;
          continue;
        }
      if (intervalsFound)
        {
          if (line.starts_with ("PWR_LO="sv))
            {
              startWatts = std::stoi (
                  line.substr (charWatt, line.length () - charWatt));
              type = WorkoutType::AbsoluteWatt;
            }
          else if (line.starts_with ("PWR_HI="sv))
            {
              endWatts = std::stoi (
                  line.substr (charWatt, line.length () - charWatt));
              type = WorkoutType::AbsoluteWatt;
            }
          else if (line.starts_with ("PERCENT_FTP_LO="sv))
            {
              startWatts = std::stoi (
                  line.substr (charFtp, line.length () - charFtp));
              type = WorkoutType::PercentFTP;
            }
          else if (line.starts_with ("PERCENT_FTP_HI="sv))
            {
              endWatts = std::stoi (
                  line.substr (charFtp, line.length () - charFtp));
              type = WorkoutType::PercentFTP;
            }
          else if (line.starts_with ("MESG_DURATION_SEC>="sv))
            {
              try
                {
                  seconds = std::stol (line.substr (
                      charDuration, line.length () - charDuration));
                  duration
                      = std::chrono::duration<long, std::ratio<1>> (seconds);
                }
              catch (std::invalid_argument &e)
                {
                  return std::unexpected (e.what ());
                }
              if (endWatts == 0)
                {
                  endWatts = startWatts;
                }
              switch (type)
                {
                case WorkoutType::AbsoluteWatt:
                  intervals.emplace_back (startWatts, endWatts,
                                          WorkoutType::AbsoluteWatt,
duration); break; case WorkoutType::PercentFTP: intervals.emplace_back
(startWatts, endWatts, WorkoutType::PercentFTP, duration); break; default:
                  break;
                }
            }
        }
    }
  return intervals;
} */
std::string
wrapDescription (std::string_view stringview)
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
/* std::expected<Workout, std::string>
readWorkout (std::istream &file)
{
  std::string line;
  std::string notes;
  std::string workoutName;
  constexpr const uint8_t charName{ 5 };
  constexpr const uint8_t charDescription{ 12 };
  while (std::getline (file, line))
    {
      if (line.starts_with ("NAME="))
        {
          workoutName = line.substr (charName);
        }
      else if (line.starts_with ("DESCRIPTION="))
        {
          notes += line.substr (charDescription);
        }
    }
  Workout workout (workoutName);
  workout.setNotes (notes);
  return workout;
} */

void
Interval::calculatePower (uint16_t power, IntensityType type, uint16_t ftp)
{
  std::size_t typeValue{ std::to_underlying (type) };
  if (type == IntensityType::PowerAbsLow
      || type == IntensityType::PowerAbsHigh)
    {
      m_intensityPower.at (typeValue) = power;
      m_intensityPower.at (typeValue + 2) = convertToRelative (power, ftp);
      m_intensityPower.at (typeValue + 4) = convertToPowerZone (power, ftp);
      if (type == IntensityType::PowerAbsHigh
          && m_intensityPower.at (typeValue - 1) == 0)
        {
          m_intensityPower.at (typeValue - 1) = power;
        }
    }
  else if (type == IntensityType::PowerRelLow
           || type == IntensityType::PowerRelHigh)
    {
      m_intensityPower.at (typeValue) = power;
      m_intensityPower.at (typeValue - 2) = convertToAbsolute (power, ftp);
      m_intensityPower.at (typeValue + 2) = convertToPowerZone (power);
      if (type == IntensityType::PowerRelHigh
          && m_intensityPower.at (typeValue - 1) == 0)
        {
          m_intensityPower.at (typeValue - 1) = power;
        }
    }
}
void
Interval::calculateHeartRate (uint8_t heartRate, IntensityType type,
                              uint8_t maxHeartRate)
{
  std::size_t typeValue{ std::to_underlying (type) };
  if (type == IntensityType::HeartRateAbsLow
      || type == IntensityType::HeartRateAbsHigh)
    {
      m_intensityHeartRate.at (typeValue) = heartRate;
      m_intensityHeartRate.at (typeValue + 2)
          = convertToRelative (heartRate, maxHeartRate);
      m_intensityHeartRate.at (typeValue + 4)
          = convertToHeartRateZone (heartRate, maxHeartRate);
    }
  else if (type == IntensityType::HeartRateRelLow
           || type == IntensityType::HeartRateRelHigh)
    {
      m_intensityHeartRate.at (typeValue) = heartRate;
      m_intensityHeartRate.at (typeValue - 2)
          = convertToAbsolute (heartRate, maxHeartRate);
      m_intensityHeartRate.at (typeValue + 2)
          = convertToHeartRateZone (heartRate, maxHeartRate);
    }
}
uint16_t
Interval::convertToRelative (uint16_t intensity, uint16_t value)
{
  constexpr uint16_t percent{ 100 };
  intensity *= percent;
  return intensity / value;
}

uint16_t
Interval::convertToAbsolute (uint16_t intensity, uint16_t value)
{
  constexpr int percent{ 100 };
  return intensity * std::div (value, percent).quot;
}

uint8_t
Interval::convertToPowerZone (uint16_t intensity, uint16_t ftp)
{
  if (ftp > 0)
    // Intensity is % of FTP
    // Calculate relative power first
    {
      intensity = convertToRelative (intensity, ftp);
    }

  if (intensity < 55)
    {
      return 1;
    }
  else if (intensity >= 55 && intensity <= 75)
    {
      return 2;
    }
  else if (intensity > 75 && intensity <= 90)
    {
      return 3;
    }
  else if (intensity > 90 && intensity <= 105)
    {
      return 4;
    }
  else if (intensity > 105 && intensity <= 120)
    {
      return 5;
    }
  else if (intensity > 120 && intensity <= 150)
    {
      return 6;
    }
  else
    {
      return 7;
    }
}
uint8_t
Interval::convertFromPowerZone (uint8_t intensity, bool getLower)
{
  switch (intensity)
    {
    case 1:
      return getLower ? 0 : 54;
    case 2:
      return getLower ? 55 : 75;
    case 3:
      return getLower ? 76 : 90;
    case 4:
      return getLower ? 91 : 105;
    case 5:
      return getLower ? 106 : 120;
    case 6:
      return getLower ? 121 : 150;
    case 7:
      return getLower ? 151 : 200;
    }
}

uint8_t
Interval::convertToHeartRateZone (uint8_t intensity, uint8_t maxHeartRate)
{
  if (maxHeartRate > 0)
    {
      intensity = convertToRelative (intensity, maxHeartRate);
    }

  if (intensity > 50 && intensity <= 60)
    {
      return 1;
    }
  if (intensity > 60 && intensity <= 70)
    {
      return 2;
    }
  if (intensity > 70 && intensity <= 80)
    {
      return 3;
    }
  if (intensity > 80 && intensity <= 90)
    {
      return 4;
    }
  if (intensity > 90 && intensity <= 100)
    {
      return 5;
    }
}
uint8_t
Interval::convertFromHeartRateZone (uint8_t intensity, bool getLower)
{
  switch (intensity)
    {
    case 1:
      return getLower ? 50 : 59;
    case 2:
      return getLower ? 60 : 69;
    case 3:
      return getLower ? 70 : 79;
    case 4:
      return getLower ? 80 : 89;
    case 5:
      return getLower ? 90 : 100;
    }
}
} // namespace Workouts