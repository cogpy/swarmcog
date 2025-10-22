#include "swarmcog/agentspace.h"
#include "swarmcog/utils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace SwarmCog {

// Atom Implementation
Atom::Atom(AtomType type, const std::string& name) 
    : type_(type), timestamp_(std::chrono::system_clock::now()) {
    id_ = generateId();
    name_ = name.empty() ? ("atom_" + id_.substr(0, 8)) : name;
}

std::string Atom::generateId() {
    return Utils::UUIDGenerator::generate();
}

TruthValue Atom::getTruthValue() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return truth_value_;
}

void Atom::setTruthValue(const TruthValue& tv) {
    std::lock_guard<std::mutex> lock(mutex_);
    truth_value_ = tv;
}

AttentionValue Atom::getAttentionValue() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return attention_value_;
}

void Atom::setAttentionValue(const AttentionValue& av) {
    std::lock_guard<std::mutex> lock(mutex_);
    attention_value_ = av;
}

void Atom::setName(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    name_ = name;
}

void Atom::setMetadata(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    metadata_[key] = value;
}

std::string Atom::getMetadata(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = metadata_.find(key);
    return (it != metadata_.end()) ? it->second : "";
}

std::string Atom::toString() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return "Atom(" + id_ + ", " + name_ + ", " + std::to_string(static_cast<int>(type_)) + ")";
}

std::map<std::string, std::string> Atom::toDict() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::map<std::string, std::string> result;
    result["id"] = id_;
    result["type"] = std::to_string(static_cast<int>(type_));
    result["name"] = name_;
    result["truth_strength"] = std::to_string(truth_value_.strength);
    result["truth_confidence"] = std::to_string(truth_value_.confidence);
    result["attention_sti"] = std::to_string(attention_value_.sti);
    result["attention_lti"] = std::to_string(attention_value_.lti);
    result["attention_vlti"] = std::to_string(attention_value_.vlti);
    result["timestamp"] = Utils::TimeUtils::timestampToString(timestamp_);
    
    // Add metadata
    for (const auto& pair : metadata_) {
        result["meta_" + pair.first] = pair.second;
    }
    
    return result;
}

// Node Implementation
Node::Node(AtomType type, const std::string& name, const std::string& value)
    : Atom(type, name), value_(value) {}

std::string Node::toString() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return "Node(" + id_ + ", " + name_ + ", " + value_ + ")";
}

std::map<std::string, std::string> Node::toDict() const {
    auto result = Atom::toDict();
    result["value"] = value_;
    return result;
}

// Link Implementation
Link::Link(AtomType type, const std::vector<AtomPtr>& outgoing, const std::string& name)
    : Atom(type, name), outgoing_(outgoing) {}

void Link::addOutgoing(const AtomPtr& atom) {
    if (atom) {
        outgoing_.push_back(atom);
    }
}

void Link::removeOutgoing(const AtomPtr& atom) {
    outgoing_.erase(
        std::remove(outgoing_.begin(), outgoing_.end(), atom),
        outgoing_.end()
    );
}

std::string Link::toString() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << "Link(" << id_ << ", " << name_ << ", [";
    for (size_t i = 0; i < outgoing_.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << (outgoing_[i] ? outgoing_[i]->getName() : "null");
    }
    oss << "])";
    return oss.str();
}

std::map<std::string, std::string> Link::toDict() const {
    auto result = Atom::toDict();
    result["arity"] = std::to_string(outgoing_.size());
    
    std::ostringstream oss;
    for (size_t i = 0; i < outgoing_.size(); ++i) {
        if (i > 0) oss << ",";
        oss << (outgoing_[i] ? outgoing_[i]->getId() : "null");
    }
    result["outgoing_ids"] = oss.str();
    
    return result;
}

// AgentSpace Implementation
AgentSpace::AgentSpace(const std::string& name) : name_(name) {
    Utils::Logger::info("Created AgentSpace: " + name_);
}

AtomPtr AgentSpace::addAtom(AtomPtr atom) {
    if (!atom) return nullptr;
    
    std::unique_lock<std::shared_mutex> lock(atoms_mutex_);
    
    const AtomId& id = atom->getId();
    atoms_[id] = atom;
    addAtomToIndices(atom);
    atom_counter_.increment();
    
    Utils::Logger::debug("Added atom: " + atom->toString());
    return atom;
}

bool AgentSpace::removeAtom(const AtomId& id) {
    std::unique_lock<std::shared_mutex> lock(atoms_mutex_);
    
    auto it = atoms_.find(id);
    if (it == atoms_.end()) {
        return false;
    }
    
    AtomPtr atom = it->second;
    removeAtomFromIndices(atom);
    atoms_.erase(it);
    
    Utils::Logger::debug("Removed atom: " + id);
    return true;
}

