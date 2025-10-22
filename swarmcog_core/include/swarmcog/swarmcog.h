#pragma once

#include "types.h"
#include "agentspace.h"
#include "microkernel.h"
#include "cognitive_agent.h"

namespace SwarmCog {

/**
 * Multi-Agent Task - Coordinated task involving multiple agents
 */
struct MultiAgentTask {
    std::string id;
    std::string description;
    std::vector<AgentId> assigned_agents;
    std::string coordination_strategy;
    std::map<std::string, std::string> parameters;
    Timestamp created_at;
    Timestamp deadline;
    bool completed = false;
    std::map<std::string, std::string> results;
    
    MultiAgentTask() : created_at(std::chrono::system_clock::now()) {}
    MultiAgentTask(const std::string& desc) 
        : description(desc), created_at(std::chrono::system_clock::now()) {}
};

/**
 * Swarm Topology - Represents the structure of agent relationships
 */
struct SwarmTopology {
    std::map<AgentId, std::vector<AgentId>> connections;
    std::map<std::pair<AgentId, AgentId>, double> trust_levels;
    std::map<AgentId, std::vector<std::string>> agent_capabilities;
    size_t total_agents = 0;
    size_t total_connections = 0;
    double average_trust_level = 0.0;
    
    SwarmTopology() = default;
};

/**
 * Agent Interaction Record
 */
struct AgentInteraction {
    AgentId agent1;
    AgentId agent2;
    std::string interaction_type;
    std::string description;
    Timestamp timestamp;
    bool successful = true;
    std::map<std::string, std::string> metadata;
    
    AgentInteraction(const AgentId& a1, const AgentId& a2, const std::string& type)
        : agent1(a1), agent2(a2), interaction_type(type), 
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * System Status
 */
struct SystemStatus {
    bool is_running = false;
    size_t active_agents = 0;
    size_t total_interactions = 0;
    size_t completed_tasks = 0;
    Timestamp start_time;
    std::chrono::milliseconds uptime{0};
    std::map<std::string, size_t> phase_statistics;
    
    SystemStatus() : start_time(std::chrono::system_clock::now()) {}
};

/**
 * SwarmCog - Main orchestration system for cognitive multi-agent coordination
 * 
 * Integrates all components:
 * - AgentSpace for knowledge representation
 * - CognitiveMicrokernel for autonomous processing
 * - CognitiveAgents for multi-agent coordination
 * 
 * Provides high-level API for:
 * - Creating and managing cognitive agents
 * - Coordinating multi-agent tasks
 * - Monitoring system performance
 * - Autonomous swarm behavior
 */
class SwarmCog {
private:
    // Configuration
    SwarmCogConfig config_;
    
    // Core components
    std::shared_ptr<AgentSpace> agentspace_;
    std::shared_ptr<CognitiveMicrokernel> microkernel_;
    
    // Agent management
    std::unordered_map<AgentId, std::shared_ptr<CognitiveAgent>> cognitive_agents_;
    mutable std::shared_mutex agents_mutex_;
    
    // Task management
    std::unordered_map<std::string, MultiAgentTask> active_tasks_;
    std::vector<MultiAgentTask> completed_tasks_;
    mutable std::mutex tasks_mutex_;
    
    // Interaction tracking
    std::vector<AgentInteraction> interaction_history_;
    mutable std::mutex interactions_mutex_;
    
    // System state
    SystemStatus system_status_;
    std::atomic<bool> autonomous_mode_{false};
    std::thread autonomous_thread_;
    std::atomic<bool> shutdown_requested_{false};
    
    // Statistics
    ThreadSafeCounter task_counter_;
    ThreadSafeCounter interaction_counter_;

public:
    explicit SwarmCog(const SwarmCogConfig& config = SwarmCogConfig());
    ~SwarmCog();

    // System lifecycle
    void initialize();
    void shutdown();
    bool isInitialized() const;
    
    // Configuration
    const SwarmCogConfig& getConfig() const { return config_; }
    void updateConfig(const SwarmCogConfig& config);
    
    // Agent management
    std::shared_ptr<CognitiveAgent> createCognitiveAgent(
        const AgentId& id,
        const std::string& name = "",
        const std::string& model = "cognitive_v1",
        const std::string& instructions = "",
        const std::vector<std::string>& capabilities = {},
        const std::vector<std::string>& goals = {},
        const std::map<std::string, std::string>& initial_beliefs = {}
    );
    
