export module trainingdatawriter;
import common;

namespace Workouts
{

export extern "C++" 
    {
        void setFtp(FtpType ftp)
        {
            TrainingData::instance().m_ftp = ftp;
        }

        void setMaxHeartRate(HrType heartRate)
        {
            TrainingData::instance().m_maxHeartRate = heartRate;
        }

        void setMinHeartRate(HrType heartRate)
        {
            TrainingData::instance().m_minHeartRate = heartRate;
        }
    }

} // namespace Workouts