import pytest
import asyncio
import json
from swarm.swarmcog import SwarmCog, SwarmCogConfig, create_swarmcog
from swarm.microkernel import ProcessingMode
from swarm.cognitive_agent import CognitiveAgent
from tests.mock_client import MockOpenAIClient


class TestSwarmCogConfig:
    def test_config_defaults(self):
        config = SwarmCogConfig()
        
        assert config.processing_mode == ProcessingMode.ASYNCHRONOUS
        assert config.cognitive_cycle_interval == 1.0
        assert config.max_agents == 50
        assert config.enable_distributed_processing == False
        assert config.log_level == "INFO"
        assert config.agentspace_name == "swarmcog_space"
    
    def test_config_custom(self):
        config = SwarmCogConfig(
            processing_mode=ProcessingMode.SYNCHRONOUS,
            cognitive_cycle_interval=0.5,
            max_agents=10,
            agentspace_name="custom_space"
        )
        
        assert config.processing_mode == ProcessingMode.SYNCHRONOUS
        assert config.cognitive_cycle_interval == 0.5
        assert config.max_agents == 10
        assert config.agentspace_name == "custom_space"
    
    def test_config_to_dict(self):
        config = SwarmCogConfig()
        result = config.to_dict()
        
        assert "processing_mode" in result
        assert "cognitive_cycle_interval" in result
        assert "max_agents" in result
        assert result["processing_mode"] == "asynchronous"


@pytest.fixture
def mock_client():
    return MockOpenAIClient()