    bool removeAgent(const AgentId& agent_id);
    std::shared_ptr<CognitiveAgent> getAgent(const AgentId& agent_id) const;
    std::vector<AgentId> listAgents() const;
    size_t getAgentCount() const;
    
    // Multi-agent task coordination
    std::string coordinateMultiAgentTask(
        const std::string& description,
        const std::vector<AgentId>& agents,
        const std::string& coordination_strategy = "collaborative"
    );
    
    bool cancelTask(const std::string& task_id);
    MultiAgentTask getTask(const std::string& task_id) const;
    std::vector<MultiAgentTask> getActiveTasks() const;
    std::vector<MultiAgentTask> getCompletedTasks() const;
    
    // Swarm topology and analysis
    SwarmTopology getSwarmTopology() const;
    std::vector<AgentInteraction> getAgentInteractions(
        const AgentId& agent_id = "", 
        const Timestamp& since = Timestamp{}
    ) const;
    
    std::vector<AgentId> findAgentsByCapability(const std::string& capability) const;
    std::vector<AgentId> getConnectedAgents(const AgentId& agent_id) const;
    double getSwarmCohesion() const;
    
    // System monitoring
    SystemStatus getSystemStatus() const;
    ProcessingStats getProcessingStats() const;
    std::map<std::string, size_t> getSystemStatistics() const;
    void resetStatistics();
    
    // Autonomous operation
    void startAutonomousProcessing();
    void stopAutonomousProcessing();
    bool isAutonomousModeActive() const { return autonomous_mode_; }
    
    // Simulation and testing
    std::future<std::map<std::string, std::string>> simulateAutonomousBehavior(
        std::chrono::seconds duration
    );
    
    void injectEvent(const std::string& event_type, 
                     const std::map<std::string, std::string>& event_data);
    
    // Knowledge and learning
    void shareKnowledgeGlobally(const std::string& knowledge_type, 
                               const std::string& content,
                               const AgentId& source_agent = "");
    
    void establishGlobalTrust(double base_trust_level = 0.5);
    void updateGlobalBeliefs(const std::map<std::string, std::string>& beliefs);
    
    // Advanced coordination
    void formCoalition(const std::vector<AgentId>& agents, 
                      const std::string& coalition_purpose);
    
    void dissolveCoalition(const std::vector<AgentId>& agents);
    
    std::vector<std::vector<AgentId>> detectCoalitions() const;
    
    // Performance optimization
    void optimizeAgentPlacement();
    void balanceWorkload();
    void pruneInactiveConnections();
    
    // Debugging and introspection
    std::string dumpSystemState() const;
    void enableDebugMode(bool enabled = true);
    std::vector<std::string> getDebugInfo() const;

private:
    // Internal system management
    void initializeComponents();
    void startSystemThreads();
    void stopSystemThreads();
    
    // Autonomous behavior implementation
    void autonomousProcessingLoop();
    void performSystemMaintenance();
    void optimizeSystemPerformance();
    void handleEmergentBehavior();
    
    // Task coordination helpers
    std::string generateTaskId();
    void assignTaskToAgents(MultiAgentTask& task);
    void monitorTaskProgress(const std::string& task_id);
    void completeTask(const std::string& task_id, bool successful);
    
    // Interaction tracking
    void recordInteraction(const AgentId& agent1, const AgentId& agent2, 
                          const std::string& type, bool successful = true);
    void analyzeInteractionPatterns();
    
    // Topology analysis
    void updateTopologyCache();
    void analyzeSwarmStructure();
    void detectEmergentPatterns();
    
    // System optimization
    void rebalanceAgentLoad();
    void optimizeCommunicationPaths();
    void adjustProcessingParameters();
    
    // Utilities
    void updateSystemStatus();
    void logSystemEvent(const std::string& event, const std::string& details = "");
    std::chrono::milliseconds calculateUptime() const;
};

// Factory function for creating SwarmCog systems
std::shared_ptr<SwarmCog> createSwarmCog(
    const std::string& agentspace_name = "default_swarm",
    ProcessingMode processing_mode = ProcessingMode::ASYNCHRONOUS,
    size_t max_agents = 50
);

// Utility functions for swarm analysis
double calculateNetworkDensity(const SwarmTopology& topology);
std::vector<AgentId> findCentralAgents(const SwarmTopology& topology, size_t limit = 5);
std::map<std::string, double> analyzeCapabilityDistribution(const SwarmTopology& topology);

} // namespace SwarmCog