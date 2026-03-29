module;
#include <chrono>
#include <cstdint>
#include <stdexcept>
module workoutlib;

namespace Workouts
{
namespace ErgFile
{
void
writeWorkout (std::iostream &file, std::string_view workoutName,
              std::string_view notes, uint16_t ftp)
{
  file << "[COURSE HEADER]" << "\n";
  file << "VERSION = 2" << "\n";
  file << "UNITS = GERMAN" << "\n";
  file << "DESCRIPTION = " << notes << "\n";
  file << "FILE NAME = " << workoutName << "\n";
  file << "FTP = " << ftp << "\n";
  file << "MINUTES WATTS" << "\n";
  file << "[END COURSE HEADER]" << "\n";
  file << "[COURSE DATA]" << "\n";
}
void
writeWorkout (std::iostream &file, Workout &workout)
{
  auto notes{ workout.getNotes () };
  auto workoutName{ workout.getName () };
  auto ftp{ workout.getFtp () };
  writeWorkout (file, workoutName, notes, ftp);
}
std::expected<Workout, std::string>
readWorkout (std::istream &file)
{
  std::string line;
  std::string notes;
  constexpr const uint8_t beginNotes{ 14 };
  std::string workoutName;
  constexpr const uint8_t beginName{ 12 };
  uint16_t ftp{ 0 };
  constexpr const uint8_t beginFtp{ 6 };
  while (std::getline (file, line))
    {
      if (line.starts_with ("DESCRIPTION ="))
        {
          notes = line.substr (beginNotes);
        }
      else if (line.starts_with ("FILE NAME ="))
        {
          workoutName = line.substr (beginName);
          if (workoutName.empty ())
            {
              return std::unexpected ("Cannot read workout name.");
            }
        }
      else if (line.starts_with ("FTP ="))
        {
          try
            {
              ftp = std::stoi (line.substr (beginFtp));
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
  return workout;
}
void
writeAbsoluteWatt (std::iostream &file, double startTime, double endTime,
                   ValueRange &value)
{
  file << std::fixed << std::setprecision (2) << startTime << "\t"
       << value.From << "\n";
  file << std::fixed << std::setprecision (2) << startTime << "\t" << endTime
       << "\n";
}
void
writeInterval (std::iostream &file, Interval &interval, WorkoutType, uint16_t)
{
  static double startTime{ 0.0 };
  static double endTime{ 0.0 };

  // Convert seconds to fractions of a minute
  constexpr uint8_t secondsInMinute{ 60 };
  using dMinutes = std::chrono::duration<double, std::ratio<secondsInMinute>>;
  dMinutes duration{ interval.getDuration () };

  endTime = startTime + duration.count ();
  ValueRange intensity = interval.getIntensityRange ();

  writeAbsoluteWatt (file, startTime, endTime, intensity);
  startTime = endTime;
}

std::expected<std::list<Interval>, std::string>
readIntervals (std::istream &file)
{
  std::string line;
  std::list<Interval> intervals;
  constexpr uint8_t secondsInMinute{ 60 };
  bool intervalsFound{ false };
  bool isTimeLine{ true };
  double startTime{ 0.0 };
  double endTime{ 0.0 };
  int watts{ 0 };
  std::chrono::seconds duration;
  while (std::getline (file, line))
    {
      if (line == "[COURSE DATA]")
        {
          intervalsFound = true;
          continue;
        }
      if (intervalsFound && isTimeLine)
        {
          try
            {
              // End of first number in the line
              auto charactersFirst{ line.find_first_of ('\t') };
              // Beginn of characters of watts
              auto charactersWatts{ line.find_last_of ('\t') };
              startTime = std::stod (line.substr (0, charactersFirst));
              watts = std::stoi (line.substr (
                  charactersWatts, line.length () - charactersWatts));
              isTimeLine = false;
            }
          catch (std::invalid_argument &e)
            {
              return std::unexpected (e.what ());
            }
        }
      else if (intervalsFound && !isTimeLine)
        {
          try
            {
              // Find end time of interval
              auto charactersEnd{ line.find_first_of ('\t') };
              endTime
                  = std::stod (line.substr (charactersEnd, line.length ()));
              auto minutes = std::chrono::duration<
                  double, std::ratio<secondsInMinute>> (endTime - startTime);
              duration
                  = std::chrono::duration_cast<std::chrono::seconds> (minutes);
              intervals.emplace_back (watts, WorkoutType::AbsoluteWatt,
                                      duration);
              isTimeLine = true;
            }
          catch (std::invalid_argument &e)
            {
              return std::unexpected (e.what ());
            }
        }
    }
  return intervals;
}
} // namespace ErgFile

namespace MrcFile
{
void
writeWorkout (std::iostream &file, std::string_view workoutName,
              std::string_view notes)
{
  file << "[COURSE HEADER]" << "\n";
  file << "VERSION = 2" << "\n";
  file << "UNITS = GERMAN" << "\n";
  file << "DESCRIPTION = " << notes << "\n";
  file << "FILE NAME = " << workoutName << "\n";
  file << "MINUTES PERCENT" << "\n";
  file << "[END COURSE HEADER]" << "\n";
  file << "[COURSE DATA]" << "\n";
}
void
writeWorkout (std::iostream &file, Workout &workout)
{
  auto notes{ workout.getNotes () };
  auto workoutName{ workout.getName () };
  writeWorkout (file, workoutName, notes);
}
std::expected<Workout, std::string>
readWorkout (std::istream &file)
{
  std::string line;
  std::string notes;
  constexpr const uint8_t beginNotes{ 14 };
  std::string workoutName;
  constexpr const uint8_t beginName{ 12 };
  uint16_t ftp{ 0 };
  constexpr const uint8_t beginFtp{ 6 };
  while (std::getline (file, line))
    {
      if (line.starts_with ("DESCRIPTION ="))
        {
          notes = line.substr (beginNotes);
        }
      else if (line.starts_with ("FILE NAME ="))
        {
          workoutName = line.substr (beginName);
          if (workoutName.empty ())
            {
              return std::unexpected ("Cannot read workout name.");
            }
        }
    }
  Workout workout (workoutName);
  workout.setNotes (notes);
  workout.setFtp (ftp);
  return workout;
}
void
writePercentFTP (std::iostream &file, double startTime, double endTime,
                 ValueRange &value)
{
  file << std::fixed << std::setprecision (2) << startTime << "\t"
       << value.From << "\n";
  file << std::fixed << std::setprecision (2) << endTime << "\t" << value.To
       << "\n";
}
void
writeInterval (std::iostream &file, Interval &interval, WorkoutType type,
               uint16_t relativeTo)
{
  static double startTime{ 0.0 };
  static double endTime{ 0.0 };

  // Convert seconds to fractions of a minute
  constexpr uint8_t secondsInMinute{ 60 };
  using dMinutes = std::chrono::duration<double, std::ratio<secondsInMinute>>;
  dMinutes duration{ interval.getDuration () };

  endTime = startTime + duration.count ();
  ValueRange intensity = interval.getIntensityRange (relativeTo, type);

  writePercentFTP (file, startTime, endTime, intensity);
  startTime = endTime;
}
std::expected<std::list<Interval>, std::string>
readIntervals (std::istream &file)
{
  std::string line;
  std::list<Interval> intervals;
  constexpr uint8_t secondsInMinute{ 60 };
  bool intervalsFound{ false };
  bool isTimeLine{ true };
  double startTime{ 0.0 };
  double endTime{ 0.0 };
  int startWatts{ 0 };
  uint16_t endWatts{ 0 };
  std::chrono::seconds duration;
  while (std::getline (file, line))
    {
      if (line == "[COURSE DATA]")
        {
          intervalsFound = true;
          continue;
        }
      if (intervalsFound && isTimeLine)
        {
          try
            {
              // End of first number in the line
              auto charactersFirst{ line.find_first_of ('\t') };
              // Beginn of characters of watts
              auto charactersWatts{ line.find_last_of ('\t') };
              startTime = std::stod (line.substr (0, charactersFirst));
              startWatts = std::stoi (line.substr (
                  charactersWatts, line.length () - charactersWatts));
              isTimeLine = false;
            }
          catch (std::invalid_argument &e)
            {
              return std::unexpected (e.what ());
            }
        }
      else if (intervalsFound && !isTimeLine)
        {
          try
            {
              // Find end time of interval
              auto charactersEnd{ line.find_first_of ('\t') };
              endTime = std::stod (line.substr (0, charactersEnd));
              auto minutes = std::chrono::duration<
                  double, std::ratio<secondsInMinute>> (endTime - startTime);
              duration
                  = std::chrono::duration_cast<std::chrono::seconds> (minutes);

              // Get end watts
              auto charactersWatts{ line.find_last_of ('\t') };
              endWatts = std::stoi (line.substr (
                  charactersWatts, line.length () - charactersWatts));
              intervals.emplace_back (startWatts, endWatts,
                                      WorkoutType::PercentFTP, duration);
              isTimeLine = true;
            }
          catch (std::invalid_argument &e)
            {
              return std::unexpected (e.what ());
            }
        }
    }
  return intervals;
}
} // namespace MrcFile

} // namespace Workouts