#include "swarmcog/swarmcog.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace SwarmCog;

int main() {
    std::cout << "SwarmCog C++ Cognitive Microkernel Demo\n";
    std::cout << "========================================\n\n";
    
    // Configure SwarmCog system
    SwarmCogConfig config;
    config.agentspace_name = "research_lab";
    config.processing_mode = ProcessingMode::ASYNCHRONOUS;
    config.max_agents = 10;
    config.cognitive_cycle_interval = 1.0;  // 1 second cycles
    
    // Create SwarmCog system
    auto swarmcog = std::make_shared<SwarmCog::SwarmCog>(config);
    
    std::cout << "1. Created SwarmCog system with AgentSpace: " << config.agentspace_name << "\n";
    
    // Create cognitive agents
    std::vector<std::string> researcher_capabilities = {"research", "analysis", "collaboration"};
    std::vector<std::string> researcher_goals = {"solve_complex_problems", "mentor_team"};
    std::map<std::string, std::string> researcher_beliefs = {
        {"domain", "AI_research"},
        {"experience", "expert"},
        {"approach", "collaborative"}
    };
    
    auto dr_alice = swarmcog->createCognitiveAgent(
        "dr_alice", "Dr. Alice", "cognitive_v1",
        "You are a research leader with expertise in AI and cognitive systems.",
        researcher_capabilities, researcher_goals, researcher_beliefs
    );
    
    std::cout << "2. Created cognitive agent: Dr. Alice\n";
    std::cout << "\nSwarmCog C++ Cognitive Microkernel Demo Complete!\n";
    std::cout << "Successfully demonstrated basic cognitive agent creation.\n";
    
    return 0;
}
