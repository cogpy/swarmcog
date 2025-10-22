#pragma once

#include "types.h"
#include "agentspace.h"
#include "microkernel.h"
#include <future>

namespace SwarmCog {

/**
 * Agent Memory - Stores experiences and learned knowledge
 */
struct AgentMemory {
    std::string id;
    std::string type;  // "episodic", "semantic", "procedural"
    std::string content;
    double importance = 0.5;
    Timestamp created_at;
    Timestamp last_accessed;
    size_t access_count = 0;
    
    AgentMemory(const std::string& t, const std::string& c, double imp = 0.5)
        : type(t), content(c), importance(imp), 
          created_at(std::chrono::system_clock::now()), 
          last_accessed(created_at) {}
};

/**
 * Trust Relationship - Manages trust between agents
 */
struct TrustRelationship {
    AgentId target_agent;
    double trust_level = 0.5;  // [0.0, 1.0]
    double confidence = 0.0;   // [0.0, 1.0]
    size_t interaction_count = 0;
    Timestamp last_interaction;
    std::vector<double> trust_history;
    
    TrustRelationship() : last_interaction(std::chrono::system_clock::now()) {}
    TrustRelationship(const AgentId& target, double level = 0.5)
        : target_agent(target), trust_level(level), 
          last_interaction(std::chrono::system_clock::now()) {}
    
    void updateTrust(double new_level);
    double getTrustTrend() const;
};

/**
 * Collaboration Record - Tracks collaborative activities
 */
struct CollaborationRecord {
    AgentId partner_agent;
    std::string collaboration_type;
    std::string description;
    Timestamp start_time;
    Timestamp end_time;
    bool successful = false;
    double satisfaction = 0.5;
    std::map<std::string, std::string> outcomes;
    
    CollaborationRecord(const AgentId& partner, const std::string& type, const std::string& desc)
        : partner_agent(partner), collaboration_type(type), description(desc),
          start_time(std::chrono::system_clock::now()) {}
};

/**
 * Cognitive Agent - Enhanced agent with autonomous cognitive capabilities
 * 
 * Core features:
 * - Autonomous reasoning and decision making
 * - Trust-based relationship management  
 * - Knowledge sharing and learning
 * - Goal-oriented planning and coordination
 * - Experience-based adaptation
 */
class CognitiveAgent {
private:
    // Basic properties
    AgentId id_;
    std::string name_;
    std::string model_;
    std::string instructions_;
    
    // Cognitive components
    std::shared_ptr<AgentSpace> agentspace_;
    std::shared_ptr<CognitiveMicrokernel> microkernel_;
    NodePtr agent_node_;  // Agent's representation in AgentSpace
    
    // Cognitive capabilities
    std::unordered_map<std::string, CognitiveCapability> capabilities_;
    std::vector<std::string> goals_;
    std::map<std::string, std::string> beliefs_;
    
    // Memory and experience
    std::vector<AgentMemory> memories_;
    std::unordered_map<AgentId, TrustRelationship> trust_relationships_;
    std::vector<CollaborationRecord> collaboration_history_;
    
    // Agent functions
    std::vector<AgentFunction> functions_;
    std::map<std::string, std::string> function_schemas_;
    
    // State and status
    CognitiveState cognitive_state_;
    std::atomic<bool> is_active_{false};
    std::atomic<bool> cognitive_processing_enabled_{true};
    
    // Thread safety
    mutable std::shared_mutex agent_mutex_;
    mutable std::mutex memory_mutex_;
    mutable std::mutex trust_mutex_;

public:
    CognitiveAgent(const AgentId& id, const std::string& name = "",
                   std::shared_ptr<AgentSpace> agentspace = nullptr,
                   std::shared_ptr<CognitiveMicrokernel> microkernel = nullptr);
    
    ~CognitiveAgent();

    // Basic getters
    const AgentId& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getModel() const { return model_; }
    const std::string& getInstructions() const { return instructions_; }
    
    // Basic setters
    void setName(const std::string& name) { name_ = name; }
    void setModel(const std::string& model) { model_ = model; }
    void setInstructions(const std::string& instructions) { instructions_ = instructions; }
    
    // Capability management
    void addCapability(const std::string& name, const std::string& description, 
                      double strength = 0.5, int experience = 0);
    void removeCapability(const std::string& name);
    bool hasCapability(const std::string& name) const;
    CognitiveCapability getCapability(const std::string& name) const;
    std::vector<CognitiveCapability> getAllCapabilities() const;
    void updateCapabilityStrength(const std::string& name, double new_strength);
    void incrementCapabilityExperience(const std::string& name, int increment = 1);
    
