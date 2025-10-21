#!/usr/bin/env python3
"""
SwarmCog Cognitive Research Team Example

This example demonstrates how to use SwarmCog to create an autonomous 
multi-agent research team that can:
- Collaborate on research tasks
- Share knowledge and insights
- Build trust relationships
- Coordinate complex multi-step projects
- Learn from interactions and adapt behavior

The example showcases the key features of SwarmCog:
- AgentSpace for knowledge representation
- Cognitive Microkernel for autonomous reasoning
- Cognitive Agents with autonomous capabilities
"""

import asyncio
import json
import time
from swarm import create_swarmcog, ProcessingMode


async def main():
    print("🧠 SwarmCog: Autonomous Multi-Agent Orchestration Framework")
    print("=" * 60)
    print("Creating a cognitive research team...\n")
    
    # Create SwarmCog system
    swarmcog = create_swarmcog(
        agentspace_name="research_lab",
        processing_mode=ProcessingMode.ASYNCHRONOUS,
        cognitive_cycle_interval=0.5,  # Fast cycles for demo
        max_agents=10
    )
    
    # Create research team members with different specializations
    print("👥 Creating research team members:")
    
    # Lead Researcher - coordinates and plans
    lead_researcher = swarmcog.create_cognitive_agent(
        name="Dr_Alice",
        capabilities=["project_management", "strategic_planning", "coordination"],
        goals=["coordinate_research_project", "ensure_team_collaboration", "deliver_results"],
        initial_beliefs={
            "role": "lead_researcher",
            "responsibility": "project_coordination",
            "team_trust": 0.8
        }
    )
    print(f"  ✓ {lead_researcher.name}: Lead Researcher (Project Management)")
    
    # Data Scientist - analyzes and processes data
    data_scientist = swarmcog.create_cognitive_agent(
        name="Dr_Bob", 
        capabilities=["data_analysis", "machine_learning", "statistics"],
        goals=["analyze_research_data", "build_predictive_models", "share_insights"],
        initial_beliefs={
            "role": "data_scientist",
            "specialty": "ML_and_statistics",
            "collaboration_preference": "data_driven"
        }
    )
    print(f"  ✓ {data_scientist.name}: Data Scientist (ML & Analytics)")
    
    # Domain Expert - provides domain knowledge
    domain_expert = swarmcog.create_cognitive_agent(
        name="Dr_Carol",
        capabilities=["domain_expertise", "literature_review", "hypothesis_generation"], 
        goals=["provide_domain_insights", "validate_hypotheses", "mentor_team"],
        initial_beliefs={
            "role": "domain_expert",
            "expertise_area": "cognitive_science",
            "years_experience": 15
        }
    )
    print(f"  ✓ {domain_expert.name}: Domain Expert (Cognitive Science)")
    
    # Research Assistant - supports and learns
    research_assistant = swarmcog.create_cognitive_agent(
        name="Alex",
        capabilities=["research_support", "data_collection", "learning"],
        goals=["support_senior_researchers", "learn_new_methods", "contribute_meaningfully"],
        initial_beliefs={
            "role": "research_assistant", 
            "learning_mode": "active",
            "eagerness": 0.9
        }
    )
    print(f"  ✓ {research_assistant.name}: Research Assistant (Learning & Support)")
    
    print(f"\n📊 Research team created with {len(swarmcog.list_agents())} members")
    
    # Establish initial trust relationships
    print("\n🤝 Establishing initial trust relationships:")
    lead_researcher.establish_trust("Dr_Bob", 0.85)
    lead_researcher.establish_trust("Dr_Carol", 0.90)
    lead_researcher.establish_trust("Alex", 0.75)
    
    data_scientist.establish_trust("Dr_Alice", 0.80)
    data_scientist.establish_trust("Dr_Carol", 0.85)
    data_scientist.establish_trust("Alex", 0.70)
    
    domain_expert.establish_trust("Dr_Alice", 0.88)
    domain_expert.establish_trust("Dr_Bob", 0.82)
    domain_expert.establish_trust("Alex", 0.75)
    
    print("  ✓ Trust network established")
    
    # Coordinate a multi-agent research project
    print("\n🎯 Coordinating research project:")
    project_result = swarmcog.coordinate_multi_agent_task(
        "Develop AI-Human Collaboration Framework", 
        ["Dr_Alice", "Dr_Bob", "Dr_Carol", "Alex"],
        "collaborative_research"
    )
    
    print(f"  ✓ Project '{project_result['description']}' initiated")
    print(f"  ✓ {len(project_result['collaboration_links'])} collaboration links created")
    
    # Share knowledge between team members
    print("\n📚 Sharing domain knowledge:")
    domain_expert.share_knowledge(
        "cognitive_frameworks",
        {
            "theory": "dual_process_theory",
            "applications": ["decision_making", "problem_solving"],
            "references": ["Kahneman_2011", "Evans_2008"]
        },
        "all_agents"
    )
    
    data_scientist.share_knowledge(
        "analysis_methods",
        {
            "techniques": ["neural_networks", "bayesian_inference", "clustering"],
            "tools": ["tensorflow", "pytorch", "scikit_learn"],
            "best_practices": ["cross_validation", "regularization"]
        }
    )
    print("  ✓ Knowledge sharing completed")
    
    # Show initial system state
    print("\n📋 Initial System State:")
    topology = swarmcog.get_swarm_topology()
    print(f"  • Agents: {topology['statistics']['total_agents']}")
    print(f"  • Collaborations: {topology['statistics']['active_collaborations']}")
    print(f"  • Average Trust: {topology['statistics']['average_trust_level']:.2f}")
    
    # Demonstrate autonomous cognitive processing
    print("\n🔄 Starting autonomous cognitive processing...")
    print("  (Agents will now think, plan, and interact autonomously)")
    
    # Start autonomous processing
    await swarmcog.start_autonomous_processing()
    
    # Let agents interact autonomously for a period
    simulation_time = 10  # seconds
    print(f"\n⏱️  Running autonomous simulation for {simulation_time} seconds...")
    
    # Monitor progress
    for i in range(simulation_time):
        await asyncio.sleep(1)
        if i % 3 == 0:  # Show progress every 3 seconds
            status = swarmcog.get_system_status()
            active_cycles = status['agents']['active_cognitive_cycles']
            print(f"    Time: {i+1}s | Active cognitive cycles: {active_cycles}")
    
    # Stop autonomous processing
    await swarmcog.stop_autonomous_processing()
    print("\n✋ Stopped autonomous processing")
    
    # Analyze results
    print("\n📈 Analysis of Autonomous Behavior:")
    
    final_topology = swarmcog.get_swarm_topology()
    interactions = swarmcog.get_agent_interactions()
    
    print(f"\nFinal System Statistics:")
    print(f"  • Total collaborations: {final_topology['statistics']['active_collaborations']}")
    print(f"  • Average trust level: {final_topology['statistics']['average_trust_level']:.2f}")
    print(f"  • Knowledge links: {len(final_topology['relationships']['knowledge_links'])}")
    
    print("\nAgent Interaction Summary:")
    for agent_name, agent_data in interactions.items():
        collaborators = len(agent_data['collaborators'])
        trust_relationships = len(agent_data['trust_levels'])
        active_goals = len(agent_data['active_goals'])
        
        print(f"  • {agent_name}:")
        print(f"    - Collaborators: {collaborators}")
        print(f"    - Trust relationships: {trust_relationships}")
        print(f"    - Active goals: {active_goals}")
        print(f"    - Capabilities: {', '.join(agent_data['capabilities'])}")
    
    # Show agent memories and learning
    print("\n🧠 Agent Learning and Memories:")
    for agent_name in swarmcog.list_agents():
        agent = swarmcog.get_agent(agent_name)
        if agent.memories:
            print(f"\n  {agent_name} memories:")
            for memory in agent.memories[-3:]:  # Show last 3 memories
                print(f"    • {memory['type']}: {memory.get('content', 'N/A')} "
                      f"(importance: {memory['importance']:.2f})")
    
    # Demonstrate agent reasoning capabilities
    print("\n🤔 Agent Cognitive Capabilities Demo:")
    
    print(f"\n{lead_researcher.name} reasoning about goals:")
    reasoning_result = lead_researcher.reason_about_goals()
    reasoning = json.loads(reasoning_result)
    for analysis in reasoning['goal_analysis'][:2]:  # Show first 2 analyses
        print(f"  • Goal: {analysis['goal']}")
        print(f"    Feasibility: {analysis['feasibility']:.2f}")
        print(f"    Required collaborators: {analysis['required_collaborators']}")
    
    print(f"\n{data_scientist.name} finding collaborators:")
    collab_result = data_scientist.find_collaborators("domain_expertise") 
    collaborators = json.loads(collab_result)
    if collaborators['potential_collaborators']:
        for collab in collaborators['potential_collaborators'][:2]:
            print(f"  • {collab['name']}: trust={collab['trust_level']:.2f}, "
                  f"capabilities={collab['capabilities']}")
    
    # Export final system state
    print(f"\n💾 Exporting system state...")
    timestamp = int(time.time())
    export_file = f"research_team_state_{timestamp}.json"
    swarmcog.export_system_state(export_file)
    print(f"  ✓ System state saved to {export_file}")
    
    # Summary
    print(f"\n🎉 SwarmCog Cognitive Research Team Demo Complete!")
    print("="*60)
    print("Key achievements:")
    print("  ✓ Created autonomous cognitive agents with specialized capabilities")
    print("  ✓ Established trust relationships and collaboration networks")
    print("  ✓ Demonstrated autonomous reasoning and planning")
    print("  ✓ Showed knowledge sharing and learning capabilities") 
    print("  ✓ Coordinated multi-agent research project")
    print("  ✓ Monitored emergent behaviors and adaptations")
    print("\nThe SwarmCog framework successfully orchestrated autonomous")
    print("multi-agent collaboration using cognitive AI principles! 🚀")


if __name__ == "__main__":
    asyncio.run(main())