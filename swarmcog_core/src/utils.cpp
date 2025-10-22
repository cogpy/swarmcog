#include "swarmcog/utils.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <regex>

namespace SwarmCog {
namespace Utils {

// UUIDGenerator implementation
std::random_device UUIDGenerator::rd_;
std::mt19937 UUIDGenerator::gen_(rd_());
std::uniform_int_distribution<> UUIDGenerator::dis_(0, 15);

std::string UUIDGenerator::generate() {
    std::stringstream ss;
    ss << std::hex;
    
    // Generate 32 hex digits
    for (int i = 0; i < 32; ++i) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << '-';
        }
        ss << dis_(gen_);
    }
    
    return ss.str();
}

std::string UUIDGenerator::generateShort(size_t length) {
    std::stringstream ss;
    ss << std::hex;
    
    for (size_t i = 0; i < length; ++i) {
        ss << dis_(gen_);
    }
    
    return ss.str();
}

// TimeUtils implementation
std::string TimeUtils::timestampToString(const Timestamp& timestamp) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()
    ) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

Timestamp TimeUtils::stringToTimestamp(const std::string& str) {
    // Simple implementation - in production would use proper parsing
    return std::chrono::system_clock::now();
}

std::chrono::milliseconds TimeUtils::timeSince(const Timestamp& timestamp) {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp);
}

std::string TimeUtils::formatDuration(std::chrono::milliseconds duration) {
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    
    std::stringstream ss;
    if (hours.count() > 0) {
        ss << hours.count() << "h ";
    }
    if (minutes.count() > 0) {
        ss << minutes.count() << "m ";
    }
    ss << seconds.count() << "." << std::setfill('0') << std::setw(3) << duration.count() << "s";
    
    return ss.str();
}

// StringUtils implementation
std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string StringUtils::join(const std::vector<std::string>& strings, const std::string& separator) {
    if (strings.empty()) return "";
    
    std::stringstream ss;
    ss << strings[0];
    
    for (size_t i = 1; i < strings.size(); ++i) {
        ss << separator << strings[i];
    }
    
    return ss.str();
}

std::string StringUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

