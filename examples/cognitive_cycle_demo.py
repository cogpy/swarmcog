#!/usr/bin/env python3
"""
Cognitive Cycle Demonstration

This example demonstrates the 7-phase cognitive cycle in action by creating
a simple scenario where cognitive agents collaborate on a research project.

Run this example to see each cognitive phase in action and understand
how the processing modes affect agent behavior.
"""

import asyncio
import time
import json
from typing import Dict, Any

from swarm import AgentSpace, CognitiveMicrokernel, CognitiveAgent, ProcessingMode


class CognitiveCycleDemo:
    """Demonstrates the cognitive cycle phases with detailed logging."""
    
    def __init__(self, processing_mode: ProcessingMode = ProcessingMode.ASYNCHRONOUS):
        self.agentspace = AgentSpace("demo_space")
        self.microkernel = CognitiveMicrokernel(self.agentspace, processing_mode)
        self.processing_mode = processing_mode
        
    def create_demo_agents(self):
        """Create demonstration agents with different capabilities and goals."""
        
        print("🤖 Creating Demo Agents...")
        
        # Research Lead Agent
        alice_state = self.microkernel.add_cognitive_agent(
            "Dr_Alice",
            goals=["coordinate_research_project", "build_team"],
            initial_beliefs={
                "role": "research_lead",
                "expertise": "project_management",
                "experience_level": "senior"
            }
        )
        
        # Machine Learning Expert
        bob_state = self.microkernel.add_cognitive_agent(
            "Dr_Bob", 
            goals=["develop_ml_models", "collaborate_on_research"],
            initial_beliefs={
                "role": "ml_expert", 
                "expertise": "machine_learning",
                "experience_level": "senior"
            }
        )
        
        # Data Scientist
        carol_state = self.microkernel.add_cognitive_agent(
            "Dr_Carol",
            goals=["analyze_data", "share_insights"],
            initial_beliefs={
                "role": "data_scientist",
                "expertise": "data_analysis", 
                "experience_level": "mid"
            }
        )
        
        print(f"✅ Created 3 agents in {self.processing_mode.value} mode")
        return [alice_state, bob_state, carol_state]
    
    def demonstrate_phase(self, phase_name: str, description: str):
        """Print demonstration header for a cognitive phase."""
        print(f"\n{'='*60}")
        print(f"🧠 PHASE: {phase_name}")
        print(f"📖 {description}")
        print(f"{'='*60}")
    
    async def demonstrate_cognitive_cycle(self, agent_id: str, cycles: int = 2):
        """Demonstrate multiple cognitive cycles for an agent with detailed logging."""
        
        print(f"\n🔄 Demonstrating Cognitive Cycle for {agent_id}")
        print(f"Processing Mode: {self.processing_mode.value}")
        
        for cycle in range(cycles):
            print(f"\n--- Cycle {cycle + 1} ---")
            
            # Get current state
            state = self.microkernel.get_agent_state(agent_id)
            if not state:
                print(f"❌ No state found for agent {agent_id}")
                continue
                
            print(f"Starting Phase: {state.current_phase.value}")
            print(f"Current Goals: {state.goals}")
            print(f"Activation Level: {state.activation_level:.2f}")
            
            # Process one complete cycle
            start_time = time.time()
            await self.microkernel.process_cognitive_cycle(agent_id)
            end_time = time.time()
            
            # Get updated state
            updated_state = self.microkernel.get_agent_state(agent_id)
            
            print(f"Completed Phase: {updated_state.current_phase.value}")
            print(f"Processing Time: {(end_time - start_time)*1000:.2f}ms")
            print(f"New Beliefs: {list(updated_state.beliefs.keys())[-3:]}")  # Show last 3 beliefs
            print(f"Attention Focus: {len(updated_state.attention_focus)} items")
            print(f"Active Intentions: {len(updated_state.intentions)}")
            
            await asyncio.sleep(0.5)  # Brief pause between cycles
    
    async def demonstrate_processing_modes(self):
        """Demonstrate different processing modes and their characteristics."""
        
        modes_to_test = [
            (ProcessingMode.SYNCHRONOUS, "Sequential, predictable processing"),
            (ProcessingMode.ASYNCHRONOUS, "Concurrent, responsive processing"),
        ]
        
        for mode, description in modes_to_test:
            print(f"\n{'='*80}")
            print(f"🔧 PROCESSING MODE: {mode.value}")
            print(f"📖 {description}")
            print(f"{'='*80}")
            
            # Create new microkernel with this mode
            demo_microkernel = CognitiveMicrokernel(self.agentspace, mode)
            
            # Add a test agent
            test_state = demo_microkernel.add_cognitive_agent(
                f"TestAgent_{mode.value}",
                goals=["test_processing_mode"],
                initial_beliefs={"mode": mode.value}
            )
            
            # Time the processing
            start_time = time.time()
            
            if mode == ProcessingMode.SYNCHRONOUS:
                # Process cycles sequentially
                for i in range(3):
                    await demo_microkernel.process_cognitive_cycle(f"TestAgent_{mode.value}")
                    print(f"  Completed cycle {i+1}")
                    
            elif mode == ProcessingMode.ASYNCHRONOUS:
                # Process cycles concurrently
                await demo_microkernel.start_all_agents(cycle_interval=0.2)
                await asyncio.sleep(1.0)  # Let it run for 1 second
                await demo_microkernel.stop_all_agents()
                print(f"  Completed asynchronous processing")
            
            end_time = time.time()
            print(f"  Total time: {(end_time - start_time)*1000:.2f}ms")
            
            # Show final state
            final_state = demo_microkernel.get_agent_state(f"TestAgent_{mode.value}")
            print(f"  Final phase: {final_state.current_phase.value}")
            print(f"  Beliefs gained: {len(final_state.beliefs)}")
    
    async def demonstrate_multi_agent_coordination(self):
        """Demonstrate how agents coordinate through the cognitive cycle."""
        
        print(f"\n{'='*80}")
        print(f"🤝 MULTI-AGENT COORDINATION DEMONSTRATION")
        print(f"{'='*80}")
        
        # Start all agents processing
        await self.microkernel.start_all_agents(cycle_interval=0.5)
        
        print("\n🚀 All agents are now processing autonomously...")
        print("Watch how they discover each other and form collaborations:")
        
        # Let them run for a while
        for i in range(6):
            await asyncio.sleep(1.0)
            
            # Show system state every second
            print(f"\n--- Time: {i+1}s ---")
            
            for agent_id in ["Dr_Alice", "Dr_Bob", "Dr_Carol"]:
                state = self.microkernel.get_agent_state(agent_id)
                if state:
                    collaborators = state.beliefs.get("potential_collaborators", [])
                    trust_info = state.beliefs.get("trust_levels", {})
                    print(f"  {agent_id}: Phase={state.current_phase.value}, "
                          f"Collaborators={len(collaborators)}, Trust relationships={len(trust_info)}")
        
        # Stop processing
        await self.microkernel.stop_all_agents()
        
        print("\n🛑 Stopped autonomous processing")
        
        # Show final coordination state
        print("\n📊 Final Coordination State:")
        agent_nodes = self.agentspace.get_agents()
        for agent in agent_nodes:
            collaborators = self.agentspace.get_collaborators(agent)
            trust_relationships = []
            for other in agent_nodes:
                if other != agent:
                    trust = self.agentspace.get_trust_level(agent, other)
                    if trust is not None:
                        trust_relationships.append(f"{other.name}:{trust:.2f}")
            
            print(f"  {agent.name}: {len(collaborators)} collaborations, "
                  f"Trust: {trust_relationships}")
    
    async def run_complete_demonstration(self):
        """Run the complete cognitive cycle demonstration."""
        
        print("🎯 SwarmCog Cognitive Cycle Demonstration")
        print("=" * 80)
        print("This demo shows:")
        print("1. Individual cognitive cycles for each agent")
        print("2. Different processing modes (Sync vs Async)")  
        print("3. Multi-agent coordination and emergence")
        print("=" * 80)
        
        # Create agents
        agent_states = self.create_demo_agents()
        
        # Demonstrate individual cognitive cycles
        print(f"\n🔍 PART 1: Individual Cognitive Cycles")
        await self.demonstrate_cognitive_cycle("Dr_Alice", cycles=2)
        
        # Demonstrate processing modes
        print(f"\n🔧 PART 2: Processing Modes")
        await self.demonstrate_processing_modes()
        
        # Demonstrate multi-agent coordination
        print(f"\n🤝 PART 3: Multi-Agent Coordination")
        await self.demonstrate_multi_agent_coordination()
        
        # Show final system state
        print(f"\n📈 FINAL SYSTEM STATE")
        system_state = self.microkernel.get_system_state()
        print(f"AgentSpace size: {len(self.agentspace)} atoms")
        print(f"Active agents: {system_state['microkernel']['active_agents']}")
        print(f"Total agent states: {len(system_state['agent_states'])}")
        
        print(f"\n✨ Demonstration complete! See CognitiveModelGuide.md for detailed explanations.")


async def main():
    """Main demonstration function."""
    
    # Run the demonstration in asynchronous mode
    demo = CognitiveCycleDemo(ProcessingMode.ASYNCHRONOUS)
    await demo.run_complete_demonstration()


if __name__ == "__main__":
    print("Starting SwarmCog Cognitive Cycle Demonstration...")
    asyncio.run(main())