class TestSwarmCog:
    def test_swarmcog_initialization(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        
        assert swarmcog.config is not None
        assert swarmcog.agentspace is not None
        assert swarmcog.microkernel is not None
        assert swarmcog.base_swarm is not None
        assert len(swarmcog.cognitive_agents) == 0
        assert not swarmcog.is_running
    
    def test_swarmcog_with_config(self, mock_client):
        config = SwarmCogConfig(
            agentspace_name="test_space",
            max_agents=5
        )
        swarmcog = SwarmCog(config, client=mock_client)
        
        assert swarmcog.config.agentspace_name == "test_space"
        assert swarmcog.config.max_agents == 5
        assert swarmcog.agentspace.name == "test_space"
    
    def test_create_cognitive_agent(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        
        agent = swarmcog.create_cognitive_agent(
            name="test_agent",
            capabilities=["reasoning", "learning"],
            goals=["solve_problems"],
            initial_beliefs={"ready": True}
        )
        
        assert isinstance(agent, CognitiveAgent)
        assert agent.name == "test_agent"
        assert "reasoning" in agent.capabilities
        assert "learning" in agent.capabilities
        assert "solve_problems" in agent.goals
        assert agent.beliefs["ready"] == True
        
        # Check agent is registered in swarmcog
        assert "test_agent" in swarmcog.cognitive_agents
        assert swarmcog.get_agent("test_agent") == agent
    
    def test_create_agent_duplicate_name(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        swarmcog.create_cognitive_agent("agent1")
        
        with pytest.raises(ValueError, match="Agent with name 'agent1' already exists"):
            swarmcog.create_cognitive_agent("agent1")
    
    def test_create_agent_max_limit(self, mock_client):
        config = SwarmCogConfig(max_agents=1)
        swarmcog = SwarmCog(config, client=mock_client)
        
        swarmcog.create_cognitive_agent("agent1")
        
        with pytest.raises(ValueError, match="Maximum number of agents"):
            swarmcog.create_cognitive_agent("agent2")
    
    def test_list_agents(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        swarmcog.create_cognitive_agent("agent1")
        swarmcog.create_cognitive_agent("agent2")
        
        agents = swarmcog.list_agents()
        assert len(agents) == 2
        assert "agent1" in agents
        assert "agent2" in agents
    
    def test_remove_agent(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        swarmcog.create_cognitive_agent("agent1")
        
        assert "agent1" in swarmcog.cognitive_agents
        
        result = swarmcog.remove_agent("agent1")
        assert result == True
        assert "agent1" not in swarmcog.cognitive_agents
        
        # Test removing non-existent agent
        result = swarmcog.remove_agent("nonexistent")
        assert result == False
    
    def test_coordinate_multi_agent_task(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        agent1 = swarmcog.create_cognitive_agent("agent1")
        agent2 = swarmcog.create_cognitive_agent("agent2")
        
        result = swarmcog.coordinate_multi_agent_task(
            "research_project",
            ["agent1", "agent2"],
            "collaborative"
        )
        
        assert "task_id" in result
        assert result["description"] == "research_project"
        assert result["participating_agents"] == ["agent1", "agent2"]
        assert result["strategy"] == "collaborative"
        assert "agent_assignments" in result
        assert "collaboration_links" in result
        
        # Check goals were added to agents
        assert any("contribute_to_task: research_project" in goal for goal in agent1.goals)
        assert any("contribute_to_task: research_project" in goal for goal in agent2.goals)
        
        # Check collaboration links created
        assert len(result["collaboration_links"]) == 1
        collaborators1 = agent1.get_collaborators()
        assert "agent2" in collaborators1
    
    def test_coordinate_task_nonexistent_agent(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        
        with pytest.raises(ValueError, match="Agent 'nonexistent' not found"):
            swarmcog.coordinate_multi_agent_task(
                "test_task",
                ["nonexistent"]
            )
    
    def test_get_agent_interactions(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        agent1 = swarmcog.create_cognitive_agent("agent1", goals=["collaborate"])
        agent2 = swarmcog.create_cognitive_agent("agent2")
        
        # Create some interaction
        agent1.establish_trust("agent2", 0.8)
        agent1.add_memory("interaction", {"with": "agent2"})
        
        # Test single agent interactions
        interactions = swarmcog.get_agent_interactions("agent1")
        
        assert "agent" in interactions
        assert interactions["agent"] == "agent1"
        assert "collaborators" in interactions
        assert "trust_levels" in interactions
        assert "recent_memories" in interactions
        assert "agent2" in interactions["trust_levels"]
        assert interactions["trust_levels"]["agent2"] == 0.8
        
        # Test all agents interactions
        all_interactions = swarmcog.get_agent_interactions()
        assert "agent1" in all_interactions
        assert "agent2" in all_interactions
        assert "active_goals" in all_interactions["agent1"]
    
    def test_get_swarm_topology(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        agent1 = swarmcog.create_cognitive_agent("agent1", capabilities=["reasoning"])
        agent2 = swarmcog.create_cognitive_agent("agent2", capabilities=["learning"])
        
        # Create relationships
        agent1.establish_trust("agent2", 0.7)
        agent1.coordinate_with_agent("agent2", "research")
        
        topology = swarmcog.get_swarm_topology()
        
        assert "agents" in topology
        assert "relationships" in topology
        assert "statistics" in topology
        
        # Check agents
        assert "agent1" in topology["agents"]
        assert "agent2" in topology["agents"]
        assert "reasoning" in topology["agents"]["agent1"]["capabilities"]
        
        # Check relationships
        assert len(topology["relationships"]["trust_relationships"]) >= 1
        assert len(topology["relationships"]["collaborations"]) >= 1
        
        # Check statistics
        assert topology["statistics"]["total_agents"] == 2
        assert topology["statistics"]["active_collaborations"] >= 1
    
    def test_get_system_status(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        swarmcog.create_cognitive_agent("agent1")
        
        status = swarmcog.get_system_status()
        
        assert "system" in status
        assert "agentspace" in status
        assert "cognitive_system" in status
        assert "agents" in status
        assert "topology" in status
        
        # Check system info
        assert "running" in status["system"]
        assert "config" in status["system"]
        
        # Check agents info
        assert status["agents"]["count"] == 1
        assert "agent1" in status["agents"]["names"]
    
    def test_export_system_state(self, mock_client):
        swarmcog = SwarmCog(client=mock_client)
        swarmcog.create_cognitive_agent("agent1")
        
        # Test export without file
        state = swarmcog.export_system_state()
        assert isinstance(state, dict)
        assert "system" in state
        
        # Test export to file
        import tempfile
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as f:
            filepath = f.name
        
        exported_state = swarmcog.export_system_state(filepath)
        
        # Verify file was created and contains valid JSON
        import os
        assert os.path.exists(filepath)
        
        with open(filepath, 'r') as f:
            loaded_state = json.load(f)
        
        assert loaded_state == exported_state
        
        # Cleanup
        os.unlink(filepath)
    
    @pytest.mark.asyncio
    async def test_autonomous_processing(self):
        swarmcog = SwarmCog()
        agent = swarmcog.create_cognitive_agent("agent1", goals=["test_goal"])
        
        assert not swarmcog.is_running
        
        # Start autonomous processing
        await swarmcog.start_autonomous_processing()
        assert swarmcog.is_running
        
        # Let it run briefly
        await asyncio.sleep(0.1)
        
        # Stop processing
        await swarmcog.stop_autonomous_processing()
        assert not swarmcog.is_running
    
    @pytest.mark.asyncio
    async def test_simulate_autonomous_behavior(self):
        swarmcog = SwarmCog()
        agent1 = swarmcog.create_cognitive_agent("agent1", goals=["collaborate"])
        agent2 = swarmcog.create_cognitive_agent("agent2", goals=["learn"])
        
        # Run short simulation
        results = await swarmcog.simulate_autonomous_behavior(duration_seconds=0.2)
        
        assert "duration" in results
        assert results["duration"] == 0.2
        assert "final_topology" in results
        assert "agent_final_states" in results
        
        # Check agent states
        assert "agent1" in results["agent_final_states"]
        assert "agent2" in results["agent_final_states"]
        
        agent1_state = results["agent_final_states"]["agent1"]
        assert "goals_count" in agent1_state
        assert "cognitive_phase" in agent1_state
    
    def test_str_representation(self, mock_client):
        config = SwarmCogConfig(agentspace_name="test_space")
        swarmcog = SwarmCog(config, client=mock_client)
        swarmcog.create_cognitive_agent("agent1")
        
        str_repr = str(swarmcog)
        assert "test_space" in str_repr
        assert "1 cognitive agents" in str_repr
        assert "STOPPED" in str_repr


class TestCreateSwarmCog:
    def test_create_swarmcog_defaults(self):
        # Note: create_swarmcog doesn't take a client parameter, it creates its own
        # We'll need to mock the OpenAI client at a different level for this test
        import os
        # Set a fake API key to avoid OpenAI client initialization error
        os.environ['OPENAI_API_KEY'] = 'fake-key-for-testing'
        
        try:
            swarmcog = create_swarmcog()
            
            assert isinstance(swarmcog, SwarmCog)
            assert swarmcog.config.agentspace_name == "swarmcog_space"
            assert swarmcog.config.processing_mode == ProcessingMode.ASYNCHRONOUS
        finally:
            # Clean up
            if 'OPENAI_API_KEY' in os.environ:
                del os.environ['OPENAI_API_KEY']
    
    def test_create_swarmcog_custom(self):
        # Set a fake API key to avoid OpenAI client initialization error
        import os
        os.environ['OPENAI_API_KEY'] = 'fake-key-for-testing'
        
        try:
            swarmcog = create_swarmcog(
                agentspace_name="custom_space",
                processing_mode=ProcessingMode.SYNCHRONOUS,
                max_agents=10
            )
            
            assert swarmcog.config.agentspace_name == "custom_space"
            assert swarmcog.config.processing_mode == ProcessingMode.SYNCHRONOUS
            assert swarmcog.config.max_agents == 10
        finally:
            # Clean up
            if 'OPENAI_API_KEY' in os.environ:
                del os.environ['OPENAI_API_KEY']