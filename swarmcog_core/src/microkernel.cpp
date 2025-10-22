#include "swarmcog/microkernel.h"
#include "swarmcog/utils.h"
#include <algorithm>

namespace SwarmCog {

// CognitiveMicrokernel Implementation
CognitiveMicrokernel::CognitiveMicrokernel(std::shared_ptr<AgentSpace> agentspace, 
                                         ProcessingMode mode, size_t num_workers)
    : agentspace_(agentspace), processing_mode_(mode), cycle_interval_(1.0),
      num_workers_(num_workers > 0 ? num_workers : Utils::ThreadUtils::getOptimalThreadCount()) {
    
    Utils::Logger::info("Initializing CognitiveMicrokernel with " + 
                       std::to_string(num_workers_) + " worker threads");
}

CognitiveMicrokernel::~CognitiveMicrokernel() {
    if (running_) {
        stop();
    }
}

CognitiveState CognitiveMicrokernel::addCognitiveAgent(const AgentId& agent_id, 
                                                     const std::vector<std::string>& goals,
                                                     const std::map<std::string, std::string>& beliefs) {
    std::unique_lock<std::shared_mutex> lock(agents_mutex_);
    
    if (agent_states_.find(agent_id) != agent_states_.end()) {
        Utils::Logger::warning("Agent already exists in microkernel: " + agent_id);
        return agent_states_[agent_id];
    }
    
    CognitiveState state(agent_id);
    state.goals = goals;
    state.beliefs = beliefs;
    state.current_phase = CognitivePhase::PERCEPTION;
    state.last_update = Utils::TimeUtils::now();
    
    agent_states_[agent_id] = state;
    
    Utils::Logger::info("Added cognitive agent to microkernel: " + agent_id);
    return state;
}

bool CognitiveMicrokernel::removeCognitiveAgent(const AgentId& agent_id) {
    std::unique_lock<std::shared_mutex> lock(agents_mutex_);
    
    auto it = agent_states_.find(agent_id);
    if (it == agent_states_.end()) {
        return false;
    }
    
    agent_states_.erase(it);
    agent_callbacks_.erase(agent_id);
    
    Utils::Logger::info("Removed cognitive agent from microkernel: " + agent_id);
    return true;
}

bool CognitiveMicrokernel::hasAgent(const AgentId& agent_id) const {
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    return agent_states_.find(agent_id) != agent_states_.end();
}

std::vector<AgentId> CognitiveMicrokernel::getActiveAgents() const {
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    
    std::vector<AgentId> agents;
    for (const auto& pair : agent_states_) {
        agents.push_back(pair.first);
    }
    
    return agents;
}

CognitiveState CognitiveMicrokernel::getCognitiveState(const AgentId& agent_id) const {
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    
    auto it = agent_states_.find(agent_id);
    if (it != agent_states_.end()) {
        return it->second;
    }
    
    return CognitiveState();  // Return empty state if not found
}

bool CognitiveMicrokernel::updateCognitiveState(const AgentId& agent_id, const CognitiveState& state) {
    std::unique_lock<std::shared_mutex> lock(agents_mutex_);
    
    auto it = agent_states_.find(agent_id);
    if (it == agent_states_.end()) {
        return false;
    }
    
    it->second = state;
    it->second.last_update = Utils::TimeUtils::now();
    
    // Notify callbacks
    lock.unlock();
    notifyCallbacks(agent_id, state);
    
    return true;
}

void CognitiveMicrokernel::addGoal(const AgentId& agent_id, const std::string& goal) {
    std::unique_lock<std::shared_mutex> lock(agents_mutex_);
    
    auto it = agent_states_.find(agent_id);
    if (it != agent_states_.end()) {
        auto& goals = it->second.goals;
        if (std::find(goals.begin(), goals.end(), goal) == goals.end()) {
            goals.push_back(goal);
            it->second.last_update = Utils::TimeUtils::now();
        }
    }
}

void CognitiveMicrokernel::updateBelief(const AgentId& agent_id, const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(agents_mutex_);
    
    auto it = agent_states_.find(agent_id);
    if (it != agent_states_.end()) {
        it->second.beliefs[key] = value;
        it->second.last_update = Utils::TimeUtils::now();
    }
}

void CognitiveMicrokernel::start() {
    if (running_) {
        Utils::Logger::warning("Microkernel already running");
        return;
    }
    
    running_ = true;
    stats_.start_time = std::chrono::system_clock::now();
    
    // Start worker threads
    for (size_t i = 0; i < num_workers_; ++i) {
        worker_threads_.emplace_back(&CognitiveMicrokernel::workerThread, this);
    }
    
    Utils::Logger::info("CognitiveMicrokernel started with " + std::to_string(num_workers_) + " threads");
}

void CognitiveMicrokernel::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    queue_cv_.notify_all();
    
