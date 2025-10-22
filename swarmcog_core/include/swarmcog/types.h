#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <algorithm>

namespace SwarmCog {

// Forward declarations
class Atom;
class Node;
class Link;
class CognitiveAgent;
class AgentSpace;

// Basic type aliases
using AtomId = std::string;
using AgentId = std::string;
using Timestamp = std::chrono::system_clock::time_point;

// Enumerations
enum class AtomType {
    NODE,
    LINK,
    AGENT_NODE,
    CAPABILITY_NODE,
    GOAL_NODE,
    BELIEF_NODE,
    MEMORY_NODE,
    COLLABORATION_LINK,
    DELEGATION_LINK,
    TRUST_LINK,
    KNOWLEDGE_LINK,
    EVALUATION_LINK
};

enum class CognitivePhase {
    PERCEPTION,
    ATTENTION,
    REASONING,
    PLANNING,
    EXECUTION,
    LEARNING,
    REFLECTION
};

enum class ProcessingMode {
    SYNCHRONOUS,
    ASYNCHRONOUS,
    DISTRIBUTED
};

// Core data structures
struct TruthValue {
    double strength = 0.5;    // [0.0, 1.0]
    double confidence = 0.0;  // [0.0, 1.0]
    
    TruthValue() = default;
    TruthValue(double s, double c) : strength(std::clamp(s, 0.0, 1.0)), confidence(std::clamp(c, 0.0, 1.0)) {}
    
    bool operator==(const TruthValue& other) const {
        return std::abs(strength - other.strength) < 1e-6 && std::abs(confidence - other.confidence) < 1e-6;
    }
};

struct AttentionValue {
    double sti = 0.0;   // Short-term importance [-1.0, 1.0]
    double lti = 0.0;   // Long-term importance [-1.0, 1.0]
    double vlti = 0.0;  // Very long-term importance [0.0, 1.0]
    
    AttentionValue() = default;
    AttentionValue(double s, double l, double v) 
        : sti(std::clamp(s, -1.0, 1.0)), lti(std::clamp(l, -1.0, 1.0)), vlti(std::clamp(v, 0.0, 1.0)) {}
};

struct CognitiveCapability {
    std::string name;
    std::string description;
    double strength = 0.5;  // [0.0, 1.0]
    int experience = 0;
    
    CognitiveCapability() = default;
    CognitiveCapability(const std::string& n, const std::string& desc, double s = 0.5, int exp = 0)
        : name(n), description(desc), strength(std::clamp(s, 0.0, 1.0)), experience(exp) {}
};

struct CognitiveState {
    AgentId agent_id;
    CognitivePhase current_phase = CognitivePhase::PERCEPTION;
    std::vector<std::string> goals;
    std::map<std::string, std::string> beliefs;
    std::vector<std::string> intentions;
    std::vector<std::string> current_focus;
    Timestamp last_update;
    
    CognitiveState() : last_update(std::chrono::system_clock::now()) {}
    CognitiveState(const AgentId& id) : agent_id(id), last_update(std::chrono::system_clock::now()) {}
};

// Configuration structures
struct SwarmCogConfig {
    ProcessingMode processing_mode = ProcessingMode::ASYNCHRONOUS;
    double cognitive_cycle_interval = 1.0;  // seconds
    size_t max_agents = 50;
    bool enable_distributed_processing = false;
    std::string log_level = "INFO";
    std::string agentspace_name = "swarmcog_space";
    
    SwarmCogConfig() = default;
};

// Callback and function types
using CognitiveCallback = std::function<void(const CognitiveState&)>;
using AgentFunction = std::function<std::string(const std::map<std::string, std::string>&)>;

// Thread safety utilities
class ThreadSafeCounter {
private:
    std::atomic<size_t> count_{0};
public:
    size_t increment() { return ++count_; }
    size_t get() const { return count_; }
    void reset() { count_ = 0; }
};

// Smart pointer aliases
using AtomPtr = std::shared_ptr<Atom>;
using NodePtr = std::shared_ptr<Node>;
using LinkPtr = std::shared_ptr<Link>;
using AgentPtr = std::shared_ptr<CognitiveAgent>;

} // namespace SwarmCog