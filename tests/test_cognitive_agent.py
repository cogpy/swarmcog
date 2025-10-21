import pytest
import asyncio
import json
from swarm.cognitive_agent import CognitiveAgent, CognitiveCapability
from swarm.agentspace import AgentSpace, AtomType
from swarm.microkernel import CognitiveMicrokernel


class TestCognitiveCapability:
    def test_capability_creation(self):
        cap = CognitiveCapability("reasoning", "Logical reasoning capability", 0.8, 5)
        
        assert cap.name == "reasoning"
        assert cap.description == "Logical reasoning capability"
        assert cap.strength == 0.8
        assert cap.experience == 5
    
    def test_capability_to_dict(self):
        cap = CognitiveCapability("learning", "Machine learning capability")
        result = cap.to_dict()
        
        expected = {
            "name": "learning",
            "description": "Machine learning capability",
            "strength": 0.5,
            "experience": 0
        }
        assert result == expected


class TestCognitiveAgent:
    def test_agent_creation(self):
        agent = CognitiveAgent(name="test_agent")
        
        assert agent.name == "test_agent"
        assert isinstance(agent.agentspace, AgentSpace)
        assert isinstance(agent.microkernel, CognitiveMicrokernel)
        assert agent.cognitive_state is not None
        assert not agent.is_cognitive_active
    
    def test_agent_with_capabilities(self):
        capabilities = ["reasoning", "learning", "communication"]
        agent = CognitiveAgent(
            name="capable_agent",
            capabilities=capabilities
        )
        
        # Check capabilities were added to agentspace
        cap_atoms = agent.agentspace.find_atoms(AtomType.CAPABILITY_NODE)
        cap_names = [atom.name for atom in cap_atoms]
        
        for cap in capabilities:
            assert cap in cap_names
    
    def test_agent_with_goals(self):
        goals = ["learn_new_skills", "collaborate_with_others"]
        agent = CognitiveAgent(
            name="goal_oriented_agent",
            goals=goals
        )
        
        assert agent.goals == goals
        
        # Check goals were added to agentspace
        goal_atoms = agent.agentspace.find_atoms(AtomType.GOAL_NODE)
        goal_names = [atom.name for atom in goal_atoms]
        
        for goal in goals:
            assert goal in goal_names
    
    def test_agent_with_beliefs(self):
        beliefs = {"world_state": "collaborative", "trust_level": 0.7}
        agent = CognitiveAgent(
            name="believing_agent",
            initial_beliefs=beliefs
        )
        
        assert agent.beliefs == beliefs
        
        # Check beliefs were added to agentspace
        belief_atoms = agent.agentspace.find_atoms(AtomType.BELIEF_NODE)
        belief_names = [atom.name for atom in belief_atoms]
        
        for belief_name in beliefs.keys():
            assert belief_name in belief_names
    
    def test_add_capability(self):
        agent = CognitiveAgent(name="test_agent")
        
        cap = agent.add_capability("problem_solving", "Solves complex problems", 0.9)
        
        assert "problem_solving" in agent.capabilities
        assert agent.capabilities["problem_solving"] == cap
        assert cap.strength == 0.9
        
        # Check capability was added to agentspace
        cap_atoms = agent.agentspace.find_atoms(AtomType.CAPABILITY_NODE, name="problem_solving")
        assert len(cap_atoms) == 1
    
    def test_add_goal(self):
        agent = CognitiveAgent(name="test_agent")
        
        agent.add_goal("master_robotics", 0.8)
        
        assert "master_robotics" in agent.goals
        
        # Check goal was added to agentspace
        goal_atoms = agent.agentspace.find_atoms(AtomType.GOAL_NODE, name="master_robotics")
        assert len(goal_atoms) == 1
        assert goal_atoms[0].truth_value.strength == 0.8
    
    def test_update_belief(self):
        agent = CognitiveAgent(name="test_agent")
        
        agent.update_belief("weather", "sunny")
        
        assert agent.beliefs["weather"] == "sunny"
        
        # Check belief was added to agentspace
        belief_atoms = agent.agentspace.find_atoms(AtomType.BELIEF_NODE, name="weather")
        assert len(belief_atoms) == 1
        assert belief_atoms[0].value == "sunny"
    
    def test_add_memory(self):
        agent = CognitiveAgent(name="test_agent")
        
        agent.add_memory("interaction", {"partner": "agent2", "outcome": "success"}, 0.7)
        
        assert len(agent.memories) == 1
        memory = agent.memories[0]
        assert memory["type"] == "interaction"
        assert memory["importance"] == 0.7
        
        # Check memory was added to agentspace
        memory_atoms = agent.agentspace.find_atoms(AtomType.MEMORY_NODE)
        assert len(memory_atoms) == 1
    
    def test_get_collaborators(self):
        # Create two agents in same agentspace
        agentspace = AgentSpace("shared_space")
        microkernel = CognitiveMicrokernel(agentspace)
        
        agent1 = CognitiveAgent(name="agent1", agentspace=agentspace, microkernel=microkernel)
        agent2 = CognitiveAgent(name="agent2", agentspace=agentspace, microkernel=microkernel)
        
        # Add collaboration link
        agent1_nodes = [n for n in agentspace.get_agents() if n.name == "agent1"]
        agent2_nodes = [n for n in agentspace.get_agents() if n.name == "agent2"]
        
        if agent1_nodes and agent2_nodes:
            agentspace.add_collaboration_link(agent1_nodes[0], agent2_nodes[0])
        
        collaborators = agent1.get_collaborators()
        assert "agent2" in collaborators
    
    def test_cognitive_functions(self):
        agent = CognitiveAgent(name="test_agent")
        
        # Test that cognitive functions are added to agent functions
        function_names = [f.__name__ for f in agent.functions if callable(f)]
        
        expected_functions = [
            "perceive_environment",
            "reason_about_goals", 
            "plan_collaboration",
            "execute_cognitive_action",
            "reflect_on_performance",
            "learn_from_experience",
            "find_collaborators",
            "establish_trust",
            "share_knowledge",
            "coordinate_with_agent"
        ]
        
        for func_name in expected_functions:
            assert func_name in function_names
    
    def test_perceive_environment(self):
        # Create multi-agent environment
        agentspace = AgentSpace("shared_space")
        microkernel = CognitiveMicrokernel(agentspace)
        
        agent1 = CognitiveAgent(name="agent1", agentspace=agentspace, microkernel=microkernel)
        agent2 = CognitiveAgent(name="agent2", agentspace=agentspace, microkernel=microkernel)
        
        result = agent1.perceive_environment()
        perception = json.loads(result)
        
        assert "visible_agents" in perception
        assert "agent2" in perception["visible_agents"]
        assert "recent_activity" in perception
        assert "agentspace_size" in perception
    
    def test_reason_about_goals(self):
        agent = CognitiveAgent(name="test_agent", goals=["collaborate_on_project", "learn_new_skills"])
        
        result = agent.reason_about_goals()
        reasoning = json.loads(result)
        
        assert "active_goals" in reasoning
        assert "goal_analysis" in reasoning
        assert len(reasoning["active_goals"]) == 2
        assert "collaborate_on_project" in reasoning["active_goals"]
    
    def test_plan_collaboration(self):
        # Create multi-agent environment
        agentspace = AgentSpace("shared_space")
        microkernel = CognitiveMicrokernel(agentspace)
        
        agent1 = CognitiveAgent(name="agent1", agentspace=agentspace, microkernel=microkernel)
        agent2 = CognitiveAgent(name="agent2", agentspace=agentspace, microkernel=microkernel)
        
        result = agent1.plan_collaboration("research_project")
        plan = json.loads(result)
        
        assert "goal" in plan
        assert plan["goal"] == "research_project"
        assert "potential_partners" in plan
        assert "action_steps" in plan
        
        # Should identify agent2 as potential partner
        assert "agent2" in plan["potential_partners"]
    
    def test_establish_trust(self):
        # Create multi-agent environment
        agentspace = AgentSpace("shared_space")
        microkernel = CognitiveMicrokernel(agentspace)
        
        agent1 = CognitiveAgent(name="agent1", agentspace=agentspace, microkernel=microkernel)
        agent2 = CognitiveAgent(name="agent2", agentspace=agentspace, microkernel=microkernel)
        
        result = agent1.establish_trust("agent2", 0.8)
        trust_result = json.loads(result)
        
        assert trust_result["action"] == "establish_trust"
        assert trust_result["target_agent"] == "agent2"
        assert trust_result["trust_level"] == 0.8
        assert trust_result["status"] == "success"
        
        # Verify trust was established in agentspace
        trust_levels = agent1.get_trust_levels()
        assert "agent2" in trust_levels
        assert trust_levels["agent2"] == 0.8
    
    def test_share_knowledge(self):
        agent = CognitiveAgent(name="test_agent")
        
        result = agent.share_knowledge("machine_learning", {"algorithm": "neural_network"}, "agent2")
        sharing_result = json.loads(result)
        
        assert sharing_result["action"] == "share_knowledge"
        assert sharing_result["knowledge_type"] == "machine_learning"
        assert sharing_result["target"] == "agent2"
        assert sharing_result["status"] == "success"
        
        # Check memory was added
        assert len(agent.memories) == 1
        assert agent.memories[0]["type"] == "knowledge_sharing"
    
    def test_coordinate_with_agent(self):
        # Create multi-agent environment
        agentspace = AgentSpace("shared_space")
        microkernel = CognitiveMicrokernel(agentspace)
        
        agent1 = CognitiveAgent(name="agent1", agentspace=agentspace, microkernel=microkernel)
        agent2 = CognitiveAgent(name="agent2", agentspace=agentspace, microkernel=microkernel)
        
        result = agent1.coordinate_with_agent("agent2", "joint_research")
        coordination_result = json.loads(result)
        
        assert coordination_result["action"] == "coordinate_with_agent"
        assert coordination_result["partner"] == "agent2"
        assert coordination_result["coordination_type"] == "joint_research"
        assert coordination_result["status"] == "success"
        
        # Check collaboration link was created
        collaborators = agent1.get_collaborators()
        assert "agent2" in collaborators
    
    def test_find_collaborators(self):
        # Create multi-agent environment with capabilities
        agentspace = AgentSpace("shared_space")
        microkernel = CognitiveMicrokernel(agentspace)
        
        agent1 = CognitiveAgent(name="agent1", agentspace=agentspace, microkernel=microkernel, 
                               capabilities=["reasoning"])
        agent2 = CognitiveAgent(name="agent2", agentspace=agentspace, microkernel=microkernel,
                               capabilities=["learning"])
        
        # Establish some trust
        agent1.establish_trust("agent2", 0.7)
        
        result = agent1.find_collaborators("learning")
        search_result = json.loads(result)
        
        assert "search_criteria" in search_result
        assert "potential_collaborators" in search_result
        assert "recommendations" in search_result
        
        # Should find agent2
        collaborators = search_result["potential_collaborators"]
        agent2_info = next((c for c in collaborators if c["name"] == "agent2"), None)
        assert agent2_info is not None
        assert agent2_info["trust_level"] == 0.7
    
    def test_to_dict(self):
        agent = CognitiveAgent(
            name="test_agent",
            capabilities=["reasoning"],
            goals=["learn"],
            initial_beliefs={"state": "active"}
        )
        
        result = agent.to_dict()
        
        assert "name" in result
        assert result["name"] == "test_agent"
        assert "cognitive_properties" in result
        
        cognitive = result["cognitive_properties"]
        assert "capabilities" in cognitive
        assert "goals" in cognitive
        assert "beliefs" in cognitive
        assert "agentspace_stats" in cognitive
    
    def test_str_representation(self):
        agent = CognitiveAgent(
            name="test_agent",
            capabilities=["reasoning", "learning"],
            goals=["master_ai"]
        )
        
        str_repr = str(agent)
        assert "test_agent" in str_repr
        assert "2 capabilities" in str_repr
        assert "1 goals" in str_repr