AtomPtr AgentSpace::getAtom(const AtomId& id) const {
    std::shared_lock<std::shared_mutex> lock(atoms_mutex_);
    
    auto it = atoms_.find(id);
    return (it != atoms_.end()) ? it->second : nullptr;
}

std::vector<AtomPtr> AgentSpace::getAtoms() const {
    std::shared_lock<std::shared_mutex> lock(atoms_mutex_);
    
    std::vector<AtomPtr> result;
    result.reserve(atoms_.size());
    
    for (const auto& pair : atoms_) {
        result.push_back(pair.second);
    }
    
    return result;
}

std::vector<AtomPtr> AgentSpace::getAtomsByType(AtomType type) const {
    std::shared_lock<std::shared_mutex> lock(atoms_mutex_);
    
    std::vector<AtomPtr> result;
    
    auto it = atoms_by_type_.find(type);
    if (it != atoms_by_type_.end()) {
        for (const AtomId& id : it->second) {
            auto atom_it = atoms_.find(id);
            if (atom_it != atoms_.end()) {
                result.push_back(atom_it->second);
            }
        }
    }
    
    return result;
}

std::vector<AtomPtr> AgentSpace::getAtomsByName(const std::string& name) const {
    std::shared_lock<std::shared_mutex> lock(atoms_mutex_);
    
    std::vector<AtomPtr> result;
    
    auto it = atoms_by_name_.find(name);
    if (it != atoms_by_name_.end()) {
        for (const AtomId& id : it->second) {
            auto atom_it = atoms_.find(id);
            if (atom_it != atoms_.end()) {
                result.push_back(atom_it->second);
            }
        }
    }
    
    return result;
}

NodePtr AgentSpace::addAgentNode(const std::string& name, const std::vector<std::string>& capabilities) {
    std::string agent_name = generateUniqueNodeName(name);
    
    auto agent_node = std::make_shared<Node>(AtomType::AGENT_NODE, agent_name);
    
    // Set agent metadata
    agent_node->setMetadata("type", "cognitive_agent");
    agent_node->setMetadata("creation_time", Utils::TimeUtils::timestampToString(Utils::TimeUtils::now()));
    
    // Add capabilities as metadata
    std::ostringstream cap_stream;
    for (size_t i = 0; i < capabilities.size(); ++i) {
        if (i > 0) cap_stream << ",";
        cap_stream << capabilities[i];
    }
    agent_node->setMetadata("capabilities", cap_stream.str());
    
    addAtom(agent_node);
    
    Utils::Logger::info("Created agent node: " + agent_name);
    return agent_node;
}

NodePtr AgentSpace::addCapabilityNode(const std::string& name, const std::string& description) {
    auto capability_node = std::make_shared<Node>(AtomType::CAPABILITY_NODE, name, description);
    addAtom(capability_node);
    return capability_node;
}

NodePtr AgentSpace::addGoalNode(const std::string& goal, double priority) {
    auto goal_node = std::make_shared<Node>(AtomType::GOAL_NODE, goal);
    goal_node->setTruthValue(TruthValue(priority, 0.8));
    addAtom(goal_node);
    return goal_node;
}

NodePtr AgentSpace::addBeliefNode(const std::string& belief, const std::string& value) {
    auto belief_node = std::make_shared<Node>(AtomType::BELIEF_NODE, belief, value);
    belief_node->setTruthValue(TruthValue(0.8, 0.7));
    addAtom(belief_node);
    return belief_node;
}

NodePtr AgentSpace::addMemoryNode(const std::string& content, const std::string& type) {
    auto memory_node = std::make_shared<Node>(AtomType::MEMORY_NODE, "memory_" + Utils::UUIDGenerator::generateShort(), content);
    memory_node->setMetadata("memory_type", type);
    memory_node->setAttentionValue(AttentionValue(0.5, 0.0, 0.3));
    addAtom(memory_node);
    return memory_node;
}

LinkPtr AgentSpace::addCollaborationLink(const AgentId& agent1, const AgentId& agent2, const std::string& type) {
    auto agent1_atom = getAtom(agent1);
    auto agent2_atom = getAtom(agent2);
    
    if (!agent1_atom || !agent2_atom) {
        Utils::Logger::warning("Cannot create collaboration link: agent not found");
        return nullptr;
    }
    
    auto collaboration_link = std::make_shared<Link>(
        AtomType::COLLABORATION_LINK,
        std::vector<AtomPtr>{agent1_atom, agent2_atom}
    );
    
    collaboration_link->setMetadata("collaboration_type", type);
    collaboration_link->setMetadata("created_time", Utils::TimeUtils::timestampToString(Utils::TimeUtils::now()));
    
    addAtom(collaboration_link);
    return collaboration_link;
}

