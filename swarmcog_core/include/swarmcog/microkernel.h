#pragma once

#include "types.h"
#include "agentspace.h"
#include <queue>
#include <condition_variable>
#include <future>

namespace SwarmCog {

/**
 * Cognitive Task - represents a unit of cognitive work
 */
struct CognitiveTask {
    std::string id;
    AgentId agent_id;
    CognitivePhase phase;
    std::string description;
    std::map<std::string, std::string> parameters;
    std::function<void()> execution_function;
    Timestamp created_at;
    Timestamp scheduled_at;
    int priority = 0;
    
    CognitiveTask() : created_at(std::chrono::system_clock::now()), scheduled_at(created_at) {}
    
    // Comparison operator for priority queue (higher priority first)
    bool operator<(const CognitiveTask& other) const {
        return priority < other.priority; // Note: reversed for max-heap behavior
    }
};

/**
 * Cognitive Context - maintains state during processing
 */
struct CognitiveContext {
    AgentId agent_id;
    std::map<std::string, std::string> variables;
    std::vector<std::string> focus_atoms;
    std::chrono::milliseconds processing_time{0};
    
    CognitiveContext(const AgentId& id) : agent_id(id) {}
};

/**
 * Processing Statistics
 */
struct ProcessingStats {
    std::atomic<size_t> total_cycles{0};
    std::atomic<size_t> completed_tasks{0};
    std::atomic<size_t> failed_tasks{0};
    std::chrono::milliseconds total_processing_time{0};
    std::chrono::system_clock::time_point start_time;
    
    ProcessingStats() : start_time(std::chrono::system_clock::now()) {}
    
    // Copy constructor - needed for returning by value
    ProcessingStats(const ProcessingStats& other)
        : total_cycles(other.total_cycles.load()),
          completed_tasks(other.completed_tasks.load()),
          failed_tasks(other.failed_tasks.load()),
          total_processing_time(other.total_processing_time),
          start_time(other.start_time) {}
    
    // Assignment operator
    ProcessingStats& operator=(const ProcessingStats& other) {
        if (this != &other) {
            total_cycles.store(other.total_cycles.load());
            completed_tasks.store(other.completed_tasks.load());
            failed_tasks.store(other.failed_tasks.load());
            total_processing_time = other.total_processing_time;
            start_time = other.start_time;
        }
        return *this;
    }
};

/**
 * Cognitive Microkernel - Core cognitive processing engine
 * 
 * Implements 7-phase cognitive cycle:
 * 1. Perception - Gather information from environment
 * 2. Attention - Focus on relevant information  
 * 3. Reasoning - Process and analyze information
 * 4. Planning - Create action plans
 * 5. Execution - Execute planned actions
 * 6. Learning - Update knowledge from outcomes
 * 7. Reflection - Evaluate performance and adjust
 */
class CognitiveMicrokernel {
private:
    // Core components
    std::shared_ptr<AgentSpace> agentspace_;
    ProcessingMode processing_mode_;
    double cycle_interval_; // seconds
    
    // Agent management
    std::unordered_map<AgentId, CognitiveState> agent_states_;
    std::unordered_map<AgentId, std::vector<CognitiveCallback>> agent_callbacks_;
    mutable std::shared_mutex agents_mutex_;
    
    // Task processing
    std::priority_queue<CognitiveTask> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    // Processing threads
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_{false};
    size_t num_workers_;
    
    // Statistics and monitoring
    ProcessingStats stats_;
    mutable std::mutex stats_mutex_;

public:
    explicit CognitiveMicrokernel(std::shared_ptr<AgentSpace> agentspace, 
                                  ProcessingMode mode = ProcessingMode::ASYNCHRONOUS,
                                  size_t num_workers = std::thread::hardware_concurrency());
    
    ~CognitiveMicrokernel();

    // Agent lifecycle
    CognitiveState addCognitiveAgent(const AgentId& agent_id, 
                                   const std::vector<std::string>& goals = {},
                                   const std::map<std::string, std::string>& beliefs = {});
    