    // Wait for all worker threads to complete
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    worker_threads_.clear();
    
    Utils::Logger::info("CognitiveMicrokernel stopped");
}

void CognitiveMicrokernel::scheduleTask(const CognitiveTask& task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push(task);
    }
    queue_cv_.notify_one();
}

void CognitiveMicrokernel::scheduleCognitivePhase(const AgentId& agent_id, CognitivePhase phase, 
                                                 const std::map<std::string, std::string>& parameters) {
    CognitiveTask task;
    task.id = generateTaskId();
    task.agent_id = agent_id;
    task.phase = phase;
    task.description = "Cognitive phase: " + std::to_string(static_cast<int>(phase));
    task.parameters = parameters;
    task.scheduled_at = Utils::TimeUtils::now();
    
    scheduleTask(task);
}

void CognitiveMicrokernel::runCognitiveCycle(const AgentId& agent_id) {
    if (!hasAgent(agent_id)) {
        Utils::Logger::warning("Cannot run cognitive cycle for unknown agent: " + agent_id);
        return;
    }
    
    Utils::Logger::debug("Starting cognitive cycle for agent: " + agent_id);
    
    // Schedule all 7 phases of the cognitive cycle
    auto phases = {
        CognitivePhase::PERCEPTION,
        CognitivePhase::ATTENTION,
        CognitivePhase::REASONING,
        CognitivePhase::PLANNING,
        CognitivePhase::EXECUTION,
        CognitivePhase::LEARNING,
        CognitivePhase::REFLECTION
    };
    
    for (auto phase : phases) {
        scheduleCognitivePhase(agent_id, phase);
    }
    
    stats_.total_cycles.fetch_add(1);
}

void CognitiveMicrokernel::runAllAgentsCycles() {
    auto agents = getActiveAgents();
    
    for (const auto& agent_id : agents) {
        runCognitiveCycle(agent_id);
    }
}

// Cognitive Phase Implementations
void CognitiveMicrokernel::processPerceptionPhase(const AgentId& agent_id, CognitiveContext& context) {
    Utils::Logger::debug("Processing perception phase for agent: " + agent_id);
    
    // Gather environmental data
    gatherEnvironmentalData(agent_id, context);
    
    // Update agent's perceptual state
    auto state = getCognitiveState(agent_id);
    state.current_phase = CognitivePhase::ATTENTION;
    updateCognitiveState(agent_id, state);
}

void CognitiveMicrokernel::processAttentionPhase(const AgentId& agent_id, CognitiveContext& context) {
    Utils::Logger::debug("Processing attention phase for agent: " + agent_id);
    
    // Select what to focus on
    selectAttentionalFocus(agent_id, context);
    
    // Update AgentSpace attention values
    agentspace_->updateAttentionValues();
    
    auto state = getCognitiveState(agent_id);
    state.current_phase = CognitivePhase::REASONING;
    state.current_focus = context.focus_atoms;
    updateCognitiveState(agent_id, state);
}

void CognitiveMicrokernel::processReasoningPhase(const AgentId& agent_id, CognitiveContext& context) {
    Utils::Logger::debug("Processing reasoning phase for agent: " + agent_id);
    
    // Perform reasoning about current situation
    performReasoning(agent_id, context);
    
    auto state = getCognitiveState(agent_id);
    state.current_phase = CognitivePhase::PLANNING;
    updateCognitiveState(agent_id, state);
}

void CognitiveMicrokernel::processPlanningPhase(const AgentId& agent_id, CognitiveContext& context) {
    Utils::Logger::debug("Processing planning phase for agent: " + agent_id);
    
    // Create action plans
    createActionPlans(agent_id, context);
    
    auto state = getCognitiveState(agent_id);
    state.current_phase = CognitivePhase::EXECUTION;
    updateCognitiveState(agent_id, state);
}

void CognitiveMicrokernel::processExecutionPhase(const AgentId& agent_id, CognitiveContext& context) {
    Utils::Logger::debug("Processing execution phase for agent: " + agent_id);
    
    // Execute planned actions
    executeActions(agent_id, context);
    
    auto state = getCognitiveState(agent_id);
    state.current_phase = CognitivePhase::LEARNING;
    updateCognitiveState(agent_id, state);
}

void CognitiveMicrokernel::processLearningPhase(const AgentId& agent_id, CognitiveContext& context) {
    Utils::Logger::debug("Processing learning phase for agent: " + agent_id);
    
    // Update knowledge based on experiences
    updateKnowledge(agent_id, context);
    
    auto state = getCognitiveState(agent_id);
    state.current_phase = CognitivePhase::REFLECTION;
    updateCognitiveState(agent_id, state);
}

