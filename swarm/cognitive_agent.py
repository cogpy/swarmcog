"""
Cognitive Agent implementation for SwarmCog framework.

This module extends the basic Agent class with cognitive capabilities,
integrating with the AgentSpace and Cognitive Microkernel for autonomous
multi-agent coordination and reasoning.
"""

from typing import Any, Dict, List, Optional, Callable, Union
import asyncio
import time
import json
from dataclasses import dataclass

from .types import Agent
from .agentspace import AgentSpace, AtomType, Node, Link, TruthValue
from .microkernel import CognitiveMicrokernel, CognitiveState, ProcessingMode


@dataclass
class CognitiveCapability:
    """Represents a cognitive capability of an agent."""
    name: str
    description: str
    strength: float = 0.5  # [0.0, 1.0]
    experience: int = 0
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "description": self.description,
            "strength": self.strength,
            "experience": self.experience
        }


class CognitiveAgent(Agent):
    """
    Enhanced Agent class with cognitive capabilities for autonomous multi-agent coordination.
    
    Integrates with AgentSpace for knowledge representation and Cognitive Microkernel
    for autonomous reasoning, planning, and coordination with other agents.
    """
    
    def __init__(self, name: str = "CognitiveAgent", model: str = "gpt-4o",
                 instructions: Union[str, Callable[[], str]] = "You are a cognitive agent capable of autonomous reasoning and collaboration.",
                 functions: List = None, agentspace: AgentSpace = None, 
                 microkernel: CognitiveMicrokernel = None, **kwargs):
        
        # Initialize base Agent
        super().__init__(
            name=name,
            model=model,
            instructions=instructions,
            functions=functions or [],
            **kwargs
        )
        
        # Cognitive components (use object.__setattr__ to bypass pydantic validation)
        object.__setattr__(self, 'agentspace', agentspace or AgentSpace(name=f"{name}_space"))
        object.__setattr__(self, 'microkernel', microkernel or CognitiveMicrokernel(
            self.agentspace, ProcessingMode.ASYNCHRONOUS
        ))
        
        # Cognitive properties (use object.__setattr__ to bypass pydantic validation)
        object.__setattr__(self, 'capabilities', {})
        object.__setattr__(self, 'goals', [])
        object.__setattr__(self, 'beliefs', {})
        object.__setattr__(self, 'memories', [])
        object.__setattr__(self, 'collaboration_history', {})
        
        # Internal state
        object.__setattr__(self, 'cognitive_state', None)
        object.__setattr__(self, 'is_cognitive_active', False)
        
        # Register agent in cognitive system
        self._register_cognitive_agent()
        
        # Add cognitive functions to the agent
        self._add_cognitive_functions()
    
    def _register_cognitive_agent(self):
        """Register this agent with the cognitive microkernel and agentspace."""
        # Add to agentspace
        agent_node = self.agentspace.add_agent_node(
            self.name, 
            capabilities=list(self.capabilities.keys())
        )
        
        # Register with microkernel
        cognitive_state = self.microkernel.add_cognitive_agent(
            self.name, 
            goals=self.goals.copy(),
            initial_beliefs=self.beliefs.copy()
        )
        object.__setattr__(self, 'cognitive_state', cognitive_state)
    
    def _add_cognitive_functions(self):
        """Add cognitive functions that can be called by the base agent."""
        cognitive_functions = [
            self.perceive_environment,
            self.reason_about_goals,
            self.plan_collaboration,
            self.execute_cognitive_action,
            self.reflect_on_performance,
            self.learn_from_experience,
            self.find_collaborators,
            self.establish_trust,
            self.share_knowledge,
            self.coordinate_with_agent
        ]
        
        self.functions.extend(cognitive_functions)
    
    def add_capability(self, name: str, description: str, strength: float = 0.5) -> CognitiveCapability:
        """Add a cognitive capability to the agent."""
        capability = CognitiveCapability(name, description, strength)
        # Use a copy and replace to avoid modifying the pydantic field directly
        new_capabilities = self.capabilities.copy()
        new_capabilities[name] = capability
        object.__setattr__(self, 'capabilities', new_capabilities)
        
        # Add capability to agentspace
        cap_node = Node(AtomType.CAPABILITY_NODE, name)
        cap_node.metadata = capability.to_dict()
        self.agentspace.add_atom(cap_node)
        
        # Link agent to capability
        agent_nodes = [n for n in self.agentspace.get_agents() if n.name == self.name]
        if agent_nodes:
            cap_link = Link(AtomType.KNOWLEDGE_LINK, [agent_nodes[0], cap_node])
            self.agentspace.add_atom(cap_link)
        
        return capability
    
    def add_goal(self, goal: str, priority: float = 0.5):
        """Add a goal to the agent's goal set."""
        if goal not in self.goals:
            # Use a copy and replace to avoid modifying the pydantic field directly
            new_goals = self.goals.copy()
            new_goals.append(goal)
            object.__setattr__(self, 'goals', new_goals)
            
            # Add goal to agentspace
            goal_node = Node(AtomType.GOAL_NODE, goal)
            goal_node.truth_value.strength = priority
            self.agentspace.add_atom(goal_node)
            
            # Update microkernel
            if self.cognitive_state:
                self.microkernel.update_agent_goals(self.name, self.goals)
    
    def update_belief(self, belief_name: str, belief_value: Any):
        """Update a belief in the agent's belief system."""
        # Use a copy and replace to avoid modifying the pydantic field directly
        new_beliefs = self.beliefs.copy()
        new_beliefs[belief_name] = belief_value
        object.__setattr__(self, 'beliefs', new_beliefs)
        
        # Add belief to agentspace
        belief_node = Node(AtomType.BELIEF_NODE, belief_name)
        belief_node.value = belief_value
        belief_node.timestamp = time.time()
        self.agentspace.add_atom(belief_node)
        
        # Update microkernel
        if self.cognitive_state:
            self.microkernel.update_agent_beliefs(self.name, {belief_name: belief_value})
    
    def add_memory(self, memory_type: str, content: Any, importance: float = 0.5):
        """Add a memory to the agent's memory system."""
        memory = {
            "type": memory_type,
            "content": content,
            "importance": importance,
            "timestamp": time.time()
        }
        # Use a copy and replace to avoid modifying the pydantic field directly
        new_memories = self.memories.copy()
        new_memories.append(memory)
        object.__setattr__(self, 'memories', new_memories)
        
        # Add memory to agentspace
        memory_node = Node(AtomType.MEMORY_NODE, f"{memory_type}_{len(self.memories)}")
        memory_node.metadata = memory
        memory_node.attention_value.lti = importance
        self.agentspace.add_atom(memory_node)
    
    async def start_cognitive_processing(self, cycle_interval: float = 1.0):
        """Start the cognitive processing cycle for this agent."""
        if not self.is_cognitive_active:
            object.__setattr__(self, 'is_cognitive_active', True)
            await self.microkernel.start_all_agents(cycle_interval)
    
    async def stop_cognitive_processing(self):
        """Stop the cognitive processing cycle for this agent."""
        if self.is_cognitive_active:
            object.__setattr__(self, 'is_cognitive_active', False)
            await self.microkernel.stop_all_agents()
    
    def get_cognitive_state(self) -> Optional[CognitiveState]:
        """Get the current cognitive state of the agent."""
        return self.microkernel.get_agent_state(self.name)
    
    def get_collaborators(self) -> List[str]:
        """Get list of current collaborators."""
        agent_nodes = [n for n in self.agentspace.get_agents() if n.name == self.name]
        if agent_nodes:
            collaborators = self.agentspace.get_collaborators(agent_nodes[0])
            return [c.name for c in collaborators]
        return []
    
    def get_trust_levels(self) -> Dict[str, float]:
        """Get trust levels with other agents."""
        trust_levels = {}
        agent_nodes = [n for n in self.agentspace.get_agents() if n.name == self.name]
        if agent_nodes:
            agent_node = agent_nodes[0]
            all_agents = self.agentspace.get_agents()
            for other_agent in all_agents:
                if other_agent != agent_node:
                    trust = self.agentspace.get_trust_level(agent_node, other_agent)
                    if trust is not None:
                        trust_levels[other_agent.name] = trust
        return trust_levels
    
    # Cognitive Functions (can be called by the base agent)
    
    def perceive_environment(self) -> str:
        """Perceive the current environment and other agents."""
        agents = [a.name for a in self.agentspace.get_agents() if a.name != self.name]
        recent_atoms = len([a for a in self.agentspace.atoms.values() 
                          if time.time() - a.timestamp < 300])  # Last 5 minutes
        
        perception = {
            "visible_agents": agents,
            "recent_activity": recent_atoms,
            "agentspace_size": len(self.agentspace),
            "timestamp": time.time()
        }
        
        self.update_belief("current_perception", perception)
        return json.dumps(perception, indent=2)
    
    def reason_about_goals(self) -> str:
        """Reason about current goals and their feasibility."""
        if not self.goals:
            return "No goals currently set."
        
        reasoning = {
            "active_goals": self.goals,
            "goal_analysis": [],
            "recommendations": []
        }
        
        for goal in self.goals:
            # Simple goal analysis
            analysis = {
                "goal": goal,
                "feasibility": 0.7,  # Default
                "required_collaborators": [],
                "estimated_effort": "medium"
            }
            
            if "collaborate" in goal.lower():
                collaborators = self.get_collaborators()
                analysis["required_collaborators"] = collaborators[:2]
                analysis["feasibility"] = 0.8 if collaborators else 0.3
            
            reasoning["goal_analysis"].append(analysis)
        
        # Generate recommendations
        if len(self.get_collaborators()) < 2:
            reasoning["recommendations"].append("Seek more collaboration opportunities")
        
        return json.dumps(reasoning, indent=2)
    
    def plan_collaboration(self, goal: str = None) -> str:
        """Plan collaboration for achieving goals."""
        target_goal = goal or (self.goals[0] if self.goals else "general_collaboration")
        
        plan = {
            "goal": target_goal,
            "collaboration_strategy": [],
            "potential_partners": [],
            "action_steps": []
        }
        
        # Find potential partners
        all_agents = [a.name for a in self.agentspace.get_agents() if a.name != self.name]
        trust_levels = self.get_trust_levels()
        
        # Sort by trust level
        potential_partners = []
        for agent_name in all_agents:
            trust = trust_levels.get(agent_name, 0.5)
            potential_partners.append((agent_name, trust))
        
        potential_partners.sort(key=lambda x: x[1], reverse=True)
        plan["potential_partners"] = [p[0] for p in potential_partners[:3]]
        
        # Create action steps
        if potential_partners:
            plan["action_steps"] = [
                f"Initiate contact with {potential_partners[0][0]}",
                f"Propose collaboration on {target_goal}",
                "Establish communication protocol",
                "Define roles and responsibilities",
                "Execute collaborative actions"
            ]
        
        return json.dumps(plan, indent=2)
    
    def execute_cognitive_action(self, action: str) -> str:
        """Execute a cognitive action based on current state."""
        state = self.get_cognitive_state()
        if not state:
            return "No cognitive state available"
        
        result = {
            "action": action,
            "current_phase": state.current_phase.value,
            "execution_result": "success",
            "timestamp": time.time()
        }
        
        # Simple action execution
        if "collaborate" in action.lower():
            collaborators = self.get_collaborators()
            if collaborators:
                result["details"] = f"Initiated collaboration with {collaborators[0]}"
                self.add_memory("collaboration", {"action": action, "partner": collaborators[0]})
            else:
                result["execution_result"] = "failed"
                result["details"] = "No collaborators available"
        
        elif "learn" in action.lower():
            result["details"] = "Initiated learning process"
            self.add_memory("learning", {"action": action})
        
        else:
            result["details"] = f"Executed general action: {action}"
        
        return json.dumps(result, indent=2)
    
    def reflect_on_performance(self) -> str:
        """Reflect on recent performance and adapt behavior."""
        state = self.get_cognitive_state()
        if not state:
            return "No cognitive state available for reflection"
        
        reflection = {
            "current_activation": state.activation_level,
            "completed_goals": [],
            "learning_insights": [],
            "behavioral_adjustments": []
        }
        
        # Analyze recent memories
        recent_memories = [m for m in self.memories if time.time() - m["timestamp"] < 3600]  # Last hour
        
        collaboration_memories = [m for m in recent_memories if m["type"] == "collaboration"]
        if collaboration_memories:
            reflection["learning_insights"].append(
                f"Engaged in {len(collaboration_memories)} collaborations recently"
            )
        
        # Performance evaluation
        if state.activation_level > 0.8:
            reflection["behavioral_adjustments"].append("Maintain high activity level")
        elif state.activation_level < 0.3:
            reflection["behavioral_adjustments"].append("Increase engagement and activity")
        
        return json.dumps(reflection, indent=2)
    
    def learn_from_experience(self, experience: str = None) -> str:
        """Learn from recent experiences and update knowledge."""
        learning_result = {
            "experience": experience or "general_experience",
            "insights": [],
            "knowledge_updates": [],
            "capability_improvements": []
        }
        
        # Analyze collaboration history
        collaborators = self.get_collaborators()
        trust_levels = self.get_trust_levels()
        
        for collaborator in collaborators:
            trust = trust_levels.get(collaborator, 0.5)
            if trust > 0.7:
                learning_result["insights"].append(
                    f"Strong trust relationship with {collaborator}"
                )
                learning_result["knowledge_updates"].append(
                    f"Reliable collaborator: {collaborator}"
                )
        
        # Update capabilities based on experience
        for cap_name, capability in self.capabilities.items():
            if capability.experience > 5:
                capability.strength = min(1.0, capability.strength + 0.1)
                learning_result["capability_improvements"].append(
                    f"Improved {cap_name} capability"
                )
        
        return json.dumps(learning_result, indent=2)
    
    def find_collaborators(self, capability_needed: str = None) -> str:
        """Find potential collaborators based on capabilities or trust."""
        search_result = {
            "search_criteria": capability_needed or "general_collaboration",
            "potential_collaborators": [],
            "recommendations": []
        }
        
        all_agents = self.agentspace.get_agents()
        trust_levels = self.get_trust_levels()
        
        for agent in all_agents:
            if agent.name != self.name:
                agent_info = {
                    "name": agent.name,
                    "trust_level": trust_levels.get(agent.name, 0.0),
                    "capabilities": []
                }
                
                # Get agent capabilities from agentspace
                cap_links = self.agentspace.get_incoming(agent)
                for link in cap_links:
                    if link.type == AtomType.KNOWLEDGE_LINK:
                        for atom in link.outgoing:
                            if atom.type == AtomType.CAPABILITY_NODE:
                                agent_info["capabilities"].append(atom.name)
                
                search_result["potential_collaborators"].append(agent_info)
        
        # Sort by trust level
        search_result["potential_collaborators"].sort(
            key=lambda x: x["trust_level"], reverse=True
        )
        
        # Generate recommendations
        top_collaborators = search_result["potential_collaborators"][:2]
        for collaborator in top_collaborators:
            search_result["recommendations"].append(
                f"Consider collaborating with {collaborator['name']} (trust: {collaborator['trust_level']:.2f})"
            )
        
        return json.dumps(search_result, indent=2)
    
    def establish_trust(self, agent_name: str, trust_level: float = 0.7) -> str:
        """Establish or update trust with another agent."""
        agent_nodes = [n for n in self.agentspace.get_agents() if n.name == self.name]
        target_nodes = [n for n in self.agentspace.get_agents() if n.name == agent_name]
        
        if not agent_nodes or not target_nodes:
            return json.dumps({"error": "Agent not found", "agent": agent_name})
        
        agent_node = agent_nodes[0]
        target_node = target_nodes[0]
        
        # Establish trust relationship
        self.agentspace.add_trust_relationship(agent_node, target_node, trust_level)
        
        result = {
            "action": "establish_trust",
            "target_agent": agent_name,
            "trust_level": trust_level,
            "timestamp": time.time(),
            "status": "success"
        }
        
        self.add_memory("trust_establishment", result)
        return json.dumps(result, indent=2)
    
    def share_knowledge(self, knowledge_type: str, content: Any, 
                       target_agent: str = None) -> str:
        """Share knowledge with other agents."""
        knowledge = {
            "type": knowledge_type,
            "content": content,
            "source": self.name,
            "timestamp": time.time()
        }
        
        # Create knowledge node in agentspace
        knowledge_node = Node(AtomType.KNOWLEDGE_LINK, f"{knowledge_type}_{int(time.time())}")
        knowledge_node.metadata = knowledge
        self.agentspace.add_atom(knowledge_node)
        
        sharing_result = {
            "action": "share_knowledge",
            "knowledge_type": knowledge_type,
            "target": target_agent or "all_agents",
            "shared_at": time.time(),
            "status": "success"
        }
        
        self.add_memory("knowledge_sharing", sharing_result)
        return json.dumps(sharing_result, indent=2)
    
    def coordinate_with_agent(self, agent_name: str, coordination_type: str = "general") -> str:
        """Coordinate actions with another specific agent."""
        agent_nodes = [n for n in self.agentspace.get_agents() if n.name == self.name]
        target_nodes = [n for n in self.agentspace.get_agents() if n.name == agent_name]
        
        if not agent_nodes or not target_nodes:
            return json.dumps({"error": "Agent not found for coordination", "agent": agent_name})
        
        # Create collaboration link
        self.agentspace.add_collaboration_link(
            agent_nodes[0], target_nodes[0], coordination_type
        )
        
        coordination_result = {
            "action": "coordinate_with_agent",
            "partner": agent_name,
            "coordination_type": coordination_type,
            "timestamp": time.time(),
            "status": "success"
        }
        
        self.add_memory("coordination", coordination_result)
        return json.dumps(coordination_result, indent=2)
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert the cognitive agent to a dictionary representation."""
        base_dict = super().model_dump() if hasattr(super(), 'model_dump') else {
            "name": self.name,
            "model": self.model,
            "instructions": self.instructions if isinstance(self.instructions, str) else "Dynamic instructions",
            "functions": [f.__name__ if callable(f) else str(f) for f in self.functions]
        }
        
        cognitive_dict = {
            "capabilities": {name: cap.to_dict() for name, cap in self.capabilities.items()},
            "goals": self.goals,
            "beliefs": self.beliefs,
            "memories": self.memories[-10:],  # Last 10 memories
            "collaborators": self.get_collaborators(),
            "trust_levels": self.get_trust_levels(),
            "cognitive_state": self.get_cognitive_state().to_dict() if self.get_cognitive_state() else None,
            "agentspace_stats": {
                "total_atoms": len(self.agentspace),
                "agent_count": len(self.agentspace.get_agents())
            }
        }
        
        return {**base_dict, "cognitive_properties": cognitive_dict}
    
    def __str__(self) -> str:
        return f"CognitiveAgent({self.name}): {len(self.capabilities)} capabilities, {len(self.goals)} goals, {len(self.get_collaborators())} collaborators"