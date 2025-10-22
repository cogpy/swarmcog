![Swarm Logo](assets/logo.png)

# SwarmCog: Autonomous Multi-Agent Orchestration Framework

**SwarmCog** is an OpenCog-inspired autonomous multi-agent orchestration framework that extends the original Swarm framework with advanced cognitive capabilities. It enables agents to reason autonomously, build trust relationships, share knowledge, and coordinate complex multi-agent tasks using cognitive AI principles.

## 🧠 Key Features

- **🤖 Cognitive Agents**: Autonomous reasoning, learning, and coordination
- **🌐 AgentSpace**: OpenCog-inspired knowledge representation system  
- **⚡ Cognitive Microkernel**: Multi-phase cognitive processing pipeline
- **🤝 Trust Networks**: Dynamic trust relationship building
- **📚 Knowledge Sharing**: Collaborative learning and information exchange
- **🎯 Goal-Oriented**: Autonomous goal decomposition and planning
- **🔄 Adaptive Behavior**: Continuous learning from experiences
- **📊 System Monitoring**: Real-time topology and performance analysis

## 🚀 Quick Start

### Basic SwarmCog Usage

```python
from swarm import create_swarmcog, ProcessingMode

# Create SwarmCog system
swarmcog = create_swarmcog(
    agentspace_name="research_lab",
    processing_mode=ProcessingMode.ASYNCHRONOUS
)

# Create cognitive agents
researcher = swarmcog.create_cognitive_agent(
    name="Dr_Alice",
    capabilities=["research", "analysis", "collaboration"],
    goals=["solve_complex_problems", "mentor_team"],
    initial_beliefs={"domain": "AI_research", "experience": "expert"}
)

analyst = swarmcog.create_cognitive_agent(
    name="Dr_Bob", 
    capabilities=["data_analysis", "machine_learning"],
    goals=["analyze_data", "build_models"]
)

# Establish trust and coordinate
researcher.establish_trust("Dr_Bob", 0.8)
researcher.coordinate_with_agent("Dr_Bob", "joint_research")

# Run autonomous multi-agent coordination
await swarmcog.start_autonomous_processing()
```

### Advanced Cognitive Features

```python
# Agent reasoning and planning
reasoning = researcher.reason_about_goals()
collaboration_plan = researcher.plan_collaboration("research_project")

# Knowledge sharing
researcher.share_knowledge(
    "research_methods", 
    {"techniques": ["analysis", "modeling"], "tools": ["python", "tensorflow"]},
    target_agent="Dr_Bob"
)

# Find collaborators based on capabilities and trust
collaborators = researcher.find_collaborators("machine_learning")

# System analysis
topology = swarmcog.get_swarm_topology()
interactions = swarmcog.get_agent_interactions()
```

## 🏗️ Architecture

SwarmCog implements a three-layer cognitive architecture:

### 1. AgentSpace (Agentic AtomSpace)
- **Knowledge Representation**: Unified graph-based knowledge system
- **Atoms & Links**: Agents, capabilities, goals, beliefs, relationships
- **Truth Values**: Uncertainty handling with strength and confidence  
- **Attention Values**: Cognitive resource allocation

### 2. Cognitive Microkernel  
- **7-Phase Cognitive Cycle**: Perception → Attention → Reasoning → Planning → Execution → Learning → Reflection
- **Processing Modes**: Synchronous, Asynchronous, Distributed
- **Autonomous Coordination**: Self-organizing multi-agent behavior

### 3. Cognitive Agents
- **Autonomous Reasoning**: Independent decision making and planning
- **Trust Management**: Dynamic relationship building
- **Memory Systems**: Experience-based learning and adaptation
- **Social Cognition**: Understanding and modeling other agents

## 📋 Original Swarm Compatibility

SwarmCog maintains full backward compatibility with the original Swarm framework:

## Install

Requires Python 3.10+

```shell
pip install git+ssh://git@github.com/openai/swarm.git
```

or

```shell
pip install git+https://github.com/openai/swarm.git
```

## Usage

