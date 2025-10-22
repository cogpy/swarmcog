#include "swarmcog/swarmcog.h"
#include "swarmcog/utils.h"
#include <algorithm>

namespace SwarmCog {

// SwarmCog Implementation
SwarmCog::SwarmCog(const SwarmCogConfig& config) : config_(config) {
    initialize();
}

SwarmCog::~SwarmCog() {
    shutdown();
}

void SwarmCog::initialize() {
    Utils::Logger::info("Initializing SwarmCog system: " + config_.agentspace_name);
    
    // Initialize core components
    agentspace_ = std::make_shared<AgentSpace>(config_.agentspace_name);
    microkernel_ = std::make_shared<CognitiveMicrokernel>(agentspace_, config_.processing_mode);
    
    // Initialize system status
    system_status_.start_time = Utils::TimeUtils::now();
    
    Utils::Logger::info("SwarmCog system initialized successfully");
}

void SwarmCog::shutdown() {
    if (autonomous_mode_) {
        stopAutonomousProcessing();
    }
    
    // Stop microkernel
    if (microkernel_) {
        microkernel_->stop();
    }
    
    // Clear agents
    {
        std::unique_lock<std::shared_mutex> lock(agents_mutex_);
        cognitive_agents_.clear();
    }
    
    Utils::Logger::info("SwarmCog system shut down");
}

bool SwarmCog::isInitialized() const {
    return agentspace_ && microkernel_;
}

void SwarmCog::updateConfig(const SwarmCogConfig& config) {
    config_ = config;
    
    if (microkernel_) {
        microkernel_->setProcessingMode(config_.processing_mode);
        microkernel_->setCycleInterval(config_.cognitive_cycle_interval);
    }
    
    Utils::Logger::info("SwarmCog configuration updated");
}

std::shared_ptr<CognitiveAgent> SwarmCog::createCognitiveAgent(
    const AgentId& id,
    const std::string& name,
    const std::string& model,
    const std::string& instructions,
    const std::vector<std::string>& capabilities,
    const std::vector<std::string>& goals,
    const std::map<std::string, std::string>& initial_beliefs) {
    
    std::unique_lock<std::shared_mutex> lock(agents_mutex_);
    
    // Check if agent already exists
    if (cognitive_agents_.find(id) != cognitive_agents_.end()) {
        Utils::Logger::warning("Agent already exists: " + id);
        return cognitive_agents_[id];
    }
    
    // Check agent limit
    if (cognitive_agents_.size() >= config_.max_agents) {
        Utils::Logger::error("Maximum number of agents reached: " + std::to_string(config_.max_agents));
        return nullptr;
    }
    
    // Create agent
    auto agent = ::SwarmCog::createCognitiveAgent(
        id, name.empty() ? id : name, capabilities, goals, initial_beliefs,
        agentspace_, microkernel_
    );
    
    if (agent) {
        agent->setModel(model);
        if (!instructions.empty()) {
            agent->setInstructions(instructions);
        }
        
        cognitive_agents_[id] = agent;
        system_status_.active_agents = cognitive_agents_.size();
        
        Utils::Logger::info("Created cognitive agent: " + id);
        
        // Record agent creation interaction
        recordInteraction("system", id, "agent_created", true);
    }
    
    return agent;
}

bool SwarmCog::removeAgent(const AgentId& agent_id) {
    std::unique_lock<std::shared_mutex> lock(agents_mutex_);
    
    auto it = cognitive_agents_.find(agent_id);
    if (it == cognitive_agents_.end()) {
        return false;
    }
    
    // Stop agent if active
    auto agent = it->second;
    if (agent->isActive()) {
        agent->stopAutonomousProcessing();
    }
    
    // Remove from microkernel
    if (microkernel_) {
        microkernel_->removeCognitiveAgent(agent_id);
    }
    
    cognitive_agents_.erase(it);
    system_status_.active_agents = cognitive_agents_.size();
    
    Utils::Logger::info("Removed agent: " + agent_id);
    return true;
}

std::shared_ptr<CognitiveAgent> SwarmCog::getAgent(const AgentId& agent_id) const {
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    
    auto it = cognitive_agents_.find(agent_id);
    return (it != cognitive_agents_.end()) ? it->second : nullptr;
}

std::vector<AgentId> SwarmCog::listAgents() const {
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    
    std::vector<AgentId> agents;
    for (const auto& pair : cognitive_agents_) {
        agents.push_back(pair.first);
    }
    
    return agents;
}

size_t SwarmCog::getAgentCount() const {
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    return cognitive_agents_.size();
}

std::string SwarmCog::coordinateMultiAgentTask(
    const std::string& description,
    const std::vector<AgentId>& agents,
    const std::string& coordination_strategy) {
    
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    // Create task
    MultiAgentTask task(description);
    task.id = generateTaskId();
    task.assigned_agents = agents;
    task.coordination_strategy = coordination_strategy;
    task.created_at = Utils::TimeUtils::now();
    
    // Assign task to agents
    assignTaskToAgents(task);
    
    active_tasks_[task.id] = task;
    
    Utils::Logger::info("Created multi-agent task: " + task.id + " with " + 
                       std::to_string(agents.size()) + " agents");
    
    return task.id;
}

SwarmTopology SwarmCog::getSwarmTopology() const {
    SwarmTopology topology;
    
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    
    topology.total_agents = cognitive_agents_.size();
    
    // Build connections based on trust relationships
    for (const auto& agent_pair : cognitive_agents_) {
        const auto& agent_id = agent_pair.first;
        const auto& agent = agent_pair.second;
        
        auto trust_relationships = agent->getAllTrustRelationships();
        
        for (const auto& trust_pair : trust_relationships) {
            const auto& target_id = trust_pair.first;
            double trust_level = trust_pair.second.trust_level;
            
            topology.connections[agent_id].push_back(target_id);
            topology.trust_levels[{agent_id, target_id}] = trust_level;
            topology.total_connections++;
        }
        
        // Add capabilities
        auto capabilities = agent->getAllCapabilities();
        for (const auto& cap : capabilities) {
            topology.agent_capabilities[agent_id].push_back(cap.name);
        }
    }
    
    // Calculate average trust level
    double trust_sum = 0.0;
    for (const auto& trust_pair : topology.trust_levels) {
        trust_sum += trust_pair.second;
    }
    
    if (!topology.trust_levels.empty()) {
        topology.average_trust_level = trust_sum / topology.trust_levels.size();
    }
    
    return topology;
}

SystemStatus SwarmCog::getSystemStatus() const {
    SystemStatus status = system_status_;
    
    status.is_running = microkernel_ ? microkernel_->isRunning() : false;
    status.active_agents = getAgentCount();
    
    {
        std::lock_guard<std::mutex> lock(interactions_mutex_);
        status.total_interactions = interaction_history_.size();
    }
    
    {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        status.completed_tasks = completed_tasks_.size();
    }
    
    status.uptime = calculateUptime();
    
    // Get processing statistics from microkernel
    if (microkernel_) {
        auto proc_stats = microkernel_->getProcessingStats();
        status.phase_statistics["total_cycles"] = proc_stats.total_cycles.load();
        status.phase_statistics["completed_tasks"] = proc_stats.completed_tasks.load();
        status.phase_statistics["failed_tasks"] = proc_stats.failed_tasks.load();
    }
    
    return status;
}

void SwarmCog::startAutonomousProcessing() {
    if (autonomous_mode_) {
        Utils::Logger::warning("Autonomous processing already active");
        return;
    }
    
    autonomous_mode_ = true;
    
    // Start microkernel
    if (microkernel_) {
        microkernel_->start();
    }
    
    // Start autonomous thread
    autonomous_thread_ = std::thread(&SwarmCog::autonomousProcessingLoop, this);
    
    Utils::Logger::info("Started autonomous processing");
}

void SwarmCog::stopAutonomousProcessing() {
    if (!autonomous_mode_) {
        return;
    }
    
    autonomous_mode_ = false;
    
    // Stop microkernel
    if (microkernel_) {
        microkernel_->stop();
    }
    
    // Wait for autonomous thread to complete
    if (autonomous_thread_.joinable()) {
        autonomous_thread_.join();
    }
    
    Utils::Logger::info("Stopped autonomous processing");
}

std::vector<AgentId> SwarmCog::findAgentsByCapability(const std::string& capability) const {
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    
    std::vector<AgentId> matching_agents;
    
    for (const auto& pair : cognitive_agents_) {
        if (pair.second->hasCapability(capability)) {
            matching_agents.push_back(pair.first);
        }
    }
    
    return matching_agents;
}

void SwarmCog::shareKnowledgeGlobally(const std::string& knowledge_type, 
                                     const std::string& content,
                                     const AgentId& source_agent) {
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    
    // Share knowledge with all agents
    for (const auto& pair : cognitive_agents_) {
        if (pair.first != source_agent) {
            pair.second->shareKnowledge(knowledge_type, content);
        }
    }
    
    Utils::Logger::info("Shared knowledge globally: " + knowledge_type);
}

std::map<std::string, size_t> SwarmCog::getSystemStatistics() const {
    std::map<std::string, size_t> stats;
    
    stats["active_agents"] = getAgentCount();
    stats["total_interactions"] = interaction_counter_.get();
    stats["completed_tasks"] = task_counter_.get();
    
    if (agentspace_) {
        auto agentspace_stats = agentspace_->getStatistics();
        for (const auto& pair : agentspace_stats) {
            stats["agentspace_" + pair.first] = pair.second;
        }
    }
    
    return stats;
}

// Private methods
void SwarmCog::autonomousProcessingLoop() {
    Utils::ThreadUtils::setThreadName("SwarmCog Autonomous");
    
    while (autonomous_mode_ && !shutdown_requested_) {
        try {
            // Run cognitive cycles for all agents
            if (microkernel_) {
                microkernel_->runAllAgentsCycles();
            }
            
            // Perform system maintenance
            performSystemMaintenance();
            
            // Update system status
            updateSystemStatus();
            
            // Sleep for cycle interval
            std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<int>(config_.cognitive_cycle_interval * 1000))
            );
            
        } catch (const std::exception& e) {
            Utils::Logger::error("Autonomous processing error: " + std::string(e.what()));
        }
    }
}

