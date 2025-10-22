#pragma once

#include "types.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

namespace SwarmCog {
namespace Utils {

/**
 * UUID Generation utilities
 */
class UUIDGenerator {
private:
    static std::random_device rd_;
    static std::mt19937 gen_;
    static std::uniform_int_distribution<> dis_;

public:
    static std::string generate();
    static std::string generateShort(size_t length = 8);
};

/**
 * Time utilities
 */
class TimeUtils {
public:
    static std::string timestampToString(const Timestamp& timestamp);
    static Timestamp stringToTimestamp(const std::string& str);
    static std::chrono::milliseconds timeSince(const Timestamp& timestamp);
    static std::string formatDuration(std::chrono::milliseconds duration);
    static Timestamp now() { return std::chrono::system_clock::now(); }
};

/**
 * String utilities
 */
class StringUtils {
public:
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& strings, const std::string& separator);
    static std::string trim(const std::string& str);
    static std::string toLower(const std::string& str);
    static std::string toUpper(const std::string& str);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
};

/**
 * Mathematical utilities
 */
class MathUtils {
public:
    static double clamp(double value, double min_val, double max_val);
    static double normalize(double value, double min_val, double max_val);
    static double sigmoid(double x);
    static double euclideanDistance(const std::vector<double>& a, const std::vector<double>& b);
    static double cosineSimilarity(const std::vector<double>& a, const std::vector<double>& b);
    static double average(const std::vector<double>& values);
    static double standardDeviation(const std::vector<double>& values);
    static std::vector<double> softmax(const std::vector<double>& values);
};

/**
 * Collections utilities
 */
template<typename T>
class CollectionUtils {
public:
    static std::vector<T> intersect(const std::vector<T>& a, const std::vector<T>& b) {
        std::vector<T> result;
        std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), 
                            std::back_inserter(result));
        return result;
    }
    
    static std::vector<T> unite(const std::vector<T>& a, const std::vector<T>& b) {
        std::vector<T> result;
        std::set_union(a.begin(), a.end(), b.begin(), b.end(), 
                      std::back_inserter(result));
        return result;
    }
    
    static std::vector<T> subtract(const std::vector<T>& a, const std::vector<T>& b) {
        std::vector<T> result;
        std::set_difference(a.begin(), a.end(), b.begin(), b.end(), 
                           std::back_inserter(result));
        return result;
    }
    
    static bool contains(const std::vector<T>& container, const T& item) {
        return std::find(container.begin(), container.end(), item) != container.end();
    }
    
    static void removeDuplicates(std::vector<T>& container) {
        std::sort(container.begin(), container.end());
        container.erase(std::unique(container.begin(), container.end()), container.end());
    }
};

/**
 * Configuration utilities
 */
class ConfigUtils {
public:
    static std::map<std::string, std::string> parseKeyValue(const std::string& input);
    static std::string serializeKeyValue(const std::map<std::string, std::string>& data);
    static bool parseBool(const std::string& str);
    static int parseInt(const std::string& str, int default_value = 0);
    static double parseDouble(const std::string& str, double default_value = 0.0);
};

/**
 * Validation utilities
 */
class ValidationUtils {
public:
    static bool isValidAgentId(const std::string& agent_id);
    static bool isValidAtomId(const std::string& atom_id);
    static bool isValidTrustLevel(double trust_level);
    static bool isValidConfidence(double confidence);
    static bool isValidProbability(double probability);
    static bool isValidCapabilityName(const std::string& name);
    static std::string sanitizeString(const std::string& input);
};

/**
 * Logging utilities
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class Logger {
private:
    static LogLevel current_level_;
    static std::mutex log_mutex_;
    static bool enable_console_output_;
    static std::string log_file_path_;

public:
    static void setLogLevel(LogLevel level) { current_level_ = level; }
    static void enableConsoleOutput(bool enabled) { enable_console_output_ = enabled; }
    static void setLogFile(const std::string& file_path) { log_file_path_ = file_path; }
    
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void critical(const std::string& message);
    
private:
    static void log(LogLevel level, const std::string& message);
    static std::string formatLogMessage(LogLevel level, const std::string& message);
    static std::string levelToString(LogLevel level);
};

/**
 * Performance monitoring utilities
 */
class PerformanceMonitor {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    std::string operation_name_;
    
public:
    explicit PerformanceMonitor(const std::string& operation_name);
    ~PerformanceMonitor();
    
    std::chrono::milliseconds getElapsedTime() const;
    void reset();
};

/**
 * Memory management utilities
 */
class MemoryUtils {
public:
    static size_t getCurrentMemoryUsage();
    static size_t getPeakMemoryUsage();
    static void logMemoryUsage(const std::string& context = "");
    static double getMemoryUsagePercent();
};

/**
 * Thread utilities
 */
class ThreadUtils {
public:
    static size_t getOptimalThreadCount();
    static void setThreadName(const std::string& name);
    static std::string getThreadName();
    static void sleepFor(std::chrono::milliseconds duration);
    static bool waitForCondition(const std::function<bool()>& condition, 
                                std::chrono::milliseconds timeout);
};

/**
 * Error handling utilities
 */
class ErrorHandler {
public:
    static void handleException(const std::exception& e, const std::string& context = "");
    static void handleUnknownException(const std::string& context = "");
    static std::string getStackTrace();
    static void registerErrorCallback(const std::function<void(const std::string&)>& callback);
};

/**
 * Serialization utilities
 */
class SerializationUtils {
public:
    static std::string serializeTruthValue(const TruthValue& tv);
    static TruthValue deserializeTruthValue(const std::string& str);
    
    static std::string serializeAttentionValue(const AttentionValue& av);
    static AttentionValue deserializeAttentionValue(const std::string& str);
    
    static std::string serializeCognitiveState(const CognitiveState& state);
    static CognitiveState deserializeCognitiveState(const std::string& str);
    
    static std::string serializeTimestamp(const Timestamp& timestamp);
    static Timestamp deserializeTimestamp(const std::string& str);
};

/**
 * Network utilities (for distributed processing)
 */
class NetworkUtils {
public:
    static bool isPortOpen(const std::string& host, int port);
    static std::string getLocalIPAddress();
    static std::string getHostname();
    static bool isValidIPAddress(const std::string& ip);
    static std::vector<std::string> resolveHostname(const std::string& hostname);
};

// Convenience macros for common operations
#define SWARMCOG_LOG_DEBUG(msg) SwarmCog::Utils::Logger::debug(msg)
#define SWARMCOG_LOG_INFO(msg) SwarmCog::Utils::Logger::info(msg)
#define SWARMCOG_LOG_WARNING(msg) SwarmCog::Utils::Logger::warning(msg)
#define SWARMCOG_LOG_ERROR(msg) SwarmCog::Utils::Logger::error(msg)
#define SWARMCOG_LOG_CRITICAL(msg) SwarmCog::Utils::Logger::critical(msg)

#define SWARMCOG_PERF_MONITOR(name) SwarmCog::Utils::PerformanceMonitor _perf_mon(name)

// Template utilities for type conversions
template<typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template<typename T>
T fromString(const std::string& str) {
    std::istringstream iss(str);
    T value;
    iss >> value;
    return value;
}

} // namespace Utils
} // namespace SwarmCog