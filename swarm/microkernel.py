"""
Cognitive Agentic Microkernel for SwarmCog framework.

This module implements the core cognitive processing system that enables
autonomous multi-agent coordination through cognitive cycles, reasoning,
and adaptive behavior.
"""

from typing import Any, Dict, List, Optional, Callable, Union
from abc import ABC, abstractmethod
from enum import Enum
from dataclasses import dataclass, field
import asyncio
import time
import threading
from collections import defaultdict, deque
import json
import logging
from concurrent.futures import ThreadPoolExecutor

from .agentspace import AgentSpace, Atom, Node, Link, AtomType, TruthValue, AttentionValue
from .types import Agent


class CognitivePhase(Enum):
    """Phases of the cognitive cycle."""
    PERCEPTION = "perception"
    ATTENTION = "attention"
    REASONING = "reasoning"
    PLANNING = "planning"
    EXECUTION = "execution"
    LEARNING = "learning"
    REFLECTION = "reflection"


class ProcessingMode(Enum):
    """Processing modes for cognitive operations."""
    SYNCHRONOUS = "synchronous"
    ASYNCHRONOUS = "asynchronous"
    DISTRIBUTED = "distributed"


@dataclass
class CognitiveState:
    """Represents the current cognitive state of an agent."""
    agent_id: str
    current_phase: CognitivePhase = CognitivePhase.PERCEPTION
    goals: List[str] = field(default_factory=list)
    beliefs: Dict[str, Any] = field(default_factory=dict)
    intentions: List[str] = field(default_factory=list)
    working_memory: Dict[str, Any] = field(default_factory=dict)
    attention_focus: List[str] = field(default_factory=list)  # Atom IDs
    activation_level: float = 1.0  # [0.0, 1.0]
    timestamp: float = field(default_factory=time.time)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "agent_id": self.agent_id,
            "current_phase": self.current_phase.value,
            "goals": self.goals,
            "beliefs": self.beliefs,
            "intentions": self.intentions,
            "working_memory": self.working_memory,
            "attention_focus": self.attention_focus,
            "activation_level": self.activation_level,
            "timestamp": self.timestamp
        }


class CognitiveProcessor(ABC):
    """Abstract base class for cognitive processors."""
    
    @abstractmethod
    async def process(self, state: CognitiveState, agentspace: AgentSpace) -> CognitiveState:
        """Process the cognitive state and return updated state."""
        pass
    
    @abstractmethod
    def get_phase(self) -> CognitivePhase:
        """Return the cognitive phase this processor handles."""
        pass


class PerceptionProcessor(CognitiveProcessor):
    """Processes sensory input and environmental changes."""
    
    def get_phase(self) -> CognitivePhase:
        return CognitivePhase.PERCEPTION
    
    async def process(self, state: CognitiveState, agentspace: AgentSpace) -> CognitiveState:
        """Process perception - gather information from environment and other agents."""
        # Update beliefs based on agentspace observations
        agent_nodes = agentspace.get_agents()
        state.beliefs["visible_agents"] = [node.name for node in agent_nodes 
                                         if node.name != state.agent_id]
        
        # Detect environmental changes
        recent_atoms = [atom for atom in agentspace.atoms.values() 
                       if time.time() - atom.timestamp < 60]  # Last minute
        state.beliefs["recent_changes"] = len(recent_atoms)
        
        # Update working memory with current perceptions
        state.working_memory["last_perception"] = time.time()
        state.current_phase = CognitivePhase.ATTENTION
        
        return state


class AttentionProcessor(CognitiveProcessor):
    """Manages attention and focus selection."""
    
    def get_phase(self) -> CognitivePhase:
        return CognitivePhase.ATTENTION
    
    async def process(self, state: CognitiveState, agentspace: AgentSpace) -> CognitiveState:
        """Process attention - select what to focus on."""
        # Get most important atoms based on attention values
        important_atoms = agentspace.get_most_important_atoms(limit=5)
        state.attention_focus = [atom.id for atom in important_atoms]
        
        # Boost attention for goal-related atoms
        for goal in state.goals:
            goal_atoms = agentspace.find_atoms(name=goal)
            for atom in goal_atoms[:2]:  # Top 2 goal-related atoms
                if atom.id not in state.attention_focus:
                    state.attention_focus.append(atom.id)
                agentspace.update_attention(atom.id, sti_delta=0.1)
        
        state.current_phase = CognitivePhase.REASONING
        return state


