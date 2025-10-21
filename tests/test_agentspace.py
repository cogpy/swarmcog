import pytest
from swarm.agentspace import AgentSpace, AtomType, Node, Link, TruthValue, AttentionValue
import time


class TestTruthValue:
    def test_truth_value_initialization(self):
        tv = TruthValue(0.8, 0.9)
        assert tv.strength == 0.8
        assert tv.confidence == 0.9
    
    def test_truth_value_bounds(self):
        # Test upper bounds
        tv = TruthValue(1.5, 2.0)
        assert tv.strength == 1.0
        assert tv.confidence == 1.0
        
        # Test lower bounds
        tv = TruthValue(-0.5, -1.0)
        assert tv.strength == 0.0
        assert tv.confidence == 0.0
    
    def test_truth_value_to_dict(self):
        tv = TruthValue(0.7, 0.6)
        result = tv.to_dict()
        assert result == {"strength": 0.7, "confidence": 0.6}


class TestAttentionValue:
    def test_attention_value_initialization(self):
        av = AttentionValue(0.5, -0.3, 0.8)
        assert av.sti == 0.5
        assert av.lti == -0.3
        assert av.vlti == 0.8
    
    def test_attention_value_bounds(self):
        # Test upper and lower bounds for sti and lti
        av = AttentionValue(2.0, -2.0, 1.5)
        assert av.sti == 1.0
        assert av.lti == -1.0
        assert av.vlti == 1.0
        
        # Test lower bound for vlti
        av = AttentionValue(0.0, 0.0, -0.5)
        assert av.vlti == 0.0


class TestNode:
    def test_node_creation(self):
        node = Node(AtomType.AGENT_NODE, "test_agent")
        assert node.type == AtomType.AGENT_NODE
        assert node.name == "test_agent"
        assert isinstance(node.truth_value, TruthValue)
        assert isinstance(node.attention_value, AttentionValue)
    
    def test_node_with_value(self):
        node = Node(AtomType.BELIEF_NODE, "test_belief", {"content": "test"})
        assert node.value == {"content": "test"}
    
    def test_node_to_dict(self):
        node = Node(AtomType.CAPABILITY_NODE, "test_capability")
        result = node.to_dict()
        
        assert result["type"] == "CapabilityNode"
        assert result["name"] == "test_capability"
        assert "id" in result
        assert "truth_value" in result
        assert "attention_value" in result
    
    def test_node_str(self):
        node = Node(AtomType.GOAL_NODE, "test_goal")
        assert str(node) == "GoalNode(test_goal)"


class TestLink:
    def test_link_creation(self):
        node1 = Node(AtomType.AGENT_NODE, "agent1")
        node2 = Node(AtomType.AGENT_NODE, "agent2")
        link = Link(AtomType.COLLABORATION_LINK, [node1, node2])
        
        assert link.type == AtomType.COLLABORATION_LINK
        assert link.arity() == 2
        assert node1 in link.outgoing
        assert node2 in link.outgoing
    
    def test_link_to_dict(self):
        node1 = Node(AtomType.AGENT_NODE, "agent1")
        node2 = Node(AtomType.CAPABILITY_NODE, "capability1")
        link = Link(AtomType.KNOWLEDGE_LINK, [node1, node2])
        
        result = link.to_dict()
        assert result["type"] == "KnowledgeLink"
        assert len(result["outgoing"]) == 2
        assert node1.id in result["outgoing"]
        assert node2.id in result["outgoing"]


