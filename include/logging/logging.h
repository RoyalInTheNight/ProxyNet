#ifndef __LOGGING
#define __LOGGING

#include <filesystem>
#include <iostream>
#include <fstream>

namespace logging {
    enum class LoggingStatus : uint8_t {
        loggingFile = 0xd1,
        loggingStdout,
        loggingDisable
    };

    class Logging {
    private:
        LoggingStatus log_status_;
        std::string     log_path_;

    public:
        void setLogging(const LoggingStatus& lstatus_) { log_status_ = lstatus_; }
        void setPath(const std::string& lpath_) { log_path_ = lpath_; }

        [[nodiscard]] LoggingStatus getLogging() const { return this->log_status_; }
        
        void logging(const std::string& message) {
            if (this->log_status_ == LoggingStatus::loggingStdout)
                std::cout << message << std::endl;

            if (this->log_status_ == LoggingStatus::loggingFile) {
                std::ofstream file(this->log_path_);

                if (file.is_open())
                    file << message << std::endl;

                file.close();
            }
        }
    };
}

#endif // __LOGGING