class ReasoningProcessor(CognitiveProcessor):
    """Handles logical reasoning and inference."""
    
    def get_phase(self) -> CognitivePhase:
        return CognitivePhase.REASONING
    
    async def process(self, state: CognitiveState, agentspace: AgentSpace) -> CognitiveState:
        """Process reasoning - make inferences and update beliefs."""
        # Simple reasoning: infer collaboration opportunities
        agent_node = None
        for node in agentspace.get_agents():
            if node.name == state.agent_id:
                agent_node = node
                break
        
        if agent_node:
            collaborators = agentspace.get_collaborators(agent_node)
            state.beliefs["potential_collaborators"] = [c.name for c in collaborators]
            
            # Reason about trust relationships
            trust_levels = {}
            for collab in collaborators:
                trust = agentspace.get_trust_level(agent_node, collab)
                if trust is not None:
                    trust_levels[collab.name] = trust
            state.beliefs["trust_levels"] = trust_levels
        
        # Update working memory with reasoning results
        state.working_memory["last_reasoning"] = time.time()
        state.current_phase = CognitivePhase.PLANNING
        
        return state


class PlanningProcessor(CognitiveProcessor):
    """Handles goal decomposition and planning."""
    
    def get_phase(self) -> CognitivePhase:
        return CognitivePhase.PLANNING
    
    async def process(self, state: CognitiveState, agentspace: AgentSpace) -> CognitiveState:
        """Process planning - create action plans for goals."""
        # Simple planning: decompose goals into intentions
        new_intentions = []
        
        for goal in state.goals:
            if goal not in state.intentions:
                # Simple goal decomposition
                if "collaborate" in goal.lower():
                    new_intentions.extend([
                        f"find_collaborator_for_{goal}",
                        f"establish_trust_for_{goal}",
                        f"coordinate_action_for_{goal}"
                    ])
                elif "learn" in goal.lower():
                    new_intentions.extend([
                        f"gather_information_for_{goal}",
                        f"process_knowledge_for_{goal}",
                        f"validate_learning_for_{goal}"
                    ])
                else:
                    new_intentions.append(f"execute_{goal}")
        
        state.intentions.extend(new_intentions)
        
        # Prioritize intentions based on goals
        state.intentions = state.intentions[:10]  # Keep top 10 intentions
        
        state.working_memory["last_planning"] = time.time()
        state.current_phase = CognitivePhase.EXECUTION
        
        return state


class ExecutionProcessor(CognitiveProcessor):
    """Handles action execution and coordination."""
    
    def get_phase(self) -> CognitivePhase:
        return CognitivePhase.EXECUTION
    
    async def process(self, state: CognitiveState, agentspace: AgentSpace) -> CognitiveState:
        """Process execution - carry out planned actions."""
        executed_intentions = []
        
        for intention in state.intentions[:3]:  # Execute top 3 intentions
            if "find_collaborator" in intention:
                # Look for potential collaborators in agentspace
                agent_node = None
                for node in agentspace.get_agents():
                    if node.name == state.agent_id:
                        agent_node = node
                        break
                
                if agent_node:
                    # Find agents with complementary capabilities
                    other_agents = [a for a in agentspace.get_agents() if a != agent_node]
                    if other_agents:
                        # Create collaboration link with first available agent
                        target_agent = other_agents[0]
                        agentspace.add_collaboration_link(agent_node, target_agent, "cognitive_task")
                        state.beliefs[f"collaborated_with_{target_agent.name}"] = True
                        executed_intentions.append(intention)
            
            elif "establish_trust" in intention:
                # Establish trust with collaborators
                agent_node = None
                for node in agentspace.get_agents():
                    if node.name == state.agent_id:
                        agent_node = node
                        break
                
                if agent_node:
                    collaborators = agentspace.get_collaborators(agent_node)
                    for collab in collaborators:
                        current_trust = agentspace.get_trust_level(agent_node, collab)
                        if current_trust is None:
                            agentspace.add_trust_relationship(agent_node, collab, 0.7)
                            executed_intentions.append(intention)
            
            else:
                # Generic intention execution
                state.working_memory[f"executed_{intention}"] = time.time()
                executed_intentions.append(intention)
        
        # Remove executed intentions
        state.intentions = [i for i in state.intentions if i not in executed_intentions]
        
        state.working_memory["last_execution"] = time.time()
        state.current_phase = CognitivePhase.LEARNING
        
        return state