void CognitiveMicrokernel::processReflectionPhase(const AgentId& agent_id, CognitiveContext& context) {
    Utils::Logger::debug("Processing reflection phase for agent: " + agent_id);
    
    // Evaluate performance and adjust
    evaluatePerformance(agent_id, context);
    
    auto state = getCognitiveState(agent_id);
    state.current_phase = CognitivePhase::PERCEPTION;  // Reset to start of cycle
    updateCognitiveState(agent_id, state);
}

void CognitiveMicrokernel::registerCallback(const AgentId& agent_id, const CognitiveCallback& callback) {
    std::unique_lock<std::shared_mutex> lock(agents_mutex_);
    agent_callbacks_[agent_id].push_back(callback);
}

void CognitiveMicrokernel::unregisterCallbacks(const AgentId& agent_id) {
    std::unique_lock<std::shared_mutex> lock(agents_mutex_);
    agent_callbacks_.erase(agent_id);
}

ProcessingStats CognitiveMicrokernel::getProcessingStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void CognitiveMicrokernel::resetStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = ProcessingStats();
}

std::map<std::string, std::string> CognitiveMicrokernel::getSystemStatus() const {
    std::map<std::string, std::string> status;
    
    status["running"] = running_ ? "true" : "false";
    status["num_workers"] = std::to_string(num_workers_);
    status["processing_mode"] = std::to_string(static_cast<int>(processing_mode_));
    status["cycle_interval"] = std::to_string(cycle_interval_);
    
    auto stats = getProcessingStats();
    status["total_cycles"] = std::to_string(stats.total_cycles.load());
    status["completed_tasks"] = std::to_string(stats.completed_tasks.load());
    status["failed_tasks"] = std::to_string(stats.failed_tasks.load());
    
    {
        std::shared_lock<std::shared_mutex> lock(agents_mutex_);
        status["active_agents"] = std::to_string(agent_states_.size());
    }
    
    // Note: Skip queue size reporting due to const method constraint
    
    return status;
}

void CognitiveMicrokernel::setProcessingMode(ProcessingMode mode) {
    processing_mode_ = mode;
    Utils::Logger::info("Processing mode changed to: " + std::to_string(static_cast<int>(mode)));
}

// Private methods
void CognitiveMicrokernel::workerThread() {
    Utils::ThreadUtils::setThreadName("CognitiveMicrokernel Worker");
    
    while (running_) {
        CognitiveTask task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this]() { return !running_ || !task_queue_.empty(); });
            
            if (!running_) break;
            
            if (!task_queue_.empty()) {
                task = task_queue_.top();
                task_queue_.pop();
            } else {
                continue;
            }
        }
        
        processTask(task);
    }
}

