#include "swarmcog/cognitive_agent.h"
#include "swarmcog/utils.h"

namespace SwarmCog {

// TrustRelationship implementation
void TrustRelationship::updateTrust(double new_level) {
    trust_history.push_back(trust_level);
    trust_level = Utils::MathUtils::clamp(new_level, 0.0, 1.0);
    interaction_count++;
    last_interaction = Utils::TimeUtils::now();
    
    // Update confidence based on interaction history
    confidence = std::min(1.0, confidence + 0.1);
}

double TrustRelationship::getTrustTrend() const {
    if (trust_history.size() < 2) return 0.0;
    
    double recent_avg = 0.0;
    double older_avg = 0.0;
    size_t half = trust_history.size() / 2;
    
    for (size_t i = 0; i < half; ++i) {
        older_avg += trust_history[i];
    }
    older_avg /= half;
    
    for (size_t i = half; i < trust_history.size(); ++i) {
        recent_avg += trust_history[i];
    }
    recent_avg /= (trust_history.size() - half);
    
    return recent_avg - older_avg;
}

// CognitiveAgent implementation
CognitiveAgent::CognitiveAgent(const AgentId& id, const std::string& name,
                              std::shared_ptr<AgentSpace> agentspace,
                              std::shared_ptr<CognitiveMicrokernel> microkernel)
    : id_(id), name_(name.empty() ? id : name), model_("cognitive_v1"),
      instructions_("You are a cognitive agent capable of autonomous reasoning and collaboration."),
      agentspace_(agentspace), microkernel_(microkernel), cognitive_state_(id) {
    
    if (!agentspace_) {
        agentspace_ = std::make_shared<AgentSpace>(name_ + "_space");
    }
    
    if (!microkernel_) {
        microkernel_ = std::make_shared<CognitiveMicrokernel>(agentspace_);
    }
    
    initializeAgentNode();
    registerWithMicrokernel();
    
    Utils::Logger::info("Created CognitiveAgent: " + name_);
}

CognitiveAgent::~CognitiveAgent() {
    if (is_active_) {
        setActive(false);
    }
}

void CognitiveAgent::addCapability(const std::string& name, const std::string& description, 
                                  double strength, int experience) {
    std::unique_lock<std::shared_mutex> lock(agent_mutex_);
    
    if (Utils::ValidationUtils::isValidCapabilityName(name)) {
        capabilities_[name] = CognitiveCapability(name, description, strength, experience);
        updateAgentSpaceRepresentation();
        
        Utils::Logger::debug("Added capability '" + name + "' to agent: " + name_);
    }
}

void CognitiveAgent::removeCapability(const std::string& name) {
    std::unique_lock<std::shared_mutex> lock(agent_mutex_);
    
    capabilities_.erase(name);
    updateAgentSpaceRepresentation();
}

bool CognitiveAgent::hasCapability(const std::string& name) const {
    std::shared_lock<std::shared_mutex> lock(agent_mutex_);
    return capabilities_.find(name) != capabilities_.end();
}

CognitiveCapability CognitiveAgent::getCapability(const std::string& name) const {
    std::shared_lock<std::shared_mutex> lock(agent_mutex_);
    
    auto it = capabilities_.find(name);
    return (it != capabilities_.end()) ? it->second : CognitiveCapability();
}

std::vector<CognitiveCapability> CognitiveAgent::getAllCapabilities() const {
    std::shared_lock<std::shared_mutex> lock(agent_mutex_);
    
    std::vector<CognitiveCapability> result;
    for (const auto& pair : capabilities_) {
        result.push_back(pair.second);
    }
    
    return result;
}

void CognitiveAgent::addGoal(const std::string& goal, double priority) {
    std::unique_lock<std::shared_mutex> lock(agent_mutex_);
    
    if (std::find(goals_.begin(), goals_.end(), goal) == goals_.end()) {
        goals_.push_back(goal);
        
        // Add goal to AgentSpace
        auto goal_node = agentspace_->addGoalNode(goal, priority);
        
        // Update microkernel
        if (microkernel_) {
            microkernel_->addGoal(id_, goal);
        }
        
        Utils::Logger::debug("Added goal '" + goal + "' to agent: " + name_);
    }
}

std::vector<std::string> CognitiveAgent::getGoals() const {
    std::shared_lock<std::shared_mutex> lock(agent_mutex_);
    return goals_;
}

void CognitiveAgent::updateBelief(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(agent_mutex_);
    beliefs_[key] = value;
    
    if (microkernel_) {
        microkernel_->updateBelief(id_, key, value);
    }
}

std::unordered_map<AgentId, TrustRelationship> CognitiveAgent::getAllTrustRelationships() const {
    std::lock_guard<std::mutex> lock(trust_mutex_);
    return trust_relationships_;
}

void CognitiveAgent::establishTrust(const AgentId& target_agent, double trust_level) {
    std::lock_guard<std::mutex> lock(trust_mutex_);
    
    if (Utils::ValidationUtils::isValidTrustLevel(trust_level)) {
        trust_relationships_[target_agent] = TrustRelationship(target_agent, trust_level);
        
        // Add trust link to AgentSpace
        agentspace_->addTrustRelationship(id_, target_agent, trust_level);
        
        Utils::Logger::info("Established trust with " + target_agent + " at level " + 
                           std::to_string(trust_level));
    }
}

double CognitiveAgent::getTrustLevel(const AgentId& target_agent) const {
    std::lock_guard<std::mutex> lock(trust_mutex_);
    
    auto it = trust_relationships_.find(target_agent);
    return (it != trust_relationships_.end()) ? it->second.trust_level : 0.0;
}

std::vector<AgentId> CognitiveAgent::findCollaborators(const std::string& capability_needed, 
                                                      double min_trust) const {
    std::vector<AgentId> collaborators;
    
    // Get agents from AgentSpace
    auto agent_atoms = agentspace_->getAtomsByType(AtomType::AGENT_NODE);
    
    for (const auto& atom : agent_atoms) {
        auto node = std::dynamic_pointer_cast<Node>(atom);
        if (!node || node->getId() == id_) continue;
        
        // Check if agent has required capability
        std::string caps = node->getMetadata("capabilities");
        auto cap_list = Utils::StringUtils::split(caps, ',');
        
        bool has_capability = false;
        for (const auto& cap : cap_list) {
            if (Utils::StringUtils::trim(cap) == capability_needed) {
                has_capability = true;
                break;
            }
        }
        
        if (has_capability) {
            // Check trust level
            double trust = getTrustLevel(node->getId());
            if (trust >= min_trust) {
                collaborators.push_back(node->getId());
            }
        }
    }
    
    return collaborators;
}

void CognitiveAgent::shareKnowledge(const std::string& knowledge_type, const std::string& content, 
                                   const AgentId& target_agent) {
    // Add knowledge to AgentSpace
    auto memory_node = agentspace_->addMemoryNode(content, knowledge_type);
    
    if (!target_agent.empty()) {
        // Create knowledge link to specific agent
        auto target_atom = agentspace_->getAtom(target_agent);
        if (target_atom) {
            agentspace_->addKnowledgeLink(memory_node->getId(), target_atom->getId(), "shared_knowledge");
        }
    }
    
    Utils::Logger::info("Shared knowledge: " + knowledge_type + " from agent: " + name_);
}

CognitiveState CognitiveAgent::getCognitiveState() const {
    std::shared_lock<std::shared_mutex> lock(agent_mutex_);
    return cognitive_state_;
}

void CognitiveAgent::updateCognitiveState(const CognitiveState& state) {
    std::unique_lock<std::shared_mutex> lock(agent_mutex_);
    cognitive_state_ = state;
    
    if (microkernel_) {
        microkernel_->updateCognitiveState(id_, state);
    }
}

std::future<void> CognitiveAgent::startAutonomousProcessing() {
    setActive(true);
    
    return std::async(std::launch::async, [this]() {
        while (is_active_ && cognitive_processing_enabled_) {
            try {
                autonomousDecisionMaking();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } catch (const std::exception& e) {
                Utils::Logger::error("Autonomous processing error for agent " + name_ + ": " + e.what());
            }
        }
    });
}

void CognitiveAgent::stopAutonomousProcessing() {
    setActive(false);
}

std::map<std::string, std::string> CognitiveAgent::toDict() const {
    std::shared_lock<std::shared_mutex> lock(agent_mutex_);
    
    std::map<std::string, std::string> result;
    result["id"] = id_;
    result["name"] = name_;
    result["model"] = model_;
    result["instructions"] = instructions_;
    result["is_active"] = is_active_ ? "true" : "false";
    result["cognitive_processing_enabled"] = cognitive_processing_enabled_ ? "true" : "false";
    result["goals"] = Utils::StringUtils::join(goals_, ",");
    
    // Serialize capabilities
    std::vector<std::string> cap_names;
    for (const auto& pair : capabilities_) {
        cap_names.push_back(pair.first);
    }
    result["capabilities"] = Utils::StringUtils::join(cap_names, ",");
    
    return result;
}

// Private methods
void CognitiveAgent::initializeAgentNode() {
    std::vector<std::string> capability_names;
    for (const auto& pair : capabilities_) {
        capability_names.push_back(pair.first);
    }
    
    agent_node_ = agentspace_->addAgentNode(name_, capability_names);
}

void CognitiveAgent::registerWithMicrokernel() {
    if (microkernel_) {
        cognitive_state_ = microkernel_->addCognitiveAgent(id_, goals_, beliefs_);
    }
}

void CognitiveAgent::updateAgentSpaceRepresentation() {
    if (agent_node_) {
        // Update capabilities metadata
        std::vector<std::string> cap_names;
        for (const auto& pair : capabilities_) {
            cap_names.push_back(pair.first);
        }
        
        agent_node_->setMetadata("capabilities", Utils::StringUtils::join(cap_names, ","));
        agent_node_->setMetadata("last_updated", Utils::TimeUtils::timestampToString(Utils::TimeUtils::now()));
    }
}

void CognitiveAgent::autonomousDecisionMaking() {
    // Run cognitive cycle through microkernel
    if (microkernel_ && cognitive_processing_enabled_) {
        microkernel_->runCognitiveCycle(id_);
    }
}

std::string CognitiveAgent::generateMemoryId() const {
    return "mem_" + Utils::UUIDGenerator::generateShort();
}

// Factory function
std::shared_ptr<CognitiveAgent> createCognitiveAgent(
    const AgentId& id, 
    const std::string& name,
    const std::vector<std::string>& capabilities,
    const std::vector<std::string>& goals,
    const std::map<std::string, std::string>& initial_beliefs,
    std::shared_ptr<AgentSpace> agentspace,
    std::shared_ptr<CognitiveMicrokernel> microkernel) {
    
    auto agent = std::make_shared<CognitiveAgent>(id, name, agentspace, microkernel);
    
    // Add capabilities
    for (const auto& capability : capabilities) {
        agent->addCapability(capability, "Default capability");
    }
    
    // Add goals  
    for (const auto& goal : goals) {
        agent->addGoal(goal);
    }
    
    // Add beliefs
    for (const auto& belief : initial_beliefs) {
        agent->updateBelief(belief.first, belief.second);
    }
    
    return agent;
}

} // namespace SwarmCog