class LearningProcessor(CognitiveProcessor):
    """Handles learning and knowledge acquisition."""
    
    def get_phase(self) -> CognitivePhase:
        return CognitivePhase.LEARNING
    
    async def process(self, state: CognitiveState, agentspace: AgentSpace) -> CognitiveState:
        """Process learning - update knowledge based on experiences."""
        # Learn from collaboration outcomes
        for key, value in state.beliefs.items():
            if key.startswith("collaborated_with_") and value:
                agent_name = key.replace("collaborated_with_", "")
                
                # Create memory node for this collaboration
                memory_node = Node(AtomType.MEMORY_NODE, f"collaboration_memory_{agent_name}")
                memory_node.metadata = {
                    "type": "collaboration",
                    "partner": agent_name,
                    "timestamp": time.time(),
                    "outcome": "positive"  # Default to positive
                }
                agentspace.add_atom(memory_node)
                
                # Increase trust if collaboration was successful
                agent_node = None
                for node in agentspace.get_agents():
                    if node.name == state.agent_id:
                        agent_node = node
                        break
                
                if agent_node:
                    partner_node = None
                    for node in agentspace.get_agents():
                        if node.name == agent_name:
                            partner_node = node
                            break
                    
                    if partner_node:
                        current_trust = agentspace.get_trust_level(agent_node, partner_node)
                        if current_trust is not None and current_trust < 0.9:
                            # Increase trust gradually
                            new_trust = min(0.95, current_trust + 0.1)
                            agentspace.add_trust_relationship(agent_node, partner_node, new_trust)
        
        state.working_memory["last_learning"] = time.time()
        state.current_phase = CognitivePhase.REFLECTION
        
        return state


class ReflectionProcessor(CognitiveProcessor):
    """Handles metacognitive reflection and adaptation."""
    
    def get_phase(self) -> CognitivePhase:
        return CognitivePhase.REFLECTION
    
    async def process(self, state: CognitiveState, agentspace: AgentSpace) -> CognitiveState:
        """Process reflection - evaluate performance and adapt."""
        # Reflect on goal achievement
        achieved_goals = []
        for goal in state.goals:
            # Simple heuristic: goal is achieved if we have positive beliefs about it
            goal_beliefs = [k for k in state.beliefs.keys() if goal.lower() in k.lower()]
            if len(goal_beliefs) > 0:
                achieved_goals.append(goal)
        
        # Remove achieved goals
        state.goals = [g for g in state.goals if g not in achieved_goals]
        
        # Adapt activation level based on recent activity
        recent_activities = sum(1 for key in state.working_memory.keys() 
                              if "last_" in key and 
                              time.time() - state.working_memory[key] < 60)
        
        # Adjust activation level based on activity
        if recent_activities > 3:
            state.activation_level = min(1.0, state.activation_level + 0.1)
        elif recent_activities < 2:
            state.activation_level = max(0.1, state.activation_level - 0.05)
        
        state.working_memory["last_reflection"] = time.time()
        state.current_phase = CognitivePhase.PERCEPTION  # Back to perception
        
        return state


