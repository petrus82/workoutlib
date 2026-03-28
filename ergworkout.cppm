export module ergworkout;

import workoutlib;
import std;
import std.compat;

using namespace Workouts;
export class ErgInterval : public Interval
{
public:
  explicit ErgInterval (std::fstream &file, WorkoutType intervalType,
                        ValueRange value, Duration duration)
      : Interval (file, intervalType, value, duration)
  {
  }
  void
  writeCommon () override
  {
  }
  void writeAbsoluteWatt () override;
  void
  writePercentFTP () override
  {
    throw std::runtime_error (
        "Please provide absolute Watts for an ERG file.");
  }
  void
  writeAbsoluteHeartRate () override
  {
    throw std::runtime_error (
        "Please provide absolute Watts for an ERG file.");
  }
  void
  writePercentMaxHeartRate () override
  {
    throw std::runtime_error (
        "Please provide absolute Watts for an ERG file.");
  }
};

export class ErgWorkout : public Workout
{
public:
  explicit ErgWorkout (const std::string file, std::string workoutName,
                       std::string notes)
      : Workout (file, workoutName, notes)
  {
  }

  ~ErgWorkout ()
  {
    writeWorkout ();
    writeToFile ();
  }

  std::size_t createInterval (WorkoutType type, ValueRange value,
                              Duration duration) override;
  void setFtp (uint16_t ftp);
  void writeWorkout () override;

private:
  uint16_t m_ftp{};
};