    // Goal management
    void addGoal(const std::string& goal, double priority = 0.5);
    void removeGoal(const std::string& goal);
    std::vector<std::string> getGoals() const;
    void updateGoalPriority(const std::string& goal, double priority);
    
    // Belief management
    void updateBelief(const std::string& key, const std::string& value);
    std::string getBelief(const std::string& key) const;
    std::map<std::string, std::string> getAllBeliefs() const;
    void removeBelief(const std::string& key);
    
    // Memory management
    void addMemory(const std::string& type, const std::string& content, double importance = 0.5);
    std::vector<AgentMemory> getMemories(const std::string& type = "") const;
    std::vector<AgentMemory> getMostImportantMemories(size_t limit = 10) const;
    void forgetMemory(const std::string& memory_id);
    
    // Trust relationship management
    void establishTrust(const AgentId& target_agent, double trust_level);
    void updateTrust(const AgentId& target_agent, double new_level);
    double getTrustLevel(const AgentId& target_agent) const;
    std::vector<AgentId> getTrustedAgents(double min_trust = 0.5) const;
    std::unordered_map<AgentId, TrustRelationship> getAllTrustRelationships() const;
    
    // Collaboration
    void startCollaboration(const AgentId& partner_agent, const std::string& type, 
                           const std::string& description);
    void endCollaboration(const AgentId& partner_agent, bool successful, 
                         double satisfaction, const std::map<std::string, std::string>& outcomes = {});
    std::vector<CollaborationRecord> getCollaborationHistory() const;
    std::vector<AgentId> getFrequentCollaborators() const;
    
    // Cognitive functions
    void perceiveEnvironment();
    std::vector<std::string> reasonAboutGoals();
    std::vector<std::string> planCollaboration(const std::string& goal);
    std::vector<AgentId> findCollaborators(const std::string& capability_needed, 
                                          double min_trust = 0.3) const;
    void shareKnowledge(const std::string& knowledge_type, const std::string& content, 
                       const AgentId& target_agent = "");
    void coordinateWithAgent(const AgentId& target_agent, const std::string& coordination_type);
    
    // Function management
    void addFunction(const AgentFunction& function, const std::string& name, 
                     const std::string& schema = "");
    void removeFunction(const std::string& name);
    std::string callFunction(const std::string& name, 
                            const std::map<std::string, std::string>& parameters) const;
    std::vector<std::string> getFunctionNames() const;
    
    // State management
    CognitiveState getCognitiveState() const;
    void updateCognitiveState(const CognitiveState& state);
    bool isActive() const { return is_active_; }
    void setActive(bool active) { is_active_ = active; }
    
    // Processing control
    void enableCognitiveProcessing() { cognitive_processing_enabled_ = true; }
    void disableCognitiveProcessing() { cognitive_processing_enabled_ = false; }
    bool isCognitiveProcessingEnabled() const { return cognitive_processing_enabled_; }
    
    // Autonomous behavior
    std::future<void> startAutonomousProcessing();
    void stopAutonomousProcessing();
    
    // Interaction with other agents
    void sendMessage(const AgentId& target_agent, const std::string& message, 
                     const std::string& message_type = "general");
    std::vector<std::string> getMessages() const;
    void clearMessages();
    
    // Learning and adaptation
    void learnFromExperience(const CollaborationRecord& record);
    void adaptBehavior(const std::string& situation, const std::string& strategy);
    void improveCapabilities();
    
    // Serialization
    std::map<std::string, std::string> toDict() const;
    void fromDict(const std::map<std::string, std::string>& dict);

private:
    // Internal helper methods
    void initializeAgentNode();
    void registerWithMicrokernel();
    void updateAgentSpaceRepresentation();
    void processIncomingMessages();
    void autonomousDecisionMaking();
    double calculateTrustUpdate(const AgentId& target_agent, bool positive_interaction);
    void maintainMemorySystem();
    void pruneOldMemories();
    void updateCapabilitiesFromExperience();
    
    // Cognitive processing helpers
    void executePerceptionCycle();
    void executeReasoningCycle();
    void executePlanningCycle();
    void executeActionCycle();
    void executeLearningCycle();
    
    // Utility methods
    std::string generateMemoryId() const;
    double calculateMemoryImportance(const AgentMemory& memory) const;
    bool shouldForgetMemory(const AgentMemory& memory) const;
};

// Factory function for creating cognitive agents
std::shared_ptr<CognitiveAgent> createCognitiveAgent(
    const AgentId& id, 
    const std::string& name = "",
    const std::vector<std::string>& capabilities = {},
    const std::vector<std::string>& goals = {},
    const std::map<std::string, std::string>& initial_beliefs = {},
    std::shared_ptr<AgentSpace> agentspace = nullptr,
    std::shared_ptr<CognitiveMicrokernel> microkernel = nullptr
);

} // namespace SwarmCog