class CognitiveMicrokernel:
    """
    The core cognitive processing system for SwarmCog agents.
    
    Implements a cognitive cycle with multiple processing phases and
    supports autonomous multi-agent coordination through shared AgentSpace.
    """
    
    def __init__(self, agentspace: AgentSpace, processing_mode: ProcessingMode = ProcessingMode.ASYNCHRONOUS):
        self.agentspace = agentspace
        self.processing_mode = processing_mode
        self.processors: Dict[CognitivePhase, CognitiveProcessor] = {}
        self.agent_states: Dict[str, CognitiveState] = {}
        self.running = False
        self.cycle_tasks: Dict[str, asyncio.Task] = {}
        self.logger = logging.getLogger(__name__)
        
        # Initialize default processors
        self._init_default_processors()
        
        # Thread pool for synchronous operations
        self.executor = ThreadPoolExecutor(max_workers=4)
    
    def _init_default_processors(self):
        """Initialize the default cognitive processors."""
        self.processors[CognitivePhase.PERCEPTION] = PerceptionProcessor()
        self.processors[CognitivePhase.ATTENTION] = AttentionProcessor()
        self.processors[CognitivePhase.REASONING] = ReasoningProcessor()
        self.processors[CognitivePhase.PLANNING] = PlanningProcessor()
        self.processors[CognitivePhase.EXECUTION] = ExecutionProcessor()
        self.processors[CognitivePhase.LEARNING] = LearningProcessor()
        self.processors[CognitivePhase.REFLECTION] = ReflectionProcessor()
    
    def register_processor(self, processor: CognitiveProcessor):
        """Register a custom cognitive processor."""
        self.processors[processor.get_phase()] = processor
    
    def add_cognitive_agent(self, agent_id: str, goals: List[str] = None, 
                           initial_beliefs: Dict[str, Any] = None) -> CognitiveState:
        """Add an agent to the cognitive processing system."""
        state = CognitiveState(
            agent_id=agent_id,
            goals=goals or [],
            beliefs=initial_beliefs or {}
        )
        self.agent_states[agent_id] = state
        
        # Add agent node to agentspace if not exists
        existing_agents = [a.name for a in self.agentspace.get_agents()]
        if agent_id not in existing_agents:
            self.agentspace.add_agent_node(agent_id)
        
        self.logger.info(f"Added cognitive agent: {agent_id}")
        return state
    
    def remove_cognitive_agent(self, agent_id: str):
        """Remove an agent from the cognitive processing system."""
        if agent_id in self.agent_states:
            del self.agent_states[agent_id]
        
        if agent_id in self.cycle_tasks:
            self.cycle_tasks[agent_id].cancel()
            del self.cycle_tasks[agent_id]
        
        self.logger.info(f"Removed cognitive agent: {agent_id}")
    
    def get_agent_state(self, agent_id: str) -> Optional[CognitiveState]:
        """Get the current cognitive state of an agent."""
        return self.agent_states.get(agent_id)
    
    async def process_cognitive_cycle(self, agent_id: str) -> CognitiveState:
        """Execute one complete cognitive cycle for an agent."""
        if agent_id not in self.agent_states:
            raise ValueError(f"Agent {agent_id} not found in cognitive system")
        
        state = self.agent_states[agent_id]
        processor = self.processors.get(state.current_phase)
        
        if processor:
            try:
                new_state = await processor.process(state, self.agentspace)
                self.agent_states[agent_id] = new_state
                self.logger.debug(f"Processed {state.current_phase.value} for agent {agent_id}")
                return new_state
            except Exception as e:
                self.logger.error(f"Error processing {state.current_phase.value} for agent {agent_id}: {e}")
                return state
        else:
            self.logger.warning(f"No processor found for phase {state.current_phase.value}")
            return state
    
    async def run_continuous_cycle(self, agent_id: str, cycle_interval: float = 1.0):
        """Run continuous cognitive cycles for an agent."""
        self.logger.info(f"Starting continuous cognitive cycle for agent {agent_id}")
        
        while self.running and agent_id in self.agent_states:
            try:
                await self.process_cognitive_cycle(agent_id)
                await asyncio.sleep(cycle_interval)
            except asyncio.CancelledError:
                self.logger.info(f"Cognitive cycle cancelled for agent {agent_id}")
                break
            except Exception as e:
                self.logger.error(f"Error in continuous cycle for agent {agent_id}: {e}")
                await asyncio.sleep(cycle_interval)
    
    async def start_all_agents(self, cycle_interval: float = 1.0):
        """Start cognitive cycles for all registered agents."""
        self.running = True
        
        for agent_id in self.agent_states.keys():
            if agent_id not in self.cycle_tasks:
                task = asyncio.create_task(
                    self.run_continuous_cycle(agent_id, cycle_interval)
                )
                self.cycle_tasks[agent_id] = task
        
        self.logger.info(f"Started cognitive cycles for {len(self.cycle_tasks)} agents")
    
    async def stop_all_agents(self):
        """Stop cognitive cycles for all agents."""
        self.running = False
        
        for task in self.cycle_tasks.values():
            task.cancel()
        
        # Wait for all tasks to complete
        if self.cycle_tasks:
            await asyncio.gather(*self.cycle_tasks.values(), return_exceptions=True)
        
        self.cycle_tasks.clear()
        self.logger.info("Stopped all cognitive cycles")
    
    def get_system_state(self) -> Dict[str, Any]:
        """Get the current state of the entire cognitive system."""
        return {
            "microkernel": {
                "running": self.running,
                "processing_mode": self.processing_mode.value,
                "active_agents": len(self.agent_states),
                "running_cycles": len(self.cycle_tasks)
            },
            "agentspace": self.agentspace.to_dict(),
            "agent_states": {agent_id: state.to_dict() 
                           for agent_id, state in self.agent_states.items()}
        }
    
    def update_agent_goals(self, agent_id: str, goals: List[str]):
        """Update the goals for a specific agent."""
        if agent_id in self.agent_states:
            self.agent_states[agent_id].goals = goals
            self.logger.info(f"Updated goals for agent {agent_id}: {goals}")
    
    def update_agent_beliefs(self, agent_id: str, beliefs: Dict[str, Any]):
        """Update the beliefs for a specific agent."""
        if agent_id in self.agent_states:
            self.agent_states[agent_id].beliefs.update(beliefs)
            self.logger.info(f"Updated beliefs for agent {agent_id}")
    
    def __del__(self):
        """Cleanup when microkernel is destroyed."""
        if hasattr(self, 'executor'):
            self.executor.shutdown(wait=False)