"""
SwarmCog: Autonomous Multi-Agent Orchestration Framework

This module implements the main SwarmCog system that coordinates cognitive agents
using AgentSpace knowledge representation and Cognitive Microkernel processing.
"""

from typing import Any, Dict, List, Optional, Union, Callable
import asyncio
import time
import json
import logging
from dataclasses import dataclass, field

from .core import Swarm
from .types import Agent, Response
from .agentspace import AgentSpace, AtomType, Node, Link
from .microkernel import CognitiveMicrokernel, ProcessingMode, CognitiveState
from .cognitive_agent import CognitiveAgent


@dataclass
class SwarmCogConfig:
    """Configuration for SwarmCog system."""
    processing_mode: ProcessingMode = ProcessingMode.ASYNCHRONOUS
    cognitive_cycle_interval: float = 1.0
    max_agents: int = 50
    enable_distributed_processing: bool = False
    log_level: str = "INFO"
    agentspace_name: str = "swarmcog_space"
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "processing_mode": self.processing_mode.value,
            "cognitive_cycle_interval": self.cognitive_cycle_interval,
            "max_agents": self.max_agents,
            "enable_distributed_processing": self.enable_distributed_processing,
            "log_level": self.log_level,
            "agentspace_name": self.agentspace_name
        }


class SwarmCog:
    """
    SwarmCog: Autonomous Multi-Agent Orchestration Framework
    
    Implements OpenCog-inspired cognitive multi-agent coordination using:
    - AgentSpace: Agentic AtomSpace for knowledge representation
    - Cognitive Microkernel: Autonomous reasoning and coordination
    - Cognitive Agents: Enhanced agents with autonomous capabilities
    """
    
    def __init__(self, config: SwarmCogConfig = None, client=None):
        self.config = config or SwarmCogConfig()
        
        # Initialize base Swarm for compatibility
        self.base_swarm = Swarm(client=client)
        
        # Initialize cognitive components
        self.agentspace = AgentSpace(name=self.config.agentspace_name)
        self.microkernel = CognitiveMicrokernel(
            self.agentspace, 
            self.config.processing_mode
        )
        
        # Agent management
        self.cognitive_agents: Dict[str, CognitiveAgent] = {}
        self.agent_tasks: Dict[str, asyncio.Task] = {}
        
        # System state
        self.is_running = False
        self.system_start_time = None
        self.total_interactions = 0
        
        # Logging
        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(getattr(logging, self.config.log_level))
        
        # Initialize system
        self._initialize_system()
    
    def _initialize_system(self):
        """Initialize the SwarmCog system."""
        self.logger.info("Initializing SwarmCog autonomous multi-agent orchestration framework")
        self.logger.info(f"AgentSpace: {self.agentspace.name}")
        self.logger.info(f"Processing Mode: {self.config.processing_mode.value}")
        self.logger.info(f"Max Agents: {self.config.max_agents}")
    
    def create_cognitive_agent(self, name: str, model: str = "gpt-4o",
                              instructions: Union[str, Callable] = None,
                              capabilities: List[str] = None,
                              goals: List[str] = None,
                              initial_beliefs: Dict[str, Any] = None,
                              functions: List = None) -> CognitiveAgent:
        """Create a new cognitive agent and add it to the swarm."""
        
        if len(self.cognitive_agents) >= self.config.max_agents:
            raise ValueError(f"Maximum number of agents ({self.config.max_agents}) reached")
        
        if name in self.cognitive_agents:
            raise ValueError(f"Agent with name '{name}' already exists")
        
        # Default instructions for cognitive agents
        if instructions is None:
            instructions = (
                f"You are {name}, a cognitive agent in the SwarmCog framework. "
                "You can reason autonomously, collaborate with other agents, learn from experiences, "
                "and coordinate complex multi-agent tasks. Use your cognitive functions to perceive, "
                "reason, plan, and execute actions while building trust and sharing knowledge with other agents."
            )
        
        # Create cognitive agent
        agent = CognitiveAgent(
            name=name,
            model=model,
            instructions=instructions,
            functions=functions or [],
            agentspace=self.agentspace,
            microkernel=self.microkernel
        )
        
        # Add capabilities
        if capabilities:
            for cap in capabilities:
                agent.add_capability(cap, f"Capability: {cap}")
        
        # Set goals
        if goals:
            for goal in goals:
                agent.add_goal(goal)
        
        # Set initial beliefs
        if initial_beliefs:
            for belief_name, belief_value in initial_beliefs.items():
                agent.update_belief(belief_name, belief_value)
        
        # Register agent
        self.cognitive_agents[name] = agent
        
        self.logger.info(f"Created cognitive agent: {name}")
        self.logger.info(f"  Capabilities: {capabilities or []}")
        self.logger.info(f"  Goals: {goals or []}")
        
        return agent
    
    def get_agent(self, name: str) -> Optional[CognitiveAgent]:
        """Get a cognitive agent by name."""
        return self.cognitive_agents.get(name)
    
    def list_agents(self) -> List[str]:
        """List all cognitive agent names."""
        return list(self.cognitive_agents.keys())
    
    def remove_agent(self, name: str) -> bool:
        """Remove a cognitive agent from the swarm."""
        if name in self.cognitive_agents:
            agent = self.cognitive_agents[name]
            
            # Stop cognitive processing if running
            if self.is_running:
                asyncio.create_task(agent.stop_cognitive_processing())
            
            # Remove from microkernel
            self.microkernel.remove_cognitive_agent(name)
            
            # Remove from registry
            del self.cognitive_agents[name]
            
            self.logger.info(f"Removed cognitive agent: {name}")
            return True
        
        return False
    
    async def start_autonomous_processing(self):
        """Start autonomous cognitive processing for all agents."""
        if self.is_running:
            self.logger.warning("Autonomous processing already running")
            return
        
        self.is_running = True
        self.system_start_time = time.time()
        
        # Start cognitive processing for all agents
        for agent in self.cognitive_agents.values():
            await agent.start_cognitive_processing(self.config.cognitive_cycle_interval)
        
        self.logger.info(f"Started autonomous processing for {len(self.cognitive_agents)} agents")
    
    async def stop_autonomous_processing(self):
        """Stop autonomous cognitive processing for all agents."""
        if not self.is_running:
            self.logger.warning("Autonomous processing not running")
            return
        
        self.is_running = False
        
        # Stop cognitive processing for all agents
        for agent in self.cognitive_agents.values():
            await agent.stop_cognitive_processing()
        
        self.logger.info("Stopped autonomous processing")
    
    def run_agent_conversation(self, agent_name: str, messages: List[Dict[str, str]], 
                             context_variables: Dict = None, **kwargs) -> Response:
        """Run a conversation with a specific cognitive agent using base Swarm compatibility."""
        
        agent = self.cognitive_agents.get(agent_name)
        if not agent:
            raise ValueError(f"Agent '{agent_name}' not found")
        
        # Use base swarm for conversation
        response = self.base_swarm.run(
            agent=agent,
            messages=messages,
            context_variables=context_variables or {},
            **kwargs
        )
        
        # Update interaction count
        self.total_interactions += 1
        
        # Log cognitive state if available
        cognitive_state = agent.get_cognitive_state()
        if cognitive_state:
            self.logger.debug(f"Agent {agent_name} cognitive phase: {cognitive_state.current_phase.value}")
        
        return response
    
    def coordinate_multi_agent_task(self, task_description: str, 
                                  participating_agents: List[str] = None,
                                  coordination_strategy: str = "collaborative") -> Dict[str, Any]:
        """Coordinate a multi-agent task across the swarm."""
        
        if participating_agents is None:
            participating_agents = list(self.cognitive_agents.keys())
        
        # Validate agents exist
        for agent_name in participating_agents:
            if agent_name not in self.cognitive_agents:
                raise ValueError(f"Agent '{agent_name}' not found")
        
        # Create coordination task
        coordination_result = {
            "task_id": f"task_{int(time.time())}",
            "description": task_description,
            "participating_agents": participating_agents,
            "strategy": coordination_strategy,
            "start_time": time.time(),
            "status": "initiated",
            "agent_assignments": {},
            "collaboration_links": []
        }
        
        # Assign task goals to participating agents
        for agent_name in participating_agents:
            agent = self.cognitive_agents[agent_name]
            task_goal = f"contribute_to_task: {task_description}"
            agent.add_goal(task_goal)
            coordination_result["agent_assignments"][agent_name] = task_goal
        
        # Create collaboration links between agents
        agents_nodes = []
        for agent_name in participating_agents:
            agent_nodes = [n for n in self.agentspace.get_agents() if n.name == agent_name]
            if agent_nodes:
                agents_nodes.append((agent_name, agent_nodes[0]))
        
        # Create pairwise collaboration links
        for i, (name1, node1) in enumerate(agents_nodes):
            for name2, node2 in agents_nodes[i+1:]:
                link = self.agentspace.add_collaboration_link(node1, node2, task_description)
                coordination_result["collaboration_links"].append({
                    "agent1": name1,
                    "agent2": name2,
                    "link_id": link.id
                })
        
        self.logger.info(f"Coordinated multi-agent task: {task_description}")
        self.logger.info(f"  Participating agents: {participating_agents}")
        self.logger.info(f"  Created {len(coordination_result['collaboration_links'])} collaboration links")
        
        return coordination_result
    
    def get_agent_interactions(self, agent_name: str = None) -> Dict[str, Any]:
        """Get interaction history and relationships for an agent or all agents."""
        
        if agent_name:
            agent = self.cognitive_agents.get(agent_name)
            if not agent:
                raise ValueError(f"Agent '{agent_name}' not found")
            
            return {
                "agent": agent_name,
                "collaborators": agent.get_collaborators(),
                "trust_levels": agent.get_trust_levels(),
                "recent_memories": agent.memories[-10:],  # Last 10 memories
                "cognitive_state": agent.get_cognitive_state().to_dict() if agent.get_cognitive_state() else None
            }
        else:
            # Return interactions for all agents
            interactions = {}
            for name, agent in self.cognitive_agents.items():
                interactions[name] = {
                    "collaborators": agent.get_collaborators(),
                    "trust_levels": agent.get_trust_levels(),
                    "active_goals": agent.goals,
                    "capabilities": list(agent.capabilities.keys())
                }
            return interactions
    
    def get_swarm_topology(self) -> Dict[str, Any]:
        """Get the current topology and relationships in the swarm."""
        topology = {
            "agents": {},
            "relationships": {
                "collaborations": [],
                "trust_relationships": [],
                "knowledge_links": []
            },
            "statistics": {
                "total_agents": len(self.cognitive_agents),
                "total_atoms": len(self.agentspace),
                "active_collaborations": 0,
                "average_trust_level": 0.0
            }
        }
        
        # Agent details
        for name, agent in self.cognitive_agents.items():
            topology["agents"][name] = {
                "capabilities": list(agent.capabilities.keys()),
                "goals": agent.goals,
                "beliefs_count": len(agent.beliefs),
                "memories_count": len(agent.memories),
                "cognitive_state": agent.get_cognitive_state().current_phase.value if agent.get_cognitive_state() else None
            }
        
        # Extract relationships from agentspace
        trust_levels = []
        for atom in self.agentspace.atoms.values():
            if isinstance(atom, Link):
                if atom.type == AtomType.COLLABORATION_LINK:
                    if len(atom.outgoing) >= 2:
                        topology["relationships"]["collaborations"].append({
                            "agent1": atom.outgoing[0].name,
                            "agent2": atom.outgoing[1].name,
                            "type": atom.metadata.get("collaboration_type", "general")
                        })
                        topology["statistics"]["active_collaborations"] += 1
                
                elif atom.type == AtomType.TRUST_LINK:
                    if len(atom.outgoing) >= 2:
                        trust_level = atom.truth_value.strength
                        topology["relationships"]["trust_relationships"].append({
                            "trustor": atom.outgoing[0].name,
                            "trustee": atom.outgoing[1].name,
                            "trust_level": trust_level
                        })
                        trust_levels.append(trust_level)
                
                elif atom.type == AtomType.KNOWLEDGE_LINK:
                    if len(atom.outgoing) >= 2:
                        topology["relationships"]["knowledge_links"].append({
                            "source": atom.outgoing[0].name,
                            "target": atom.outgoing[1].name,
                            "knowledge_type": atom.outgoing[1].type.value
                        })
        
        # Calculate statistics
        if trust_levels:
            topology["statistics"]["average_trust_level"] = sum(trust_levels) / len(trust_levels)
        
        return topology
    
    def get_system_status(self) -> Dict[str, Any]:
        """Get comprehensive system status."""
        uptime = time.time() - self.system_start_time if self.system_start_time else 0
        
        status = {
            "system": {
                "running": self.is_running,
                "uptime_seconds": uptime,
                "total_interactions": self.total_interactions,
                "config": self.config.to_dict()
            },
            "agentspace": self.agentspace.to_dict(),
            "cognitive_system": self.microkernel.get_system_state(),
            "agents": {
                "count": len(self.cognitive_agents),
                "names": list(self.cognitive_agents.keys()),
                "active_cognitive_cycles": len([a for a in self.cognitive_agents.values() 
                                              if a.is_cognitive_active])
            },
            "topology": self.get_swarm_topology()
        }
        
        return status
    
    def export_system_state(self, filepath: str = None) -> Dict[str, Any]:
        """Export the complete system state to a file or return as dict."""
        system_state = self.get_system_status()
        
        if filepath:
            with open(filepath, 'w') as f:
                json.dump(system_state, f, indent=2, default=str)
            self.logger.info(f"System state exported to {filepath}")
        
        return system_state
    
    async def simulate_autonomous_behavior(self, duration_seconds: float = 60.0):
        """Run a simulation of autonomous multi-agent behavior."""
        
        if not self.cognitive_agents:
            self.logger.warning("No agents available for simulation")
            return
        
        self.logger.info(f"Starting autonomous behavior simulation for {duration_seconds} seconds")
        
        # Start autonomous processing
        await self.start_autonomous_processing()
        
        try:
            # Let agents run autonomously
            await asyncio.sleep(duration_seconds)
        finally:
            # Stop processing
            await self.stop_autonomous_processing()
        
        # Return simulation results
        results = {
            "duration": duration_seconds,
            "final_topology": self.get_swarm_topology(),
            "agent_final_states": {}
        }
        
        for name, agent in self.cognitive_agents.items():
            state = agent.get_cognitive_state()
            results["agent_final_states"][name] = {
                "goals_count": len(agent.goals),
                "beliefs_count": len(agent.beliefs),
                "memories_count": len(agent.memories),
                "collaborators_count": len(agent.get_collaborators()),
                "cognitive_phase": state.current_phase.value if state else None,
                "activation_level": state.activation_level if state else 0.0
            }
        
        self.logger.info("Autonomous behavior simulation completed")
        return results
    
    def __str__(self) -> str:
        return (f"SwarmCog({self.config.agentspace_name}): "
                f"{len(self.cognitive_agents)} cognitive agents, "
                f"{len(self.agentspace)} atoms, "
                f"{'RUNNING' if self.is_running else 'STOPPED'}")
    
    def __repr__(self) -> str:
        return self.__str__()


# Convenience function for creating SwarmCog instances
def create_swarmcog(agentspace_name: str = "swarmcog_space", 
                   processing_mode: ProcessingMode = ProcessingMode.ASYNCHRONOUS,
                   **config_kwargs) -> SwarmCog:
    """Create a SwarmCog instance with specified configuration."""
    
    config = SwarmCogConfig(
        agentspace_name=agentspace_name,
        processing_mode=processing_mode,
        **config_kwargs
    )
    
    return SwarmCog(config=config)