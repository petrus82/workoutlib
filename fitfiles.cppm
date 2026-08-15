export module fitfiles;

import config;
import common;
import filehandling;
import std;
import fitmodule;
import interval;

namespace Workouts
{
namespace fitFiles
{

const constexpr int AbsolutePowerOffset = 1000;
const constexpr int AbsoluteHeartRateOffset = 100;
// convert from minutes::seconds to msec.
constexpr const auto msecInSec{ 1000U };
constexpr const auto secInMinute{ 60U };
intervalReturn getFitInterval (const fit::WorkoutStepMesg &msg,
                               const CapacityT &capacity);
constexpr fit::WorkoutStepMesg writeFitInterval (const Interval &interval);

/**
 * @brief Internal helper function used by getFitInterval to set the
 * intensity of the interval from the fit message.
 *
 * @param interval The interval to set the intensity for.
 * @param intensity The numerical intensity value from the fit message. If
 * above the AbsolutePowerOffset or AbsoluteHeartRateOffset, it is an
 * absolute value; otherwise, it is a relative value.
 * @param isLowValue Indicates whether the intensity value is the low or
 * high value of the interval.
 * @param isPower If true, the intensity value is a power value; if false,
 * it is a heart rate value.
 * @return A std::expected<void, std::string>. In case of an error (e.g.
 * invalid intensity value), the error message is returned as a string.
 */
constexpr void applyIntensity (Interval &interval, uintType intensity,
                               Level level, bool isPower)
{
  if (isPower)
    {
      if (intensity >= AbsolutePowerOffset)
        {
          interval.getIntensity ().setTarget (intensity - AbsolutePowerOffset,
                                              IntensityUnit::Watts, level);
          return;
        }
      interval.getIntensity ().setTarget (intensity, IntensityUnit::PercentFTP,
                                          level);
      return;
    }
  if (intensity >= AbsoluteHeartRateOffset)
    {
      interval.getIntensity ().setTarget (intensity - AbsoluteHeartRateOffset,
                                          IntensityUnit::HeartRateBPM, level);
      return;
    }
  interval.getIntensity ().setTarget (intensity, IntensityUnit::PercentMaxHR,
                                      level);
}

/**
 * @brief Returns an Interval object based on the data from a
 * fit::WorkoutStepMesg
 *
 * @param msg The fit::WorkoutStepMesg containing the interval data.
 * @param capValues This struct holds the ftp and max heart rate values
 * that are needed to convert relative intensity values to absolute values.
 * @return A std::expected<Interval, std::string>. In case of an error
 * (e.g. invalid intensity value), the error message is returned as a
 * string.
 */
intervalReturn getFitInterval (const fit::WorkoutStepMesg &msg,
                               CapacityT capValues)
{
  Interval interval{};
  if (std::holds_alternative<FtpType> (capValues))
    {
      interval.getIntensity ().setFTP (std::get<FtpType> (capValues));
    }
  else
    {
      interval.getIntensity ().setMaxHeartRate (std::get<HrType> (capValues));
    }
  if (msg.IsCustomTargetPowerLowValid () != 0U)
    {
      auto intensityLow{ msg.GetCustomTargetPowerLow () };
      applyIntensity (interval, intensityLow, Level::Low, true);
    }
  if (msg.IsCustomTargetPowerHighValid () != 0U)
    {
      auto intensityHigh{ msg.GetCustomTargetPowerHigh () };
      applyIntensity (interval, intensityHigh, Level::High, true);
    }
  if (msg.IsCustomTargetHeartRateLowValid () != 0U)
    {
      auto heartRateLow{ msg.GetCustomTargetHeartRateLow () };
      applyIntensity (interval, heartRateLow, Level::Low, false);
    }
  if (msg.IsCustomTargetHeartRateHighValid () != 0U)
    {
      auto heartRateHigh{ msg.GetCustomTargetHeartRateHigh () };
      applyIntensity (interval, heartRateHigh, Level::High, false);
    }
  if (msg.IsTargetPowerZoneValid () != 0U)
    {
      auto pwrZone{ msg.GetTargetPowerZone () };
      interval.getIntensity ().setTarget (pwrZone, IntensityUnit::PowerZone,
                                          Level::Low);
    }
  if (msg.IsTargetHrZoneValid () != 0U)
    {
      auto hrZone{ msg.GetTargetHrZone () };
      interval.getIntensity ().setTarget (hrZone, IntensityUnit::HeartRateZone,
                                          Level::Low);
    }
  if (msg.IsDurationTimeValid () != 0U)
    {
      auto duration{ msg.GetDurationValue () };
      interval.setDuration (std::chrono::seconds (duration));
    }
  return interval;
}
constexpr fit::WorkoutStepMesg writeFitInterval (const Interval &interval)
{
  fit::WorkoutStepMesg msg;
  msg.SetIntensity (FIT_INTENSITY_ACTIVE);
  msg.SetDurationType (FIT_WKT_STEP_DURATION_TIME);
  auto IntensityUnit{ interval.getIntensity ().getType () };

  std::pair<uintType, uintType> intensity{
    interval.getIntensity ().getTarget (Level::Low),
    interval.getIntensity ().getTarget (Level::High)
  };

  switch (IntensityUnit)
    {
    case IntensityUnit::Watts:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetCustomTargetPowerLow (intensity.first + AbsolutePowerOffset);
      msg.SetCustomTargetPowerHigh (intensity.second + AbsolutePowerOffset);
      break;
    case IntensityUnit::PercentFTP:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetCustomTargetPowerLow (intensity.first);
      msg.SetCustomTargetPowerHigh (intensity.second);
      break;
    case IntensityUnit::HeartRateBPM:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetCustomTargetHeartRateLow (intensity.first
                                       + AbsoluteHeartRateOffset);
      msg.SetCustomTargetHeartRateHigh (intensity.second
                                        + AbsoluteHeartRateOffset);
      break;
    case IntensityUnit::PercentMaxHR:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetCustomTargetHeartRateLow (intensity.first);
      msg.SetCustomTargetHeartRateHigh (intensity.second);
      break;
    case IntensityUnit::PowerZone:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
      msg.SetTargetPowerZone (intensity.first);
      break;
    case IntensityUnit::HeartRateZone:
      msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
      msg.SetTargetHrZone (intensity.first);
      break;
    default: std::unreachable ();
    }
  msg.SetDurationTime (interval.getDuration ().count ());
  return msg;
}

/* Untested!
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
        workoutStepMsg.SetCustomTargetPowerHigh (value.To +
  AbsolutePowerOffset);
      }
    else
      {
        // Power zone
        workoutStepMsg.SetTargetPowerZone (value.From);
      }
    encoder.Write (workoutStepMsg);
  }
  void writePercentFTP (fit::Encode &encoder,
                        fit::WorkoutStepMesg &workoutStepMsg, ValueRange
  &value)
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
                                                    +
  AbsoluteHeartRateOffset); workoutStepMsg.SetCustomTargetHeartRateHigh
  (value.To
                                                    +
  AbsoluteHeartRateOffset);
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
    auto m_encoder = std::make_unique<fit::Encode>
  (fit::ProtocolVersion::V20); m_encoder->Open (file);

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
          if (workoutMesg.IsWktNameValid () != 0U)
            {
              auto w_workoutName = workoutMesg.GetWktName ();
              std::string workoutName{ w_workoutName.begin (),
                                      w_workoutName.end () };
              m_workout.setName (workoutName);
            }
          if (workoutMesg.IsWktDescriptionValid () != 0U)
            {
              auto w_description{ workoutMesg.GetWktDescription () };
              std::string description{ w_description.begin (),
                                      w_description.end () };
              m_workout.setNotes (description);
            }
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
*/
/*   std::string wrapDescription (std::string_view stringview)
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
  } */

export constexpr std::expected<void, std::string>
CheckValidFit (fit::Decode &decoder, std::ifstream &file) noexcept
{
  if (bool isInvalid{ FIT_FILE_INVALID == decoder.CheckIntegrity (file) };
      isInvalid)
    {
      return std::unexpected ("Not a valid fit file");
    }
  return {};
}

export constexpr std::expected<std::string, std::string>
getWorkoutName (fit::Decode &decoder, const std::ifstream &filestream)
{ return std::string{}; }

export constexpr std::expected<std::string, std::string>
getWorkoutNotes (fit::Decode &decoder, const std::ifstream &filestream)
{ return std::string{}; }

export template <typename W, typename I>
  requires WorkoutT<W, I>
std::expected<W, std::string> getIntervals (fit::Decode &decoder,
                                            const std::ifstream &filestream,
                                            W &&workout)
{ return std::forward<W> (workout); }

} // namespace fitFiles
}; // namespace Workouts