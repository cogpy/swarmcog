"""
AgentSpace: Agentic AtomSpace for SwarmCog cognitive multi-agent framework.

This module implements a knowledge representation system inspired by OpenCog's AtomSpace
but specifically designed for multi-agent cognitive processing and coordination.
"""

from typing import Any, Dict, List, Optional, Set, Union, Callable
from abc import ABC, abstractmethod
from enum import Enum
from dataclasses import dataclass, field
import uuid
import time
from threading import Lock
from collections import defaultdict
import json


class AtomType(Enum):
    """Types of atoms in the AgentSpace."""
    # Basic types
    NODE = "Node"
    LINK = "Link"
    
    # Agent-specific types
    AGENT_NODE = "AgentNode"
    CAPABILITY_NODE = "CapabilityNode"
    GOAL_NODE = "GoalNode"
    BELIEF_NODE = "BeliefNode"
    MEMORY_NODE = "MemoryNode"
    
    # Relationship types
    COLLABORATION_LINK = "CollaborationLink"
    DELEGATION_LINK = "DelegationLink"
    TRUST_LINK = "TrustLink"
    KNOWLEDGE_LINK = "KnowledgeLink"
    EVALUATION_LINK = "EvaluationLink"


@dataclass
class TruthValue:
    """Represents the truth value of an atom with strength and confidence."""
    strength: float = 0.5  # [0.0, 1.0]
    confidence: float = 0.0  # [0.0, 1.0]
    
    def __post_init__(self):
        self.strength = max(0.0, min(1.0, self.strength))
        self.confidence = max(0.0, min(1.0, self.confidence))
    
    def to_dict(self) -> Dict[str, float]:
        return {"strength": self.strength, "confidence": self.confidence}


@dataclass
class AttentionValue:
    """Represents the attention value of an atom for cognitive processing."""
    sti: float = 0.0  # Short-term importance [-1.0, 1.0]
    lti: float = 0.0  # Long-term importance [-1.0, 1.0] 
    vlti: float = 0.0  # Very long-term importance [0.0, 1.0]
    
    def __post_init__(self):
        self.sti = max(-1.0, min(1.0, self.sti))
        self.lti = max(-1.0, min(1.0, self.lti))
        self.vlti = max(0.0, min(1.0, self.vlti))
    
    def to_dict(self) -> Dict[str, float]:
        return {"sti": self.sti, "lti": self.lti, "vlti": self.vlti}


class Atom(ABC):
    """Base class for all atoms in the AgentSpace."""
    
    def __init__(self, atom_type: AtomType, name: Optional[str] = None):
        self.id = str(uuid.uuid4())
        self.type = atom_type
        self.name = name or f"{atom_type.value}_{self.id[:8]}"
        self.truth_value = TruthValue()
        self.attention_value = AttentionValue()
        self.timestamp = time.time()
        self.metadata: Dict[str, Any] = {}
    
    @abstractmethod
    def to_dict(self) -> Dict[str, Any]:
        """Serialize atom to dictionary."""
        pass
    
    @abstractmethod
    def __str__(self) -> str:
        pass
    
    def __eq__(self, other):
        return isinstance(other, Atom) and self.id == other.id
    
    def __hash__(self):
        return hash(self.id)


class Node(Atom):
    """Represents a node atom in the AgentSpace."""
    
    def __init__(self, atom_type: AtomType, name: str, value: Any = None):
        super().__init__(atom_type, name)
        self.value = value
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "type": self.type.value,
            "name": self.name,
            "value": self.value,
            "truth_value": self.truth_value.to_dict(),
            "attention_value": self.attention_value.to_dict(),
            "timestamp": self.timestamp,
            "metadata": self.metadata
        }
    
    def __str__(self) -> str:
        return f"{self.type.value}({self.name})"


class Link(Atom):
    """Represents a link atom connecting other atoms in the AgentSpace."""
    
    def __init__(self, atom_type: AtomType, outgoing: List[Atom], name: Optional[str] = None):
        super().__init__(atom_type, name)
        self.outgoing = outgoing
    
    def arity(self) -> int:
        """Returns the number of atoms this link connects."""
        return len(self.outgoing)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "type": self.type.value,
            "name": self.name,
            "outgoing": [atom.id for atom in self.outgoing],
            "truth_value": self.truth_value.to_dict(),
            "attention_value": self.attention_value.to_dict(),
            "timestamp": self.timestamp,
            "metadata": self.metadata
        }
    
    def __str__(self) -> str:
        outgoing_str = ", ".join(str(atom) for atom in self.outgoing)
        return f"{self.type.value}({outgoing_str})"