void SwarmCog::performSystemMaintenance() {
    // Update attention values in AgentSpace
    if (agentspace_) {
        agentspace_->updateAttentionValues();
    }
    
    // Monitor task progress
    monitorTaskProgress("");
    
    // Analyze interaction patterns
    analyzeInteractionPatterns();
}

std::string SwarmCog::generateTaskId() {
    return "task_" + Utils::UUIDGenerator::generateShort();
}

void SwarmCog::assignTaskToAgents(MultiAgentTask& task) {
    // Simple assignment - in practice would be more sophisticated
    for (const auto& agent_id : task.assigned_agents) {
        auto agent = getAgent(agent_id);
        if (agent) {
            agent->addGoal("complete_task_" + task.id);
        }
    }
}

void SwarmCog::monitorTaskProgress(const std::string& task_id) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    // Simple monitoring - check if agents have completed their goals
    for (auto& task_pair : active_tasks_) {
        auto& task = task_pair.second;
        
        bool all_complete = true;
        for (const auto& agent_id : task.assigned_agents) {
            auto agent = getAgent(agent_id);
            if (!agent) {
                all_complete = false;
                break;
            }
            
            // Check if agent has task goal (simplified check)
            auto goals = agent->getGoals();
            bool has_task_goal = false;
            for (const auto& goal : goals) {
                if (goal.find("complete_task_" + task.id) != std::string::npos) {
                    has_task_goal = true;
                    break;
                }
            }
            
            if (has_task_goal) {
                all_complete = false;
                break;
            }
        }
        
        if (all_complete && !task.completed) {
            completeTask(task.id, true);
        }
    }
}