LinkPtr AgentSpace::addTrustRelationship(const AgentId& agent1, const AgentId& agent2, double trust_level) {
    auto agent1_atom = getAtom(agent1);
    auto agent2_atom = getAtom(agent2);
    
    if (!agent1_atom || !agent2_atom) {
        Utils::Logger::warning("Cannot create trust relationship: agent not found");
        return nullptr;
    }
    
    auto trust_link = std::make_shared<Link>(
        AtomType::TRUST_LINK,
        std::vector<AtomPtr>{agent1_atom, agent2_atom}
    );
    
    trust_link->setTruthValue(TruthValue(trust_level, 0.5));
    trust_link->setMetadata("trust_level", std::to_string(trust_level));
    
    addAtom(trust_link);
    return trust_link;
}

LinkPtr AgentSpace::addKnowledgeLink(const AtomId& source, const AtomId& target, const std::string& relation) {
    auto source_atom = getAtom(source);
    auto target_atom = getAtom(target);
    
    if (!source_atom || !target_atom) {
        Utils::Logger::warning("Cannot create knowledge link: atom not found");
        return nullptr;
    }
    
    auto knowledge_link = std::make_shared<Link>(
        AtomType::KNOWLEDGE_LINK,
        std::vector<AtomPtr>{source_atom, target_atom}
    );
    
    knowledge_link->setMetadata("relation", relation);
    
    addAtom(knowledge_link);
    return knowledge_link;
}

std::vector<AtomPtr> AgentSpace::findAtoms(AtomType type, const std::string& name) const {
    if (name.empty()) {
        return getAtomsByType(type);
    } else {
        auto atoms_by_name = getAtomsByName(name);
        std::vector<AtomPtr> result;
        
        for (const auto& atom : atoms_by_name) {
            if (atom->getType() == type) {
                result.push_back(atom);
            }
        }
        
        return result;
    }
}

std::vector<AtomId> AgentSpace::getCollaborators(const AgentId& agent_id) const {
    std::shared_lock<std::shared_mutex> lock(atoms_mutex_);
    
    std::vector<AtomId> collaborators;
    auto agent_atom = getAtom(agent_id);
    if (!agent_atom) return collaborators;
    
    // Find collaboration links involving this agent
    auto collaboration_links = getAtomsByType(AtomType::COLLABORATION_LINK);
    
    for (const auto& link_atom : collaboration_links) {
        auto link = std::dynamic_pointer_cast<Link>(link_atom);
        if (!link || link->getArity() != 2) continue;
        
        const auto& outgoing = link->getOutgoing();
        if (outgoing[0]->getId() == agent_id) {
            collaborators.push_back(outgoing[1]->getId());
        } else if (outgoing[1]->getId() == agent_id) {
            collaborators.push_back(outgoing[0]->getId());
        }
    }
    
    return collaborators;
}

double AgentSpace::getTrustLevel(const AgentId& agent1, const AgentId& agent2) const {
    std::shared_lock<std::shared_mutex> lock(atoms_mutex_);
    
    auto trust_links = getAtomsByType(AtomType::TRUST_LINK);
    
    for (const auto& link_atom : trust_links) {
        auto link = std::dynamic_pointer_cast<Link>(link_atom);
        if (!link || link->getArity() != 2) continue;
        
        const auto& outgoing = link->getOutgoing();
        if ((outgoing[0]->getId() == agent1 && outgoing[1]->getId() == agent2) ||
            (outgoing[0]->getId() == agent2 && outgoing[1]->getId() == agent1)) {
            return link->getTruthValue().strength;
        }
    }
    
    return 0.0; // No trust relationship found
}

std::vector<AtomPtr> AgentSpace::getMostImportantAtoms(size_t limit) const {
    auto atoms = getAtoms();
    
    // Sort by combined attention values
    std::sort(atoms.begin(), atoms.end(), [](const AtomPtr& a, const AtomPtr& b) {
        auto av_a = a->getAttentionValue();
        auto av_b = b->getAttentionValue();
        double importance_a = av_a.sti + av_a.lti + av_a.vlti;
        double importance_b = av_b.sti + av_b.lti + av_b.vlti;
        return importance_a > importance_b;
    });
    
    if (atoms.size() > limit) {
        atoms.resize(limit);
    }
    
    return atoms;
}

