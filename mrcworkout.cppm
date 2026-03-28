export module mrcworkout;

import workoutlib;
import std;

using namespace Workouts;
export class MrcInterval : public Interval
{
public:
  explicit MrcInterval (std::fstream &file, WorkoutType intervalType,
                        ValueRange value, Duration duration)
      : Interval (file, intervalType, value, duration)
  {
  }

  void
  writeCommon () override
  {
  }
  void
  writeAbsoluteWatt () override
  {
    throw std::runtime_error ("Please provide relative FTP for an MRC file.");
  }
  void writePercentFTP () override;
  void
  writeAbsoluteHeartRate () override
  {
    throw std::runtime_error ("Please provide relative FTP for an MRC file.");
  }
  void
  writePercentMaxHeartRate () override
  {
    throw std::runtime_error ("Please provide relative FTP for an MRC file.");
  }
};

export class MrcWorkout : public Workout
{
public:
  explicit MrcWorkout (const std::string file, std::string workoutName,
                       std::string notes)
      : Workout (file, workoutName, notes)
  {
  }
  ~MrcWorkout ()
  {
    writeWorkout ();
    writeToFile ();
  }

  std::size_t createInterval (WorkoutType type, ValueRange value,
                              Duration duration) override;
  void writeWorkout () override;
};