void SwarmCog::completeTask(const std::string& task_id, bool successful) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = active_tasks_.find(task_id);
    if (it != active_tasks_.end()) {
        auto task = it->second;
        task.completed = true;
        task.results["status"] = successful ? "success" : "failed";
        task.results["completion_time"] = Utils::TimeUtils::timestampToString(Utils::TimeUtils::now());
        
        completed_tasks_.push_back(task);
        active_tasks_.erase(it);
        
        task_counter_.increment();
        
        Utils::Logger::info("Completed task: " + task_id + " (status: " + 
                           (successful ? "success" : "failed") + ")");
    }
}

void SwarmCog::recordInteraction(const AgentId& agent1, const AgentId& agent2, 
                                const std::string& type, bool successful) {
    std::lock_guard<std::mutex> lock(interactions_mutex_);
    
    AgentInteraction interaction(agent1, agent2, type);
    interaction.successful = successful;
    
    interaction_history_.push_back(interaction);
    interaction_counter_.increment();
}

void SwarmCog::analyzeInteractionPatterns() {
    // Simple analysis - count successful vs failed interactions
    std::lock_guard<std::mutex> lock(interactions_mutex_);
    
    size_t successful = 0;
    for (const auto& interaction : interaction_history_) {
        if (interaction.successful) {
            successful++;
        }
    }
    
    if (!interaction_history_.empty()) {
        double success_rate = static_cast<double>(successful) / interaction_history_.size();
        Utils::Logger::debug("Interaction success rate: " + std::to_string(success_rate));
    }
}

