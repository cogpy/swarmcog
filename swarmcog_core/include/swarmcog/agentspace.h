#pragma once

#include "types.h"
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace SwarmCog {

/**
 * Base Atom class - fundamental unit of knowledge representation
 * Inspired by OpenCog's AtomSpace but adapted for multi-agent systems
 */
class Atom {
protected:
    AtomId id_;
    AtomType type_;
    std::string name_;
    TruthValue truth_value_;
    AttentionValue attention_value_;
    Timestamp timestamp_;
    std::map<std::string, std::string> metadata_;
    mutable std::mutex mutex_;

public:
    Atom(AtomType type, const std::string& name = "");
    virtual ~Atom() = default;

    // Getters
    const AtomId& getId() const { return id_; }
    AtomType getType() const { return type_; }
    const std::string& getName() const { return name_; }
    TruthValue getTruthValue() const;
    AttentionValue getAttentionValue() const;
    Timestamp getTimestamp() const { return timestamp_; }
    
    // Setters
    void setName(const std::string& name);
    void setTruthValue(const TruthValue& tv);
    void setAttentionValue(const AttentionValue& av);
    void setMetadata(const std::string& key, const std::string& value);
    std::string getMetadata(const std::string& key) const;
    
    // Virtual methods
    virtual std::string toString() const;
    virtual std::map<std::string, std::string> toDict() const;
    
    // Operators
    bool operator==(const Atom& other) const { return id_ == other.id_; }
    bool operator!=(const Atom& other) const { return !(*this == other); }

private:
    static std::string generateId();
};

/**
 * Node class - represents entities, concepts, agents, etc.
 */
class Node : public Atom {
private:
    std::string value_;

public:
    Node(AtomType type, const std::string& name = "", const std::string& value = "");
    
    const std::string& getValue() const { return value_; }
    void setValue(const std::string& value) { value_ = value; }
    
    std::string toString() const override;
    std::map<std::string, std::string> toDict() const override;
};

/**
 * Link class - represents relationships between atoms
 */
class Link : public Atom {
private:
    std::vector<AtomPtr> outgoing_;

public:
    Link(AtomType type, const std::vector<AtomPtr>& outgoing = {}, const std::string& name = "");
    
    const std::vector<AtomPtr>& getOutgoing() const { return outgoing_; }
    void addOutgoing(const AtomPtr& atom);
    void removeOutgoing(const AtomPtr& atom);
    size_t getArity() const { return outgoing_.size(); }
    
    std::string toString() const override;
    std::map<std::string, std::string> toDict() const override;
};

/**
 * AgentSpace - Thread-safe knowledge representation system for multi-agent coordination
 * Central repository for all atoms, relationships, and knowledge
 */
class AgentSpace {
private:
    std::string name_;
    std::unordered_map<AtomId, AtomPtr> atoms_;
    std::unordered_map<AtomType, std::unordered_set<AtomId>> atoms_by_type_;
    std::unordered_map<std::string, std::unordered_set<AtomId>> atoms_by_name_;
    
    // Thread safety
    mutable std::shared_mutex atoms_mutex_;
    ThreadSafeCounter atom_counter_;
    
    // Attention mechanism
    std::vector<AtomId> attentional_focus_;
    mutable std::mutex focus_mutex_;

public:
    explicit AgentSpace(const std::string& name = "default_space");
    ~AgentSpace() = default;

    // Basic atom operations
    AtomPtr addAtom(AtomPtr atom);
    bool removeAtom(const AtomId& id);
    AtomPtr getAtom(const AtomId& id) const;
    std::vector<AtomPtr> getAtoms() const;
    std::vector<AtomPtr> getAtomsByType(AtomType type) const;
    std::vector<AtomPtr> getAtomsByName(const std::string& name) const;
    
    // Agent-specific operations
    NodePtr addAgentNode(const std::string& name, const std::vector<std::string>& capabilities = {});
    NodePtr addCapabilityNode(const std::string& name, const std::string& description = "");
    NodePtr addGoalNode(const std::string& goal, double priority = 0.5);
    NodePtr addBeliefNode(const std::string& belief, const std::string& value = "");
    NodePtr addMemoryNode(const std::string& content, const std::string& type = "episodic");
    
    // Relationship operations
    LinkPtr addCollaborationLink(const AgentId& agent1, const AgentId& agent2, const std::string& type = "general");
    LinkPtr addTrustRelationship(const AgentId& agent1, const AgentId& agent2, double trust_level = 0.5);
    LinkPtr addKnowledgeLink(const AtomId& source, const AtomId& target, const std::string& relation = "knows");
    
    // Query operations
    std::vector<AtomPtr> findAtoms(AtomType type, const std::string& name = "") const;
    std::vector<AtomId> getCollaborators(const AgentId& agent_id) const;
    double getTrustLevel(const AgentId& agent1, const AgentId& agent2) const;
    std::vector<AtomPtr> getMostImportantAtoms(size_t limit = 10) const;
    
    // Attention management
    void addToAttentionalFocus(const AtomId& atom_id);
    void removeFromAttentionalFocus(const AtomId& atom_id);
    std::vector<AtomId> getAttentionalFocus() const;
    void updateAttentionValues();
    
    // Utility methods
    size_t getAtomCount() const;
    std::string getName() const { return name_; }
    void clear();
    std::map<std::string, size_t> getStatistics() const;

private:
    std::string generateUniqueNodeName(const std::string& base) const;
    void addAtomToIndices(const AtomPtr& atom);
    void removeAtomFromIndices(const AtomPtr& atom);
};

// Factory functions for creating specific atom types
NodePtr createAgentNode(const std::string& name, const std::vector<std::string>& capabilities = {});
NodePtr createCapabilityNode(const std::string& name, const std::string& description = "");
LinkPtr createTrustLink(const AtomPtr& agent1, const AtomPtr& agent2, double trust_level = 0.5);
LinkPtr createCollaborationLink(const AtomPtr& agent1, const AtomPtr& agent2, const std::string& type = "general");

} // namespace SwarmCog