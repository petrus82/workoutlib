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
writeInterval (std::iostream &file, Interval &interval, WorkoutType)
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
}
}