void SwarmCog::updateSystemStatus() {
    system_status_.uptime = calculateUptime();
    system_status_.active_agents = getAgentCount();
}

std::chrono::milliseconds SwarmCog::calculateUptime() const {
    auto now = Utils::TimeUtils::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - system_status_.start_time);
}

// Factory function
std::shared_ptr<SwarmCog> createSwarmCog(
    const std::string& agentspace_name,
    ProcessingMode processing_mode,
    size_t max_agents) {
    
    SwarmCogConfig config;
    config.agentspace_name = agentspace_name;
    config.processing_mode = processing_mode;
    config.max_agents = max_agents;
    
    return std::make_shared<SwarmCog>(config);
}

// Utility functions for swarm analysis
double calculateNetworkDensity(const SwarmTopology& topology) {
    if (topology.total_agents <= 1) return 0.0;
    
    size_t possible_connections = topology.total_agents * (topology.total_agents - 1);
    return static_cast<double>(topology.total_connections) / possible_connections;
}

std::vector<AgentId> findCentralAgents(const SwarmTopology& topology, size_t limit) {
    std::vector<std::pair<AgentId, size_t>> agent_connections;
    
    for (const auto& conn_pair : topology.connections) {
        agent_connections.push_back({conn_pair.first, conn_pair.second.size()});
    }
    
    // Sort by connection count
    std::sort(agent_connections.begin(), agent_connections.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<AgentId> central_agents;
    for (size_t i = 0; i < std::min(limit, agent_connections.size()); ++i) {
        central_agents.push_back(agent_connections[i].first);
    }
    
    return central_agents;
}

std::map<std::string, double> analyzeCapabilityDistribution(const SwarmTopology& topology) {
    std::map<std::string, size_t> capability_counts;
    
    for (const auto& agent_caps : topology.agent_capabilities) {
        for (const auto& capability : agent_caps.second) {
            capability_counts[capability]++;
        }
    }
    
    std::map<std::string, double> distribution;
    for (const auto& cap_pair : capability_counts) {
        distribution[cap_pair.first] = static_cast<double>(cap_pair.second) / topology.total_agents;
    }
    
    return distribution;
}

} // namespace SwarmCog