class AgentSpace:
    """
    AgentSpace: A knowledge representation system for cognitive multi-agent coordination.
    
    This is inspired by OpenCog's AtomSpace but designed specifically for agent cognitive
    processing, collaboration, and autonomous coordination.
    """
    
    def __init__(self, name: str = "default"):
        self.name = name
        self.atoms: Dict[str, Atom] = {}
        self.type_index: Dict[AtomType, Set[str]] = defaultdict(set)
        self.incoming_index: Dict[str, Set[str]] = defaultdict(set)  # atom_id -> set of link_ids
        self.name_index: Dict[str, Set[str]] = defaultdict(set)  # name -> set of atom_ids
        self._lock = Lock()
    
    def add_atom(self, atom: Atom) -> Atom:
        """Add an atom to the AgentSpace."""
        with self._lock:
            if atom.id in self.atoms:
                return self.atoms[atom.id]
            
            self.atoms[atom.id] = atom
            self.type_index[atom.type].add(atom.id)
            self.name_index[atom.name].add(atom.id)
            
            # Update incoming index for links
            if isinstance(atom, Link):
                for target_atom in atom.outgoing:
                    self.incoming_index[target_atom.id].add(atom.id)
            
            return atom
    
    def get_atom(self, atom_id: str) -> Optional[Atom]:
        """Retrieve an atom by its ID."""
        return self.atoms.get(atom_id)
    
    def find_atoms(self, atom_type: Optional[AtomType] = None, 
                   name: Optional[str] = None) -> List[Atom]:
        """Find atoms by type and/or name."""
        result_ids = set(self.atoms.keys())
        
        if atom_type is not None:
            result_ids &= self.type_index[atom_type]
        
        if name is not None:
            result_ids &= self.name_index[name]
        
        return [self.atoms[atom_id] for atom_id in result_ids]
    
    def get_incoming(self, atom: Atom) -> List[Link]:
        """Get all links that have this atom in their outgoing set."""
        link_ids = self.incoming_index[atom.id]
        return [self.atoms[link_id] for link_id in link_ids if isinstance(self.atoms[link_id], Link)]
    
    def get_agents(self) -> List[Node]:
        """Get all agent nodes in the AgentSpace."""
        return [atom for atom in self.find_atoms(AtomType.AGENT_NODE) if isinstance(atom, Node)]
    
    def add_agent_node(self, agent_name: str, capabilities: List[str] = None) -> Node:
        """Add an agent node with optional capabilities."""
        agent_node = Node(AtomType.AGENT_NODE, agent_name)
        self.add_atom(agent_node)
        
        # Add capabilities if provided
        if capabilities:
            for cap in capabilities:
                cap_node = Node(AtomType.CAPABILITY_NODE, cap)
                self.add_atom(cap_node)
                
                # Link agent to capability
                cap_link = Link(AtomType.KNOWLEDGE_LINK, [agent_node, cap_node])
                self.add_atom(cap_link)
        
        return agent_node
    
    def add_collaboration_link(self, agent1: Node, agent2: Node, 
                             collaboration_type: str = "general") -> Link:
        """Create a collaboration link between two agents."""
        link = Link(AtomType.COLLABORATION_LINK, [agent1, agent2])
        link.metadata["collaboration_type"] = collaboration_type
        return self.add_atom(link)
    
    def add_trust_relationship(self, trustor: Node, trustee: Node, trust_level: float) -> Link:
        """Add a trust relationship between agents."""
        trust_link = Link(AtomType.TRUST_LINK, [trustor, trustee])
        trust_link.truth_value.strength = max(0.0, min(1.0, trust_level))
        trust_link.truth_value.confidence = 0.8  # Default confidence
        return self.add_atom(trust_link)
    
    def get_collaborators(self, agent: Node) -> List[Node]:
        """Get all agents that collaborate with the given agent."""
        collaborators = []
        for link in self.get_incoming(agent):
            if link.type == AtomType.COLLABORATION_LINK:
                for other_agent in link.outgoing:
                    if other_agent != agent and other_agent.type == AtomType.AGENT_NODE:
                        collaborators.append(other_agent)
        return collaborators
    
    def get_trust_level(self, trustor: Node, trustee: Node) -> Optional[float]:
        """Get the trust level between two agents."""
        for link in self.find_atoms(AtomType.TRUST_LINK):
            if (isinstance(link, Link) and 
                len(link.outgoing) == 2 and 
                link.outgoing[0] == trustor and 
                link.outgoing[1] == trustee):
                return link.truth_value.strength
        return None
    
    def update_attention(self, atom_id: str, sti_delta: float = 0, 
                        lti_delta: float = 0, vlti_delta: float = 0):
        """Update attention values for an atom."""
        atom = self.get_atom(atom_id)
        if atom:
            with self._lock:
                atom.attention_value.sti = max(-1.0, min(1.0, 
                    atom.attention_value.sti + sti_delta))
                atom.attention_value.lti = max(-1.0, min(1.0, 
                    atom.attention_value.lti + lti_delta))
                atom.attention_value.vlti = max(0.0, min(1.0, 
                    atom.attention_value.vlti + vlti_delta))
    
    def get_most_important_atoms(self, limit: int = 10) -> List[Atom]:
        """Get atoms with highest attention values."""
        atoms_with_attention = [(atom, atom.attention_value.sti + atom.attention_value.lti) 
                              for atom in self.atoms.values()]
        atoms_with_attention.sort(key=lambda x: x[1], reverse=True)
        return [atom for atom, _ in atoms_with_attention[:limit]]
    
    def to_dict(self) -> Dict[str, Any]:
        """Serialize the entire AgentSpace to a dictionary."""
        return {
            "name": self.name,
            "atoms": {atom_id: atom.to_dict() for atom_id, atom in self.atoms.items()},
            "statistics": {
                "total_atoms": len(self.atoms),
                "atom_types": {atom_type.value: len(atom_ids) 
                             for atom_type, atom_ids in self.type_index.items()},
                "timestamp": time.time()
            }
        }
    
    def __len__(self) -> int:
        return len(self.atoms)
    
    def __str__(self) -> str:
        return f"AgentSpace({self.name}): {len(self.atoms)} atoms"