void CognitiveMicrokernel::processTask(const CognitiveTask& task) {
    auto start_time = std::chrono::high_resolution_clock::now();
    bool success = false;
    
    try {
        Utils::Logger::debug("Processing task: " + task.id + " for agent: " + task.agent_id);
        
        CognitiveContext context(task.agent_id);
        context.variables = task.parameters;
        
        executePhaseFunction(task.agent_id, task.phase, context);
        success = true;
        
    } catch (const std::exception& e) {
        Utils::Logger::error("Task failed: " + task.id + " - " + e.what());
        success = false;
    } catch (...) {
        Utils::Logger::error("Task failed with unknown error: " + task.id);
        success = false;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    updateStats(task, success, duration);
}

void CognitiveMicrokernel::executePhaseFunction(const AgentId& agent_id, CognitivePhase phase, CognitiveContext& context) {
    switch (phase) {
        case CognitivePhase::PERCEPTION:
            processPerceptionPhase(agent_id, context);
            break;
        case CognitivePhase::ATTENTION:
            processAttentionPhase(agent_id, context);
            break;
        case CognitivePhase::REASONING:
            processReasoningPhase(agent_id, context);
            break;
        case CognitivePhase::PLANNING:
            processPlanningPhase(agent_id, context);
            break;
        case CognitivePhase::EXECUTION:
            processExecutionPhase(agent_id, context);
            break;
        case CognitivePhase::LEARNING:
            processLearningPhase(agent_id, context);
            break;
        case CognitivePhase::REFLECTION:
            processReflectionPhase(agent_id, context);
            break;
    }
}

void CognitiveMicrokernel::notifyCallbacks(const AgentId& agent_id, const CognitiveState& state) {
    std::shared_lock<std::shared_mutex> lock(agents_mutex_);
    
    auto it = agent_callbacks_.find(agent_id);
    if (it != agent_callbacks_.end()) {
        for (const auto& callback : it->second) {
            try {
                callback(state);
            } catch (const std::exception& e) {
                Utils::Logger::error("Callback error for agent " + agent_id + ": " + e.what());
            }
        }
    }
}

// Phase implementation helpers
void CognitiveMicrokernel::gatherEnvironmentalData(const AgentId& agent_id, CognitiveContext& context) {
    // Get agent's current focus from AgentSpace
    auto focus = agentspace_->getAttentionalFocus();
    
    // Add relevant atoms to context
    for (const auto& atom_id : focus) {
        context.focus_atoms.push_back(atom_id);
    }
    
    // Update context variables with environmental data
    context.variables["perception_timestamp"] = Utils::TimeUtils::timestampToString(Utils::TimeUtils::now());
    context.variables["environment_state"] = "active";
}

void CognitiveMicrokernel::selectAttentionalFocus(const AgentId& agent_id, CognitiveContext& context) {
    // Get most important atoms for this agent
    auto important_atoms = agentspace_->getMostImportantAtoms(5);
    
    context.focus_atoms.clear();
    for (const auto& atom : important_atoms) {
        context.focus_atoms.push_back(atom->getId());
        agentspace_->addToAttentionalFocus(atom->getId());
    }
}

void CognitiveMicrokernel::performReasoning(const AgentId& agent_id, CognitiveContext& context) {
    // Simple reasoning based on current goals and beliefs
    auto state = getCognitiveState(agent_id);
    
    // Analyze current goals
    context.variables["active_goals"] = Utils::StringUtils::join(state.goals, ",");
    
    // Add reasoning results
    context.variables["reasoning_result"] = "goal_analysis_complete";
    context.variables["reasoning_confidence"] = "0.8";
}

void CognitiveMicrokernel::createActionPlans(const AgentId& agent_id, CognitiveContext& context) {
    auto state = getCognitiveState(agent_id);
    
    // Create simple action plans based on goals
    std::vector<std::string> plans;
    for (const auto& goal : state.goals) {
        plans.push_back("plan_for_" + goal);
    }
    
    context.variables["action_plans"] = Utils::StringUtils::join(plans, ",");
    
    // Update agent's intentions
    state.intentions = plans;
    updateCognitiveState(agent_id, state);
}

void CognitiveMicrokernel::executeActions(const AgentId& agent_id, CognitiveContext& context) {
    // Execute planned actions
    auto plans_str = context.variables["action_plans"];
    auto plans = Utils::StringUtils::split(plans_str, ',');
    
    for (const auto& plan : plans) {
        Utils::Logger::debug("Executing action plan: " + plan + " for agent: " + agent_id);
        // In a real implementation, this would execute specific actions
    }
    
    context.variables["actions_executed"] = std::to_string(plans.size());
}

void CognitiveMicrokernel::updateKnowledge(const AgentId& agent_id, CognitiveContext& context) {
    // Update agent's knowledge based on execution results
    auto actions_executed = Utils::ConfigUtils::parseInt(context.variables["actions_executed"]);
    
    if (actions_executed > 0) {
        // Add memory of successful actions
        auto memory_content = "Executed " + std::to_string(actions_executed) + " actions successfully";
        agentspace_->addMemoryNode(memory_content, "procedural");
        
        context.variables["learning_outcome"] = "knowledge_updated";
    }
}

void CognitiveMicrokernel::evaluatePerformance(const AgentId& agent_id, CognitiveContext& context) {
    // Simple performance evaluation
    auto actions_executed = Utils::ConfigUtils::parseInt(context.variables["actions_executed"]);
    double performance_score = actions_executed > 0 ? 0.8 : 0.3;
    
    context.variables["performance_score"] = std::to_string(performance_score);
    context.variables["reflection_complete"] = "true";
    
    Utils::Logger::debug("Performance evaluation for agent " + agent_id + 
                        ": score=" + std::to_string(performance_score));
}

std::string CognitiveMicrokernel::generateTaskId() const {
    return "task_" + Utils::UUIDGenerator::generateShort(8);
}

void CognitiveMicrokernel::updateStats(const CognitiveTask& task, bool success, std::chrono::milliseconds duration) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    if (success) {
        stats_.completed_tasks.fetch_add(1);
    } else {
        stats_.failed_tasks.fetch_add(1);
    }
    
    stats_.total_processing_time += duration;
}

// CognitivePhaseFactory implementation
void CognitivePhaseFactory::registerProcessor(std::unique_ptr<CognitivePhaseProcessor> processor) {
    if (processor) {
        CognitivePhase phase = processor->getPhaseType();
        processors_[phase] = std::move(processor);
    }
}

CognitivePhaseProcessor* CognitivePhaseFactory::getProcessor(CognitivePhase phase) const {
    auto it = processors_.find(phase);
    return (it != processors_.end()) ? it->second.get() : nullptr;
}

bool CognitivePhaseFactory::hasProcessor(CognitivePhase phase) const {
    return processors_.find(phase) != processors_.end();
}

} // namespace SwarmCog