class TestAgentSpace:
    def test_agentspace_creation(self):
        agentspace = AgentSpace("test_space")
        assert agentspace.name == "test_space"
        assert len(agentspace) == 0
    
    def test_add_atom(self):
        agentspace = AgentSpace()
        node = Node(AtomType.AGENT_NODE, "test_agent")
        
        result = agentspace.add_atom(node)
        assert result == node
        assert len(agentspace) == 1
        assert agentspace.get_atom(node.id) == node
    
    def test_add_duplicate_atom(self):
        agentspace = AgentSpace()
        node = Node(AtomType.AGENT_NODE, "test_agent")
        
        result1 = agentspace.add_atom(node)
        result2 = agentspace.add_atom(node)
        
        assert result1 == result2
        assert len(agentspace) == 1
    
    def test_find_atoms_by_type(self):
        agentspace = AgentSpace()
        agent1 = Node(AtomType.AGENT_NODE, "agent1")
        agent2 = Node(AtomType.AGENT_NODE, "agent2")
        goal = Node(AtomType.GOAL_NODE, "goal1")
        
        agentspace.add_atom(agent1)
        agentspace.add_atom(agent2)
        agentspace.add_atom(goal)
        
        agents = agentspace.find_atoms(AtomType.AGENT_NODE)
        assert len(agents) == 2
        assert agent1 in agents
        assert agent2 in agents
        
        goals = agentspace.find_atoms(AtomType.GOAL_NODE)
        assert len(goals) == 1
        assert goal in goals
    
    def test_find_atoms_by_name(self):
        agentspace = AgentSpace()
        agent = Node(AtomType.AGENT_NODE, "test_agent")
        capability = Node(AtomType.CAPABILITY_NODE, "test_capability")
        
        agentspace.add_atom(agent)
        agentspace.add_atom(capability)
        
        found = agentspace.find_atoms(name="test_agent")
        assert len(found) == 1
        assert found[0] == agent
    
    def test_add_agent_node_with_capabilities(self):
        agentspace = AgentSpace()
        capabilities = ["reasoning", "learning", "collaboration"]
        
        agent = agentspace.add_agent_node("test_agent", capabilities)
        
        assert agent.name == "test_agent"
        assert agent.type == AtomType.AGENT_NODE
        
        # Check capabilities were added
        cap_nodes = agentspace.find_atoms(AtomType.CAPABILITY_NODE)
        assert len(cap_nodes) == 3
        
        cap_names = [node.name for node in cap_nodes]
        for cap in capabilities:
            assert cap in cap_names
    
    def test_collaboration_link(self):
        agentspace = AgentSpace()
        agent1 = agentspace.add_agent_node("agent1")
        agent2 = agentspace.add_agent_node("agent2")
        
        collab_link = agentspace.add_collaboration_link(agent1, agent2, "test_collaboration")
        
        assert collab_link.type == AtomType.COLLABORATION_LINK
        assert collab_link.metadata["collaboration_type"] == "test_collaboration"
        assert agent1 in collab_link.outgoing
        assert agent2 in collab_link.outgoing
        
        # Test get_collaborators
        collaborators = agentspace.get_collaborators(agent1)
        assert len(collaborators) == 1
        assert agent2 in collaborators
    
    def test_trust_relationship(self):
        agentspace = AgentSpace()
        agent1 = agentspace.add_agent_node("agent1")
        agent2 = agentspace.add_agent_node("agent2")
        
        trust_level = 0.8
        trust_link = agentspace.add_trust_relationship(agent1, agent2, trust_level)
        
        assert trust_link.type == AtomType.TRUST_LINK
        assert trust_link.truth_value.strength == trust_level
        
        # Test get_trust_level
        retrieved_trust = agentspace.get_trust_level(agent1, agent2)
        assert retrieved_trust == trust_level
    
    def test_incoming_links(self):
        agentspace = AgentSpace()
        agent = Node(AtomType.AGENT_NODE, "test_agent")
        capability = Node(AtomType.CAPABILITY_NODE, "test_capability")
        
        agentspace.add_atom(agent)
        agentspace.add_atom(capability)
        
        link = Link(AtomType.KNOWLEDGE_LINK, [agent, capability])
        agentspace.add_atom(link)
        
        # Test incoming links for capability
        incoming = agentspace.get_incoming(capability)
        assert len(incoming) == 1
        assert incoming[0] == link
    
    def test_attention_updates(self):
        agentspace = AgentSpace()
        node = Node(AtomType.AGENT_NODE, "test_agent")
        agentspace.add_atom(node)
        
        original_sti = node.attention_value.sti
        agentspace.update_attention(node.id, sti_delta=0.2)
        
        assert node.attention_value.sti == original_sti + 0.2
    
    def test_most_important_atoms(self):
        agentspace = AgentSpace()
        
        # Create atoms with different attention values
        atom1 = Node(AtomType.AGENT_NODE, "agent1")
        atom1.attention_value.sti = 0.9
        atom1.attention_value.lti = 0.8
        
        atom2 = Node(AtomType.AGENT_NODE, "agent2")
        atom2.attention_value.sti = 0.3
        atom2.attention_value.lti = 0.2
        
        atom3 = Node(AtomType.GOAL_NODE, "goal1")
        atom3.attention_value.sti = 0.7
        atom3.attention_value.lti = 0.6
        
        agentspace.add_atom(atom1)
        agentspace.add_atom(atom2)
        agentspace.add_atom(atom3)
        
        important = agentspace.get_most_important_atoms(limit=2)
        
        assert len(important) == 2
        # atom1 should be first (highest combined attention)
        assert important[0] == atom1
    
    def test_agentspace_to_dict(self):
        agentspace = AgentSpace("test_space")
        agent = agentspace.add_agent_node("test_agent", ["capability1"])
        
        result = agentspace.to_dict()
        
        assert result["name"] == "test_space"
        assert "atoms" in result
        assert "statistics" in result
        assert result["statistics"]["total_atoms"] > 0
    
    def test_agentspace_str(self):
        agentspace = AgentSpace("test_space")
        agentspace.add_agent_node("agent1")
        
        str_repr = str(agentspace)
        assert "test_space" in str_repr
        assert "atoms" in str_repr