```python
from swarm import Swarm, Agent

client = Swarm()

def transfer_to_agent_b():
    return agent_b


agent_a = Agent(
    name="Agent A",
    instructions="You are a helpful agent.",
    functions=[transfer_to_agent_b],
)

agent_b = Agent(
    name="Agent B",
    instructions="Only speak in Haikus.",
)

response = client.run(
    agent=agent_a,
    messages=[{"role": "user", "content": "I want to talk to agent B."}],
)

print(response.messages[-1]["content"])
```

```
Hope glimmers brightly,
New paths converge gracefully,
What can I assist?
```

## Table of Contents

- [Overview](#overview)
- [Examples](#examples)
- [Documentation](#documentation)
  - [Running Swarm](#running-swarm)
  - [Agents](#agents)
  - [Functions](#functions)
  - [Streaming](#streaming)
- [Evaluations](#evaluations)
- [Utils](#utils)

# Overview

Swarm focuses on making agent **coordination** and **execution** lightweight, highly controllable, and easily testable.

It accomplishes this through two primitive abstractions: `Agent`s and **handoffs**. An `Agent` encompasses `instructions` and `tools`, and can at any point choose to hand off a conversation to another `Agent`.

These primitives are powerful enough to express rich dynamics between tools and networks of agents, allowing you to build scalable, real-world solutions while avoiding a steep learning curve.

> [!NOTE]
> Swarm Agents are not related to Assistants in the Assistants API. They are named similarly for convenience, but are otherwise completely unrelated. Swarm is entirely powered by the Chat Completions API and is hence stateless between calls.

## Why Swarm

Swarm explores patterns that are lightweight, scalable, and highly customizable by design. Approaches similar to Swarm are best suited for situations dealing with a large number of independent capabilities and instructions that are difficult to encode into a single prompt.

The Assistants API is a great option for developers looking for fully-hosted threads and built in memory management and retrieval. However, Swarm is an educational resource for developers curious to learn about multi-agent orchestration. Swarm runs (almost) entirely on the client and, much like the Chat Completions API, does not store state between calls.

# Examples

## 🧠 SwarmCog Examples

- [`cognitive_research_team`](examples/cognitive_research_team.py): **Autonomous Multi-Agent Research Team** - Demonstrates cognitive agents collaborating on research projects with trust building, knowledge sharing, and autonomous coordination
- [`cognitive_cycle_demo`](examples/cognitive_cycle_demo.py): **Cognitive Cycle Demonstration** - Interactive demonstration of the 7-phase cognitive cycle and 3 processing modes with detailed logging

## 📚 Original Swarm Examples

Check out `/examples` for inspiration! Learn more about each one in its README.

- [`basic`](examples/basic): Simple examples of fundamentals like setup, function calling, handoffs, and context variables
- [`triage_agent`](examples/triage_agent): Simple example of setting up a basic triage step to hand off to the right agent
- [`weather_agent`](examples/weather_agent): Simple example of function calling
- [`airline`](examples/airline): A multi-agent setup for handling different customer service requests in an airline context.
- [`support_bot`](examples/support_bot): A customer service bot which includes a user interface agent and a help center agent with several tools
- [`personal_shopper`](examples/personal_shopper): A personal shopping agent that can help with making sales and refunding orders

# Documentation

![Swarm Diagram](assets/swarm_diagram.png)

## Running Swarm

Start by instantiating a Swarm client (which internally just instantiates an `OpenAI` client).

```python
from swarm import Swarm

client = Swarm()
```

### `client.run()`

Swarm's `run()` function is analogous to the `chat.completions.create()` function in the Chat Completions API – it takes `messages` and returns `messages` and saves no state between calls. Importantly, however, it also handles Agent function execution, hand-offs, context variable references, and can take multiple turns before returning to the user.

At its core, Swarm's `client.run()` implements the following loop:

1. Get a completion from the current Agent
2. Execute tool calls and append results
3. Switch Agent if necessary
4. Update context variables, if necessary
5. If no new function calls, return

#### Arguments

| Argument              | Type    | Description                                                                                                                                            | Default        |
| --------------------- | ------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------- |
| **agent**             | `Agent` | The (initial) agent to be called.                                                                                                                      | (required)     |
| **messages**          | `List`  | A list of message objects, identical to [Chat Completions `messages`](https://platform.openai.com/docs/api-reference/chat/create#chat-create-messages) | (required)     |
| **context_variables** | `dict`  | A dictionary of additional context variables, available to functions and Agent instructions                                                            | `{}`           |
| **max_turns**         | `int`   | The maximum number of conversational turns allowed                                                                                                     | `float("inf")` |
| **model_override**    | `str`   | An optional string to override the model being used by an Agent                                                                                        | `None`         |
| **execute_tools**     | `bool`  | If `False`, interrupt execution and immediately returns `tool_calls` message when an Agent tries to call a function                                    | `True`         |
| **stream**            | `bool`  | If `True`, enables streaming responses                                                                                                                 | `False`        |
| **debug**             | `bool`  | If `True`, enables debug logging                                                                                                                       | `False`        |

Once `client.run()` is finished (after potentially multiple calls to agents and tools) it will return a `Response` containing all the relevant updated state. Specifically, the new `messages`, the last `Agent` to be called, and the most up-to-date `context_variables`. You can pass these values (plus new user messages) in to your next execution of `client.run()` to continue the interaction where it left off – much like `chat.completions.create()`. (The `run_demo_loop` function implements an example of a full execution loop in `/swarm/repl/repl.py`.)

#### `Response` Fields

| Field                 | Type    | Description                                                                                                                                                                                                                                                                  |
| --------------------- | ------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **messages**          | `List`  | A list of message objects generated during the conversation. Very similar to [Chat Completions `messages`](https://platform.openai.com/docs/api-reference/chat/create#chat-create-messages), but with a `sender` field indicating which `Agent` the message originated from. |
| **agent**             | `Agent` | The last agent to handle a message.                                                                                                                                                                                                                                          |
| **context_variables** | `dict`  | The same as the input variables, plus any changes.                                                                                                                                                                                                                           |

## Agents

An `Agent` simply encapsulates a set of `instructions` with a set of `functions` (plus some additional settings below), and has the capability to hand off execution to another `Agent`.

While it's tempting to personify an `Agent` as "someone who does X", it can also be used to represent a very specific workflow or step defined by a set of `instructions` and `functions` (e.g. a set of steps, a complex retrieval, single step of data transformation, etc). This allows `Agent`s to be composed into a network of "agents", "workflows", and "tasks", all represented by the same primitive.

## `Agent` Fields

| Field            | Type                     | Description                                                                   | Default                      |
| ---------------- | ------------------------ | ----------------------------------------------------------------------------- | ---------------------------- |
| **name**         | `str`                    | The name of the agent.                                                        | `"Agent"`                    |
| **model**        | `str`                    | The model to be used by the agent.                                            | `"gpt-4o"`                   |
| **instructions** | `str` or `func() -> str` | Instructions for the agent, can be a string or a callable returning a string. | `"You are a helpful agent."` |
| **functions**    | `List`                   | A list of functions that the agent can call.                                  | `[]`                         |
| **tool_choice**  | `str`                    | The tool choice for the agent, if any.                                        | `None`                       |

### Instructions

`Agent` `instructions` are directly converted into the `system` prompt of a conversation (as the first message). Only the `instructions` of the active `Agent` will be present at any given time (e.g. if there is an `Agent` handoff, the `system` prompt will change, but the chat history will not.)

```python
agent = Agent(
   instructions="You are a helpful agent."
)
```

The `instructions` can either be a regular `str`, or a function that returns a `str`. The function can optionally receive a `context_variables` parameter, which will be populated by the `context_variables` passed into `client.run()`.

```python
def instructions(context_variables):
   user_name = context_variables["user_name"]
   return f"Help the user, {user_name}, do whatever they want."

agent = Agent(
   instructions=instructions
)
response = client.run(
   agent=agent,
   messages=[{"role":"user", "content": "Hi!"}],
   context_variables={"user_name":"John"}
)
print(response.messages[-1]["content"])
```

```
Hi John, how can I assist you today?
```

## Functions

- Swarm `Agent`s can call python functions directly.
- Function should usually return a `str` (values will be attempted to be cast as a `str`).
- If a function returns an `Agent`, execution will be transferred to that `Agent`.
- If a function defines a `context_variables` parameter, it will be populated by the `context_variables` passed into `client.run()`.

```python
def greet(context_variables, language):
   user_name = context_variables["user_name"]
   greeting = "Hola" if language.lower() == "spanish" else "Hello"
   print(f"{greeting}, {user_name}!")
   return "Done"

agent = Agent(
   functions=[greet]
)

client.run(
   agent=agent,
   messages=[{"role": "user", "content": "Usa greet() por favor."}],
   context_variables={"user_name": "John"}
)
```

```
Hola, John!
```

- If an `Agent` function call has an error (missing function, wrong argument, error) an error response will be appended to the chat so the `Agent` can recover gracefully.
- If multiple functions are called by the `Agent`, they will be executed in that order.

### Handoffs and Updating Context Variables

An `Agent` can hand off to another `Agent` by returning it in a `function`.

```python
sales_agent = Agent(name="Sales Agent")

def transfer_to_sales():
   return sales_agent

agent = Agent(functions=[transfer_to_sales])

response = client.run(agent, [{"role":"user", "content":"Transfer me to sales."}])
print(response.agent.name)
```

```
Sales Agent
```

It can also update the `context_variables` by returning a more complete `Result` object. This can also contain a `value` and an `agent`, in case you want a single function to return a value, update the agent, and update the context variables (or any subset of the three).

```python
sales_agent = Agent(name="Sales Agent")

def talk_to_sales():
   print("Hello, World!")
   return Result(
       value="Done",
       agent=sales_agent,
       context_variables={"department": "sales"}
   )

agent = Agent(functions=[talk_to_sales])

response = client.run(
   agent=agent,
   messages=[{"role": "user", "content": "Transfer me to sales"}],
   context_variables={"user_name": "John"}
)
print(response.agent.name)
print(response.context_variables)
```

```
Sales Agent
{'department': 'sales', 'user_name': 'John'}
```

> [!NOTE]
> If an `Agent` calls multiple functions to hand-off to an `Agent`, only the last handoff function will be used.

### Function Schemas

Swarm automatically converts functions into a JSON Schema that is passed into Chat Completions `tools`.

- Docstrings are turned into the function `description`.
- Parameters without default values are set to `required`.
- Type hints are mapped to the parameter's `type` (and default to `string`).
- Per-parameter descriptions are not explicitly supported, but should work similarly if just added in the docstring. (In the future docstring argument parsing may be added.)

```python
def greet(name, age: int, location: str = "New York"):
   """Greets the user. Make sure to get their name and age before calling.

   Args:
      name: Name of the user.
      age: Age of the user.
      location: Best place on earth.
   """
   print(f"Hello {name}, glad you are {age} in {location}!")
```

```javascript
{
   "type": "function",
   "function": {
      "name": "greet",
      "description": "Greets the user. Make sure to get their name and age before calling.\n\nArgs:\n   name: Name of the user.\n   age: Age of the user.\n   location: Best place on earth.",
      "parameters": {
         "type": "object",
         "properties": {
            "name": {"type": "string"},
            "age": {"type": "integer"},
            "location": {"type": "string"}
         },
         "required": ["name", "age"]
      }
   }
}
```

## Streaming

```python
stream = client.run(agent, messages, stream=True)
for chunk in stream:
   print(chunk)
```

Uses the same events as [Chat Completions API streaming](https://platform.openai.com/docs/api-reference/streaming). See `process_and_print_streaming_response` in `/swarm/repl/repl.py` as an example.

Two new event types have been added:

- `{"delim":"start"}` and `{"delim":"end"}`, to signal each time an `Agent` handles a single message (response or function call). This helps identify switches between `Agent`s.
- `{"response": Response}` will return a `Response` object at the end of a stream with the aggregated (complete) response, for convenience.

# Evaluations

Evaluations are crucial to any project, and we encourage developers to bring their own eval suites to test the performance of their swarms. For reference, we have some examples for how to eval swarm in the `airline`, `weather_agent` and `triage_agent` quickstart examples. See the READMEs for more details.

# Utils

Use the `run_demo_loop` to test out your swarm! This will run a REPL on your command line. Supports streaming.

```python
from swarm.repl import run_demo_loop
...
run_demo_loop(agent, stream=True)
```

# SwarmCog API Reference

## Core Classes

### SwarmCog
Main orchestration system for cognitive multi-agent coordination.

```python
from swarm import SwarmCog, SwarmCogConfig, ProcessingMode

# Configuration
config = SwarmCogConfig(
    processing_mode=ProcessingMode.ASYNCHRONOUS,
    agentspace_name="my_swarm",
    max_agents=20,
    cognitive_cycle_interval=1.0
)

swarmcog = SwarmCog(config)

# Agent Management
agent = swarmcog.create_cognitive_agent(name, capabilities, goals, beliefs)
swarmcog.remove_agent(name)
agents = swarmcog.list_agents()

# Task Coordination  
task = swarmcog.coordinate_multi_agent_task(description, agents, strategy)

# System Monitoring
topology = swarmcog.get_swarm_topology()
status = swarmcog.get_system_status()
interactions = swarmcog.get_agent_interactions()

# Autonomous Operation
await swarmcog.start_autonomous_processing()
results = await swarmcog.simulate_autonomous_behavior(duration)
await swarmcog.stop_autonomous_processing()
```

### CognitiveAgent
Enhanced agent with autonomous cognitive capabilities.

```python
from swarm import CognitiveAgent

agent = CognitiveAgent(name, capabilities, goals, initial_beliefs)

# Capability Management
agent.add_capability(name, description, strength)
agent.add_goal(goal, priority)
agent.update_belief(name, value)
agent.add_memory(type, content, importance)

# Cognitive Functions
agent.perceive_environment()
agent.reason_about_goals()
agent.plan_collaboration(goal)
agent.find_collaborators(capability_needed)
agent.establish_trust(agent_name, trust_level)
agent.share_knowledge(type, content, target)
agent.coordinate_with_agent(agent_name, type)

# State Management
state = agent.get_cognitive_state()
collaborators = agent.get_collaborators()
trust_levels = agent.get_trust_levels()
```

### AgentSpace
Knowledge representation system for multi-agent coordination.

```python  
from swarm import AgentSpace, AtomType

agentspace = AgentSpace("space_name")

# Agent Management
agent_node = agentspace.add_agent_node(name, capabilities)
agents = agentspace.get_agents()

# Relationship Management
collab_link = agentspace.add_collaboration_link(agent1, agent2, type)
trust_link = agentspace.add_trust_relationship(agent1, agent2, level)

# Query Operations
atoms = agentspace.find_atoms(atom_type, name)
collaborators = agentspace.get_collaborators(agent)
trust_level = agentspace.get_trust_level(agent1, agent2)
important_atoms = agentspace.get_most_important_atoms(limit)
```

## 📖 Documentation

- [SwarmCog Architecture Guide](docs/SwarmCog_Architecture.md)
- [Cognitive Model Guide](docs/CognitiveModelGuide.md) - Comprehensive explanation of the 7-phase cognitive cycle and 3 processing modes
- [Cognitive Research Team Example](examples/cognitive_research_team.py)
- API Documentation: See source code docstrings

## 🤝 Contributing

SwarmCog builds upon the original Swarm framework. Contributions welcome for:
- Advanced cognitive processing algorithms
- New cognitive agent types and capabilities  
- Performance optimizations and scalability improvements
- Additional examples and use cases

# Original Swarm Contributors

- Ilan Bigio - [ibigio](https://github.com/ibigio)
- James Hills - [jhills20](https://github.com/jhills20)
- Shyamal Anadkat - [shyamal-anadkat](https://github.com/shyamal-anadkat)
- Charu Jaiswal - [charuj](https://github.com/charuj)
- Colin Jarvis - [colin-openai](https://github.com/colin-openai)
- Katia Gil Guzman - [katia-openai](https://github.com/katia-openai)

# SwarmCog Contributors

- SwarmCog Development Team
