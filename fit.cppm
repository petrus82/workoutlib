module;
#include "fit.hpp"
#include "fit_date_time.hpp"
#include "fit_decode.hpp"
#include "fit_mesg.hpp"
#include "fit_mesg_broadcaster.hpp"
#include "fit_mesg_listener.hpp"
#include "fit_profile.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fit_encode.hpp>
#include <fit_file_id_mesg.hpp>
#include <fit_workout_mesg.hpp>
#include <fit_workout_step_mesg.hpp>
#include <fit_workout_step_mesg_listener.hpp>
#include <iosfwd>
#include <map>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <unordered_map>
#include <vector>

export module fitmodule;
import std;
export import fitprofile;

export extern "C++" namespace fit
{
  using fit::DateTime;
  using fit::Decode;
  using fit::Encode;
  using fit::FileIdMesg;
  using fit::FileIdMesgListener;
  using fit::Mesg;
  using fit::MesgBroadcaster;
  using fit::MesgListener;
  using fit::ProtocolVersion;
  using fit::WorkoutMesg;
  using fit::WorkoutMesgListener;
  using fit::WorkoutStepMesg;
  using fit::WorkoutStepMesgListener;
}