    bool removeCognitiveAgent(const AgentId& agent_id);
    bool hasAgent(const AgentId& agent_id) const;
    std::vector<AgentId> getActiveAgents() const;
    
    // State management
    CognitiveState getCognitiveState(const AgentId& agent_id) const;
    bool updateCognitiveState(const AgentId& agent_id, const CognitiveState& state);
    void addGoal(const AgentId& agent_id, const std::string& goal);
    void updateBelief(const AgentId& agent_id, const std::string& key, const std::string& value);
    
    // Processing control
    void start();
    void stop();
    bool isRunning() const { return running_; }
    
    // Task scheduling
    void scheduleTask(const CognitiveTask& task);
    void scheduleCognitivePhase(const AgentId& agent_id, CognitivePhase phase, 
                               const std::map<std::string, std::string>& parameters = {});
    
    // Cognitive cycle processing - 7 phases
    void processPerceptionPhase(const AgentId& agent_id, CognitiveContext& context);
    void processAttentionPhase(const AgentId& agent_id, CognitiveContext& context);
    void processReasoningPhase(const AgentId& agent_id, CognitiveContext& context);
    void processPlanningPhase(const AgentId& agent_id, CognitiveContext& context);
    void processExecutionPhase(const AgentId& agent_id, CognitiveContext& context);
    void processLearningPhase(const AgentId& agent_id, CognitiveContext& context);
    void processReflectionPhase(const AgentId& agent_id, CognitiveContext& context);
    
    // Full cognitive cycle
    void runCognitiveCycle(const AgentId& agent_id);
    void runAllAgentsCycles();
    
    // Callback management
    void registerCallback(const AgentId& agent_id, const CognitiveCallback& callback);
    void unregisterCallbacks(const AgentId& agent_id);
    
    // Monitoring and statistics
    ProcessingStats getProcessingStats() const;
    void resetStats();
    std::map<std::string, std::string> getSystemStatus() const;
    
    // Configuration
    void setProcessingMode(ProcessingMode mode);
    ProcessingMode getProcessingMode() const { return processing_mode_; }
    void setCycleInterval(double seconds) { cycle_interval_ = seconds; }
    double getCycleInterval() const { return cycle_interval_; }

private:
    // Internal processing methods
    void workerThread();
    void processTask(const CognitiveTask& task);
    void executePhaseFunction(const AgentId& agent_id, CognitivePhase phase, CognitiveContext& context);
    void notifyCallbacks(const AgentId& agent_id, const CognitiveState& state);
    
    // Phase implementation helpers
    void gatherEnvironmentalData(const AgentId& agent_id, CognitiveContext& context);
    void selectAttentionalFocus(const AgentId& agent_id, CognitiveContext& context);
    void performReasoning(const AgentId& agent_id, CognitiveContext& context);
    void createActionPlans(const AgentId& agent_id, CognitiveContext& context);
    void executeActions(const AgentId& agent_id, CognitiveContext& context);
    void updateKnowledge(const AgentId& agent_id, CognitiveContext& context);
    void evaluatePerformance(const AgentId& agent_id, CognitiveContext& context);
    
    // Utilities
    std::string generateTaskId() const;
    void updateStats(const CognitiveTask& task, bool success, std::chrono::milliseconds duration);
};

/**
 * Cognitive Phase Processor - Interface for custom phase implementations
 */
class CognitivePhaseProcessor {
public:
    virtual ~CognitivePhaseProcessor() = default;
    virtual void process(const AgentId& agent_id, CognitiveContext& context, 
                        std::shared_ptr<AgentSpace> agentspace) = 0;
    virtual CognitivePhase getPhaseType() const = 0;
};

/**
 * Phase Factory - Creates processors for different cognitive phases
 */
class CognitivePhaseFactory {
private:
    std::unordered_map<CognitivePhase, std::unique_ptr<CognitivePhaseProcessor>> processors_;

public:
    void registerProcessor(std::unique_ptr<CognitivePhaseProcessor> processor);
    CognitivePhaseProcessor* getProcessor(CognitivePhase phase) const;
    bool hasProcessor(CognitivePhase phase) const;
};

} // namespace SwarmCog