void AgentSpace::addToAttentionalFocus(const AtomId& atom_id) {
    std::lock_guard<std::mutex> lock(focus_mutex_);
    
    // Remove if already exists to avoid duplicates
    attentional_focus_.erase(
        std::remove(attentional_focus_.begin(), attentional_focus_.end(), atom_id),
        attentional_focus_.end()
    );
    
    attentional_focus_.push_back(atom_id);
    
    // Limit focus size
    const size_t max_focus_size = 20;
    if (attentional_focus_.size() > max_focus_size) {
        attentional_focus_.erase(attentional_focus_.begin());
    }
}

void AgentSpace::removeFromAttentionalFocus(const AtomId& atom_id) {
    std::lock_guard<std::mutex> lock(focus_mutex_);
    
    attentional_focus_.erase(
        std::remove(attentional_focus_.begin(), attentional_focus_.end(), atom_id),
        attentional_focus_.end()
    );
}

std::vector<AtomId> AgentSpace::getAttentionalFocus() const {
    std::lock_guard<std::mutex> lock(focus_mutex_);
    return attentional_focus_;
}

void AgentSpace::updateAttentionValues() {
    auto atoms = getAtoms();
    
    for (auto& atom : atoms) {
        auto av = atom->getAttentionValue();
        
        // Decay attention values
        av.sti *= 0.99;
        av.lti = av.lti * 0.999 + av.sti * 0.001;
        av.vlti = av.vlti * 0.9999 + av.lti * 0.0001;
        
        atom->setAttentionValue(av);
    }
}

size_t AgentSpace::getAtomCount() const {
    std::shared_lock<std::shared_mutex> lock(atoms_mutex_);
    return atoms_.size();
}

void AgentSpace::clear() {
    std::unique_lock<std::shared_mutex> atoms_lock(atoms_mutex_);
    std::lock_guard<std::mutex> focus_lock(focus_mutex_);
    
    atoms_.clear();
    atoms_by_type_.clear();
    atoms_by_name_.clear();
    attentional_focus_.clear();
    atom_counter_.reset();
    
    Utils::Logger::info("Cleared AgentSpace: " + name_);
}

std::map<std::string, size_t> AgentSpace::getStatistics() const {
    std::shared_lock<std::shared_mutex> lock(atoms_mutex_);
    
    std::map<std::string, size_t> stats;
    stats["total_atoms"] = atoms_.size();
    stats["attentional_focus_size"] = attentional_focus_.size();
    
    for (const auto& pair : atoms_by_type_) {
        std::string type_name = "type_" + std::to_string(static_cast<int>(pair.first));
        stats[type_name] = pair.second.size();
    }
    
    return stats;
}

std::string AgentSpace::generateUniqueNodeName(const std::string& base) const {
    std::string candidate = base;
    int counter = 1;
    
    while (!getAtomsByName(candidate).empty()) {
        candidate = base + "_" + std::to_string(counter++);
    }
    
    return candidate;
}

void AgentSpace::addAtomToIndices(const AtomPtr& atom) {
    atoms_by_type_[atom->getType()].insert(atom->getId());
    atoms_by_name_[atom->getName()].insert(atom->getId());
}

void AgentSpace::removeAtomFromIndices(const AtomPtr& atom) {
    atoms_by_type_[atom->getType()].erase(atom->getId());
    atoms_by_name_[atom->getName()].erase(atom->getId());
}

// Factory functions
NodePtr createAgentNode(const std::string& name, const std::vector<std::string>& capabilities) {
    auto node = std::make_shared<Node>(AtomType::AGENT_NODE, name);
    
    std::ostringstream cap_stream;
    for (size_t i = 0; i < capabilities.size(); ++i) {
        if (i > 0) cap_stream << ",";
        cap_stream << capabilities[i];
    }
    node->setMetadata("capabilities", cap_stream.str());
    
    return node;
}

NodePtr createCapabilityNode(const std::string& name, const std::string& description) {
    return std::make_shared<Node>(AtomType::CAPABILITY_NODE, name, description);
}

LinkPtr createTrustLink(const AtomPtr& agent1, const AtomPtr& agent2, double trust_level) {
    auto link = std::make_shared<Link>(AtomType::TRUST_LINK, std::vector<AtomPtr>{agent1, agent2});
    link->setTruthValue(TruthValue(trust_level, 0.5));
    return link;
}

LinkPtr createCollaborationLink(const AtomPtr& agent1, const AtomPtr& agent2, const std::string& type) {
    auto link = std::make_shared<Link>(AtomType::COLLABORATION_LINK, std::vector<AtomPtr>{agent1, agent2});
    link->setMetadata("collaboration_type", type);
    return link;
}

} // namespace SwarmCog