std::string StringUtils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string StringUtils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c) { return std::toupper(c); });
    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix) {
    return str.length() >= prefix.length() && 
           str.compare(0, prefix.length(), prefix) == 0;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix) {
    return str.length() >= suffix.length() && 
           str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string StringUtils::replace(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    
    return result;
}

// MathUtils implementation
double MathUtils::clamp(double value, double min_val, double max_val) {
    return std::max(min_val, std::min(value, max_val));
}

double MathUtils::normalize(double value, double min_val, double max_val) {
    if (max_val == min_val) return 0.0;
    return (value - min_val) / (max_val - min_val);
}

double MathUtils::sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

double MathUtils::euclideanDistance(const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return -1.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    
    return std::sqrt(sum);
}

double MathUtils::cosineSimilarity(const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return -1.0;
    
    double dot_product = 0.0, norm_a = 0.0, norm_b = 0.0;
    
    for (size_t i = 0; i < a.size(); ++i) {
        dot_product += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    
    if (norm_a == 0.0 || norm_b == 0.0) return 0.0;
    
    return dot_product / (std::sqrt(norm_a) * std::sqrt(norm_b));
}

double MathUtils::average(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

double MathUtils::standardDeviation(const std::vector<double>& values) {
    if (values.size() <= 1) return 0.0;
    
    double mean = average(values);
    double variance = 0.0;
    
    for (double value : values) {
        double diff = value - mean;
        variance += diff * diff;
    }
    
    variance /= (values.size() - 1);
    return std::sqrt(variance);
}

std::vector<double> MathUtils::softmax(const std::vector<double>& values) {
    if (values.empty()) return {};
    
    // Find maximum for numerical stability
    double max_val = *std::max_element(values.begin(), values.end());
    
    std::vector<double> exp_values;
    double sum = 0.0;
    
    for (double value : values) {
        double exp_val = std::exp(value - max_val);
        exp_values.push_back(exp_val);
        sum += exp_val;
    }
    
    // Normalize
    for (double& exp_val : exp_values) {
        exp_val /= sum;
    }
    
    return exp_values;
}

// ConfigUtils implementation
std::map<std::string, std::string> ConfigUtils::parseKeyValue(const std::string& input) {
    std::map<std::string, std::string> result;
    auto lines = StringUtils::split(input, '\n');
    
    for (const auto& line : lines) {
        auto trimmed = StringUtils::trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;
        
        size_t pos = trimmed.find('=');
        if (pos != std::string::npos) {
            std::string key = StringUtils::trim(trimmed.substr(0, pos));
            std::string value = StringUtils::trim(trimmed.substr(pos + 1));
            result[key] = value;
        }
    }
    
    return result;
}

std::string ConfigUtils::serializeKeyValue(const std::map<std::string, std::string>& data) {
    std::stringstream ss;
    
    for (const auto& pair : data) {
        ss << pair.first << "=" << pair.second << "\n";
    }
    
    return ss.str();
}

bool ConfigUtils::parseBool(const std::string& str) {
    std::string lower = StringUtils::toLower(StringUtils::trim(str));
    return lower == "true" || lower == "1" || lower == "yes" || lower == "on";
}

int ConfigUtils::parseInt(const std::string& str, int default_value) {
    try {
        return std::stoi(StringUtils::trim(str));
    } catch (...) {
        return default_value;
    }
}

double ConfigUtils::parseDouble(const std::string& str, double default_value) {
    try {
        return std::stod(StringUtils::trim(str));
    } catch (...) {
        return default_value;
    }
}

// ValidationUtils implementation
bool ValidationUtils::isValidAgentId(const std::string& agent_id) {
    return !agent_id.empty() && agent_id.length() <= 64 && 
           std::regex_match(agent_id, std::regex("[a-zA-Z0-9_-]+"));
}

bool ValidationUtils::isValidAtomId(const std::string& atom_id) {
    return !atom_id.empty() && atom_id.length() <= 64;
}

bool ValidationUtils::isValidTrustLevel(double trust_level) {
    return trust_level >= 0.0 && trust_level <= 1.0;
}

bool ValidationUtils::isValidConfidence(double confidence) {
    return confidence >= 0.0 && confidence <= 1.0;
}

bool ValidationUtils::isValidProbability(double probability) {
    return probability >= 0.0 && probability <= 1.0;
}

bool ValidationUtils::isValidCapabilityName(const std::string& name) {
    return !name.empty() && name.length() <= 32 && 
           std::regex_match(name, std::regex("[a-zA-Z_][a-zA-Z0-9_]*"));
}

std::string ValidationUtils::sanitizeString(const std::string& input) {
    std::string result;
    
    for (char c : input) {
        if (std::isalnum(c) || c == '_' || c == '-' || c == ' ') {
            result += c;
        }
    }
    
    return StringUtils::trim(result);
}

// Logger implementation
LogLevel Logger::current_level_ = LogLevel::INFO;
std::mutex Logger::log_mutex_;
bool Logger::enable_console_output_ = true;
std::string Logger::log_file_path_;

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level_) return;
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::string formatted = formatLogMessage(level, message);
    
    if (enable_console_output_) {
        std::cout << formatted << std::endl;
    }
    
    if (!log_file_path_.empty()) {
        std::ofstream file(log_file_path_, std::ios::app);
        if (file.is_open()) {
            file << formatted << std::endl;
        }
    }
}

std::string Logger::formatLogMessage(LogLevel level, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    std::string timestamp = TimeUtils::timestampToString(now);
    
    std::stringstream ss;
    ss << "[" << timestamp << "] " << levelToString(level) << ": " << message;
    
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// PerformanceMonitor implementation
PerformanceMonitor::PerformanceMonitor(const std::string& operation_name) 
    : operation_name_(operation_name) {
    start_time_ = std::chrono::high_resolution_clock::now();
}

PerformanceMonitor::~PerformanceMonitor() {
    auto duration = getElapsedTime();
    Logger::debug("Performance: " + operation_name_ + " took " + 
                  TimeUtils::formatDuration(duration));
}

std::chrono::milliseconds PerformanceMonitor::getElapsedTime() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
}

void PerformanceMonitor::reset() {
    start_time_ = std::chrono::high_resolution_clock::now();
}

// MemoryUtils implementation  
size_t MemoryUtils::getCurrentMemoryUsage() {
    // Platform-specific implementation would go here
    // For now, return 0 as placeholder
    return 0;
}

size_t MemoryUtils::getPeakMemoryUsage() {
    // Platform-specific implementation would go here
    return 0;
}

void MemoryUtils::logMemoryUsage(const std::string& context) {
    size_t current = getCurrentMemoryUsage();
    Logger::debug("Memory usage" + (context.empty() ? "" : " (" + context + ")") + 
                  ": " + toString(current) + " bytes");
}

double MemoryUtils::getMemoryUsagePercent() {
    // Platform-specific implementation would go here
    return 0.0;
}

// ThreadUtils implementation
size_t ThreadUtils::getOptimalThreadCount() {
    size_t hardware_threads = std::thread::hardware_concurrency();
    return hardware_threads > 0 ? hardware_threads : 4;
}

void ThreadUtils::setThreadName(const std::string& name) {
    // Platform-specific implementation would go here
}

std::string ThreadUtils::getThreadName() {
    // Platform-specific implementation would go here
    return "unknown";
}

void ThreadUtils::sleepFor(std::chrono::milliseconds duration) {
    std::this_thread::sleep_for(duration);
}

bool ThreadUtils::waitForCondition(const std::function<bool()>& condition, 
                                  std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();
    
    while (!condition()) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed >= timeout) {
            return false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return true;
}

// ErrorHandler implementation
void ErrorHandler::handleException(const std::exception& e, const std::string& context) {
    std::string message = "Exception";
    if (!context.empty()) {
        message += " in " + context;
    }
    message += ": ";
    message += e.what();
    
    Logger::error(message);
}

void ErrorHandler::handleUnknownException(const std::string& context) {
    std::string message = "Unknown exception";
    if (!context.empty()) {
        message += " in " + context;
    }
    
    Logger::error(message);
}

std::string ErrorHandler::getStackTrace() {
    // Platform-specific implementation would go here
    return "Stack trace not available";
}

void ErrorHandler::registerErrorCallback(const std::function<void(const std::string&)>& callback) {
    // Implementation would store callbacks and call them on errors
}

// SerializationUtils implementation
std::string SerializationUtils::serializeTruthValue(const TruthValue& tv) {
    std::stringstream ss;
    ss << tv.strength << "," << tv.confidence;
    return ss.str();
}

TruthValue SerializationUtils::deserializeTruthValue(const std::string& str) {
    auto parts = StringUtils::split(str, ',');
    if (parts.size() != 2) {
        return TruthValue();
    }
    
    double strength = ConfigUtils::parseDouble(parts[0]);
    double confidence = ConfigUtils::parseDouble(parts[1]);
    
    return TruthValue(strength, confidence);
}

std::string SerializationUtils::serializeAttentionValue(const AttentionValue& av) {
    std::stringstream ss;
    ss << av.sti << "," << av.lti << "," << av.vlti;
    return ss.str();
}

AttentionValue SerializationUtils::deserializeAttentionValue(const std::string& str) {
    auto parts = StringUtils::split(str, ',');
    if (parts.size() != 3) {
        return AttentionValue();
    }
    
    double sti = ConfigUtils::parseDouble(parts[0]);
    double lti = ConfigUtils::parseDouble(parts[1]);
    double vlti = ConfigUtils::parseDouble(parts[2]);
    
    return AttentionValue(sti, lti, vlti);
}

std::string SerializationUtils::serializeCognitiveState(const CognitiveState& state) {
    std::map<std::string, std::string> data;
    data["agent_id"] = state.agent_id;
    data["current_phase"] = toString(static_cast<int>(state.current_phase));
    data["goals"] = StringUtils::join(state.goals, ",");
    data["intentions"] = StringUtils::join(state.intentions, ",");
    data["current_focus"] = StringUtils::join(state.current_focus, ",");
    data["last_update"] = serializeTimestamp(state.last_update);
    
    // Serialize beliefs
    std::stringstream beliefs_ss;
    bool first = true;
    for (const auto& belief : state.beliefs) {
        if (!first) beliefs_ss << ";";
        beliefs_ss << belief.first << "=" << belief.second;
        first = false;
    }
    data["beliefs"] = beliefs_ss.str();
    
    return ConfigUtils::serializeKeyValue(data);
}

CognitiveState SerializationUtils::deserializeCognitiveState(const std::string& str) {
    auto data = ConfigUtils::parseKeyValue(str);
    
    CognitiveState state;
    state.agent_id = data["agent_id"];
    state.current_phase = static_cast<CognitivePhase>(ConfigUtils::parseInt(data["current_phase"]));
    state.goals = StringUtils::split(data["goals"], ',');
    state.intentions = StringUtils::split(data["intentions"], ',');
    state.current_focus = StringUtils::split(data["current_focus"], ',');
    state.last_update = deserializeTimestamp(data["last_update"]);
    
    // Deserialize beliefs
    auto belief_pairs = StringUtils::split(data["beliefs"], ';');
    for (const auto& pair : belief_pairs) {
        auto kv = StringUtils::split(pair, '=');
        if (kv.size() == 2) {
            state.beliefs[kv[0]] = kv[1];
        }
    }
    
    return state;
}

std::string SerializationUtils::serializeTimestamp(const Timestamp& timestamp) {
    return TimeUtils::timestampToString(timestamp);
}

Timestamp SerializationUtils::deserializeTimestamp(const std::string& str) {
    return TimeUtils::stringToTimestamp(str);
}

// NetworkUtils implementation
bool NetworkUtils::isPortOpen(const std::string& host, int port) {
    // Platform-specific implementation would go here
    return false;
}

std::string NetworkUtils::getLocalIPAddress() {
    // Platform-specific implementation would go here
    return "127.0.0.1";
}

std::string NetworkUtils::getHostname() {
    // Platform-specific implementation would go here
    return "localhost";
}

bool NetworkUtils::isValidIPAddress(const std::string& ip) {
    std::regex ip_regex(R"(^([0-9]{1,3}\.){3}[0-9]{1,3}$)");
    return std::regex_match(ip, ip_regex);
}

std::vector<std::string> NetworkUtils::resolveHostname(const std::string& hostname) {
    // Platform-specific implementation would go here
    return {getLocalIPAddress()};
}

} // namespace Utils
} // namespace SwarmCog