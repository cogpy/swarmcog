#include "swarmcog/swarmcog.h"
#include "swarmcog/utils.h"
#include <iostream>
#include <cassert>

using namespace SwarmCog;

void testAgentSpaceBasics() {
    std::cout << "Testing AgentSpace basics..." << std::endl;
    
    auto agentspace = std::make_shared<AgentSpace>("test_space");
    assert(agentspace->getAtomCount() == 0);
    
    // Add an agent node
    auto agent_node = agentspace->addAgentNode("test_agent", {"reasoning", "planning"});
    assert(agent_node != nullptr);
    assert(agentspace->getAtomCount() == 1);
    
    // Add a capability node
    auto capability_node = agentspace->addCapabilityNode("reasoning", "Logical reasoning capability");
    assert(capability_node != nullptr);
    assert(agentspace->getAtomCount() == 2);
    
    std::cout << "AgentSpace test passed!" << std::endl;
}

void testCognitiveMicrokernel() {
    std::cout << "Testing CognitiveMicrokernel basics..." << std::endl;
    
    auto agentspace = std::make_shared<AgentSpace>("kernel_test_space");
    auto microkernel = std::make_shared<CognitiveMicrokernel>(agentspace);
    
    // Add a cognitive agent to the microkernel
    std::vector<std::string> goals = {"solve_problems", "collaborate"};
    std::map<std::string, std::string> beliefs = {{"domain", "AI"}, {"experience", "expert"}};
    
    auto state = microkernel->addCognitiveAgent("agent1", goals, beliefs);
    assert(state.agent_id == "agent1");
    assert(state.goals.size() == 2);
    
    // Check if agent exists
    assert(microkernel->hasAgent("agent1"));
    assert(!microkernel->hasAgent("nonexistent"));
    
    std::cout << "CognitiveMicrokernel test passed!" << std::endl;
}

void testCognitiveAgent() {
    std::cout << "Testing CognitiveAgent basics..." << std::endl;
    
    auto agentspace = std::make_shared<AgentSpace>("agent_test_space");
    auto microkernel = std::make_shared<CognitiveMicrokernel>(agentspace);
    
    // Create a cognitive agent
    auto agent = std::make_shared<CognitiveAgent>("agent1", "TestAgent", agentspace, microkernel);
    
    // Test capabilities
    agent->addCapability("reasoning", "Logical reasoning", 0.8);
    assert(agent->hasCapability("reasoning"));
    assert(!agent->hasCapability("nonexistent"));
    
    auto capability = agent->getCapability("reasoning");
    assert(capability.name == "reasoning");
    assert(capability.strength >= 0.79 && capability.strength <= 0.81); // floating point comparison
    
    // Test goals
    agent->addGoal("test_goal", 0.9);
    auto goals = agent->getGoals();
    assert(goals.size() == 1);
    assert(goals[0] == "test_goal");
    
    std::cout << "CognitiveAgent test passed!" << std::endl;
}

void testSwarmCog() {
    std::cout << "Testing SwarmCog system..." << std::endl;
    
    SwarmCogConfig config;
    config.agentspace_name = "test_swarm";
    config.max_agents = 10;
    
    auto swarmcog = std::make_shared<SwarmCog::SwarmCog>(config);
    assert(swarmcog->isInitialized());
    
    // Create an agent
    std::vector<std::string> capabilities = {"reasoning", "planning"};
    std::vector<std::string> goals = {"solve_problems"};
    std::map<std::string, std::string> beliefs = {{"domain", "testing"}};
    
    auto agent = swarmcog->createCognitiveAgent(
        "test_agent", "TestAgent", "cognitive_v1", "Test instructions",
        capabilities, goals, beliefs
    );
    
    assert(agent != nullptr);
    assert(swarmcog->getAgentCount() == 1);
    
    // Test agent retrieval
    auto retrieved_agent = swarmcog->getAgent("test_agent");
    assert(retrieved_agent != nullptr);
    assert(retrieved_agent->getId() == "test_agent");
    
    std::cout << "SwarmCog test passed!" << std::endl;
}

void testUtils() {
    std::cout << "Testing utility functions..." << std::endl;
    
    // Test UUID generation
    auto uuid1 = Utils::UUIDGenerator::generate();
    auto uuid2 = Utils::UUIDGenerator::generate();
    assert(uuid1 != uuid2);
    assert(uuid1.length() > 10);
    
    // Test string utilities
    std::vector<std::string> parts = Utils::StringUtils::split("a,b,c", ',');
    assert(parts.size() == 3);
    assert(parts[0] == "a");
    assert(parts[1] == "b");
    assert(parts[2] == "c");
    
    std::string joined = Utils::StringUtils::join(parts, "-");
    assert(joined == "a-b-c");
    
    // Test math utilities
    assert(Utils::MathUtils::clamp(1.5, 0.0, 1.0) == 1.0);
    assert(Utils::MathUtils::clamp(-0.5, 0.0, 1.0) == 0.0);
    assert(Utils::MathUtils::clamp(0.5, 0.0, 1.0) == 0.5);
    
    std::cout << "Utils test passed!" << std::endl;
}

int main() {
    std::cout << "Running SwarmCog C++ Core Tests..." << std::endl;
    
    try {
        testUtils();
        testAgentSpaceBasics();
        testCognitiveMicrokernel();
        testCognitiveAgent();
        testSwarmCog();
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}