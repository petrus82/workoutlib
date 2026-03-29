module;
#include <chrono>
#include <cstdint>
#include <fit_encode.hpp>
#include <fit_workout_step_mesg.hpp>
#include <ostream>
#include <ranges>
#include <sys/types.h>
#include <system_error>
export module workoutlib;

import config;
import std;
import fitmodule;
import std.compat;

#if TESTING == TRUE
#define EXPORT_TEST export
#else
#define EXPORT_TEST
#endif

namespace Workouts
{

export enum class WorkoutType : uint8_t {
  AbsoluteWatt,
  PercentFTP,
  AbsoluteHeartRate,
  PercentMaxHeartRate
};

export std::string
intervalTypeToString (WorkoutType type)
{
  switch (type)
    {
    case WorkoutType::AbsoluteWatt:
      return "AbsoluteWatt";
    case WorkoutType::PercentFTP:
      return "PercentFTP";
    case WorkoutType::AbsoluteHeartRate:
      return "AbsoluteHeartRate";
    case WorkoutType::PercentMaxHeartRate:
      return "PercentMaxHeartRate";
    default:
      return "Unknown";
    }
}
using uintType = uint16_t;
export struct Duration
{
  uintType Minutes;
  uintType Seconds;
};

export struct ValueRange
{
  uintType From;
  uintType To;
};

export class Interval;
export class Workout;
export enum class FileType : uint8_t { Fit, Plan, Erg, Mrc };

static constexpr int AbsolutePowerOffset = 1000;
static constexpr int AbsoluteHeartRateOffset = 100;

EXPORT_TEST namespace ErgFile
{

  void writeAbsoluteWatt (std::iostream & file, Duration & duration,
                          ValueRange & value, float startTime);
  void writeWorkout (std::iostream & file, std::string_view workoutName,
                     std::string_view notes, uint16_t ftp);
  void writeWorkout (std::iostream & file, Workout & workout);
  std::expected<Workout, std::string> readWorkout (std::istream & file);
  void writeInterval (std::iostream & file, Interval & interval, WorkoutType,
                      uint16_t relativeTo = 0);
  std::expected<std::list<Interval>, std::string> readIntervals (std::istream
                                                                 & file);
} // namespace ErgFile

EXPORT_TEST namespace FitFile
{
  void writeCommon (Duration & duration, uint16_t index)
  {
    fit::WorkoutStepMesg workoutStepMsg;
    workoutStepMsg.SetMessageIndex (index);
    workoutStepMsg.SetIntensity (FIT_INTENSITY_ACTIVE);
    workoutStepMsg.SetDurationType (FIT_WKT_STEP_DURATION_TIME);

    // convert from minutes::seconds to msec.
    constexpr auto msecInSec{ 1000U };
    constexpr auto secInMinute{ 60U };
    workoutStepMsg.SetDurationValue (
        ((duration.Minutes * secInMinute) + duration.Seconds) * msecInSec);
  }
  void writeAbsoluteWatt (fit::Encode & encoder,
                          fit::WorkoutStepMesg & workoutStepMsg,
                          ValueRange & value)
  {
    workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
    if (value.To > 0)
      {
        // Power range
        workoutStepMsg.SetCustomTargetPowerLow (value.From
                                                + AbsolutePowerOffset);
        workoutStepMsg.SetCustomTargetPowerHigh (value.To
                                                 + AbsolutePowerOffset);
      }
    else
      {
        // Power zone
        workoutStepMsg.SetTargetPowerZone (value.From);
      }
    encoder.Write (workoutStepMsg);
  }

  void writePercentFTP (fit::Encode & encoder,
                        fit::WorkoutStepMesg & workoutStepMsg,
                        ValueRange & value)
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

  void writeAbsoluteHeartRate (fit::Encode & encoder,
                               fit::WorkoutStepMesg & workoutStepMsg,
                               ValueRange & value)
  {
    workoutStepMsg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
    if (value.To > 0)
      {
        // heart rate range
        workoutStepMsg.SetCustomTargetHeartRateLow (value.From
                                                    + AbsoluteHeartRateOffset);
        workoutStepMsg.SetCustomTargetHeartRateHigh (
            value.To + AbsoluteHeartRateOffset);
      }
    else
      {
        // heart rate zone
        workoutStepMsg.SetTargetHrZone (value.From);
      }
    encoder.Write (workoutStepMsg);
  }

  void writePercentMaxHeartRate (fit::Encode & encoder,
                                 fit::WorkoutStepMesg & workoutStepMsg,
                                 ValueRange & value)
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
  void writeInterval (std::iostream & file, Interval & interval,
                      WorkoutType type, uint16_t relativeTo)
  {
  }
  void writeWorkout (std::fstream file, std::string_view workoutName,
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
  void writeWorkout (std::iostream & file, Workout & workout) {}
}

EXPORT_TEST namespace MrcFile
{
  void writePercentFTP (std::iostream & file, ValueRange & value,
                        Duration & duration, float startTime);

  void writeWorkout (std::iostream & file, std::string_view workoutName,
                     std::string_view notes);
  void writeWorkout (std::iostream & file, Workout & workout);
  std::expected<Workout, std::string> readWorkout (std::istream & file);
  void writeInterval (std::iostream & file, Interval & interval,
                      WorkoutType type, uint16_t relativeTo);
}

EXPORT_TEST namespace PlanFile
{
  constexpr const int secondsInMinute{ 60 };
  void writeCommon (std::iostream & file)
  {
    file << "=INTERVAL=" << "\n" << "\n";
  }
  void writeAbsoluteWatt (std::iostream & file, ValueRange & value,
                          Duration & duration)
  {
    file << "PWR_LO=" << value.From << "\n";
    file << "PWR_HI=" << value.To << "\n";

    file << "MESG_DURATION_SEC>="
         << (duration.Minutes * secondsInMinute) + duration.Seconds << "?EXIT"
         << "\n";
  }
  void writePercentFTP (std::iostream & file, ValueRange & value,
                        Duration & duration)
  {
    file << "PERCENT_FTP_LO=" << value.From << "\n";
    file << "PERCENT_FTP_HI=" << value.To << "\n";
    file << "MESG_DURATION_SEC>="
         << (duration.Minutes * secondsInMinute) + duration.Seconds << "?EXIT"
         << "\n";
  }
  void writeInterval (std::iostream & file, Interval & interval,
                      WorkoutType type, uint16_t relativeTo)
  {
  }
  void writeWorkout (std::iostream & file, Workout & workout) {}
  void writeWorkout (std::iostream & file, std::string_view workoutName,
                     uint32_t duration, std::string_view notes)
  {
    file << "=HEADER=" << "\n" << "\n";
    file << "NAME=" << workoutName << "\n" << "\n";
    file << "DURATION=" << std::to_string (duration) << "\n";
    file << "PLAN_TYPE=0" << "\n";
    file << "WORKOUT_TYPE=0" << "\n";
    file << "DESCRIPTION=" << notes << "\n" << "\n";
    file << "=STREAM=" << "\n" << "\n";
  }

  void wrapDescription (std::string & string)
  {
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
  }
}

class Interval
{
public:
  Interval () = default;

  explicit Interval (uintType intensity, WorkoutType type,
                     std::chrono::seconds duration)
      : m_value (intensity, intensity), m_type{ type }, m_duration{ duration }
  {
  }
  explicit Interval (uintType intensityFrom, uintType intensityTo,
                     WorkoutType type, std::chrono::seconds duration)
      : m_value (intensityFrom, intensityTo), m_type{ type },
        m_duration{ duration }
  {
  }
  // relativeTo is the ftp if called with WorkoutType::PercentFTP or the
  // maxHeartRate if called with PercentMaxHeartRate
  ValueRange
  getIntensityRange (uint16_t relativeTo = 0,
                     WorkoutType type = WorkoutType::AbsoluteWatt) const
  {
    if (type == m_type)
      {
        return m_value;
      }
    if (type == WorkoutType::AbsoluteWatt && m_type == WorkoutType::PercentFTP
        || m_type == WorkoutType::AbsoluteHeartRate
               && type == WorkoutType::PercentMaxHeartRate)
      {
        ValueRange absolute{};
        absolute.From = convertToAbsolute (m_value.From, relativeTo);
        absolute.To = convertToAbsolute (m_value.To, relativeTo);
        return absolute;
      }
    if (type == WorkoutType::PercentFTP && m_type == WorkoutType::AbsoluteWatt
        || type == WorkoutType::PercentMaxHeartRate
               && m_type == WorkoutType::AbsoluteHeartRate)
      {
        ValueRange relative{};
        relative.From = convertToRelative (m_value.From, relativeTo);
        relative.To = convertToRelative (m_value.To, relativeTo);
        return relative;
      }
    throw std::runtime_error (
        "Calculating power to or from heart rate is not possible.");
  }
  void
  setIntensity (uintType intensity, WorkoutType type)
  {
    m_type = type;
    m_value.From = intensity;
  }
  void
  setIntensity (uintType from, uintType to, WorkoutType type)
  {
    m_type = type;
    m_value.From = from;
    m_value.To = to;
  }

  // relativeTo is the ftp if called with WorkoutType::PercentFTP or the
  // maxHeartRate if called with PercentMaxHeartRate
  uintType
  getIntensity (uint16_t relativeTo = 0,
                WorkoutType type = WorkoutType::AbsoluteWatt) const
  {
    if (type == m_type)
      {
        return m_value.From;
      }
    if (type == WorkoutType::AbsoluteWatt && m_type == WorkoutType::PercentFTP
        || m_type == WorkoutType::AbsoluteHeartRate
               && type == WorkoutType::PercentMaxHeartRate)
      {
        return convertToAbsolute (m_value.From, relativeTo);
      }
    if (type == WorkoutType::PercentFTP && m_type == WorkoutType::AbsoluteWatt
        || type == WorkoutType::PercentMaxHeartRate
               && m_type == WorkoutType::AbsoluteHeartRate)
      {
        return convertToRelative (m_value.From, relativeTo);
      }
    throw std::runtime_error (
        "Calculating power to or from heart rate is not possible.");
  }

  std::chrono::seconds
  getDuration () const
  {
    return m_duration;
  }
  template <class Rep, class Period>
  void
  setDuration (std::chrono::duration<Rep, Period> duration)
  {
    m_duration = std::chrono::duration_cast<std::chrono::seconds> (duration);
  }

private:
  static uint16_t
  convertToRelative (uint16_t intensity, uint16_t value)
  {
    constexpr uint16_t percent{ 100 };
    intensity *= percent;
    return intensity / value;
  }

  static uint16_t
  convertToAbsolute (uint16_t intensity, uint16_t value)
  {
    constexpr int percent{ 100 };
    return intensity * std::div (value, percent).quot;
  }

private:
  ValueRange m_value;
  std::chrono::seconds m_duration;
  WorkoutType m_type;
};
using Intervals = std::list<Interval>;
using IteratorType = Intervals::iterator;
using WriteFunction = std::function<void (std::iostream &, Interval &,
                                          WorkoutType &, uint16_t)>;
class Workout
{
public:
  explicit Workout (std::string_view workoutName) : m_workoutName (workoutName)
  {
  }
  explicit Workout (std::string_view workoutName, std::string_view notes)
      : m_workoutName (workoutName), m_notes (notes)
  {
  }
  void
  createInterval (Interval &interval)
  {
    m_order.emplace_back (interval);
  }

  void
  createRepeat (const IteratorType &from, const IteratorType &to,
                uint8_t times)
  {
    auto range = std::ranges::subrange (from, to);
    auto repeated = std::views::repeat (range, times) | std::views::join;
    m_order.insert_range (from, repeated);
  }
  void
  removeIntervals (const IteratorType &from, const IteratorType &to)
  {
    m_order.erase (from, to);
  }

  auto
  begin ()
  {
    return m_order.begin ();
  }
  static auto
  next (std::list<std::weak_ptr<Interval>>::iterator current)
  {
    return std::next (current);
  }

  static auto
  prev (std::list<std::weak_ptr<Interval>>::iterator current)
  {
    return std::prev (current);
  }

  auto
  back ()
  {
    return m_order.back ();
  }

  std::expected<void, std::string>
  writeFile (std::filesystem::path &file, FileType fileType,
             WorkoutType workoutType, uint16_t relativeTo)
  {
    WriteFunction writeFunc;
    std::fstream filestream (file, std::ios::out);
    if (filestream.fail ())
      {
        std::error_code error;
        return std::unexpected (std::format (
            "Cannot open file {}. {}", file.string (), error.message ()));
      }
    switch (fileType)
      {
      case FileType::Erg:
        writeFunc = [] (std::iostream &filestream, Interval &interval,
                        WorkoutType type, uint16_t relativeTo)
          { ErgFile::writeInterval (filestream, interval, type, relativeTo); };
        ErgFile::writeWorkout (filestream, *this);
        break;
      case FileType::Fit:
        writeFunc = [] (std::iostream &filestream, Interval &interval,
                        WorkoutType type, uint16_t relativeTo)
          { FitFile::writeInterval (filestream, interval, type, relativeTo); };
        filestream.open (file, std::ios::out | std::ios::binary);
        FitFile::writeWorkout (filestream, *this);
        break;
      case FileType::Mrc:
        writeFunc = [] (std::iostream &filestream, Interval &interval,
                        WorkoutType type, uint16_t relativeTo)
          { MrcFile::writeInterval (filestream, interval, type, relativeTo); };
        MrcFile::writeWorkout (filestream, *this);
        break;
      case FileType::Plan:
        writeFunc = [] (std::iostream &filestream, Interval &interval,
                        WorkoutType type, uint16_t relativeTo)
          {
            PlanFile::writeInterval (filestream, interval, type, relativeTo);
          };
        PlanFile::writeWorkout (filestream, *this);
        break;
      }
    for (auto &interval : m_order)
      {
        writeFunc (filestream, interval, workoutType, relativeTo);
      }
    return {};
  }

  std::string
  getName () const
  {
    return m_workoutName;
  }
  void
  setName (std::string_view name)
  {
    m_workoutName = name;
  }

  std::string
  getNotes () const
  {
    return m_notes;
  }
  void
  setNotes (std::string_view notes)
  {
    m_notes = notes;
  }

  uint16_t
  getFtp () const
  {
    return m_ftp;
  }
  void
  setFtp (uint16_t ftp)
  {
    m_ftp = ftp;
  }

  uint8_t
  getMaxHeartRate () const
  {
    return m_maxHeartRate;
  }
  void
  setMaxHeartRate (uint8_t heartRate)
  {
    m_maxHeartRate = heartRate;
  }

  uint8_t
  getMinHeartRate () const
  {
    return m_minHeartRate;
  }
  void
  setMinHeartRate (uint8_t heartRate)
  {
    m_minHeartRate = heartRate;
  }

private:
  std::string m_workoutName;
  std::string m_notes;
  uint16_t m_ftp{ 0 };
  uint8_t m_maxHeartRate{ 0 };
  uint8_t m_minHeartRate{ 0 };
  Intervals m_order;
};

EXPORT_TEST std::expected<std::ifstream, std::string>
getFileStream (const std::filesystem::path &file)
{
  std::ifstream filestream (file, std::ios::binary);
  if (!filestream)
    {
      return std::unexpected (
          std::format ("Cannot open file {}", file.string ()));
    }
  return filestream;
}

EXPORT_TEST std::expected<bool, std::string>
isValidFit (const std::filesystem::path &file, fit::Decode &decoder)
{
  auto filestream{ getFileStream (file) };
  if (filestream)
    {
      return decoder.CheckIntegrity (filestream.value ());
    }
  return std::unexpected (filestream.error ());
}

} // namespace Workouts