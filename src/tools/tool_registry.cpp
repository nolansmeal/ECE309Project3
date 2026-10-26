// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.

#include "tools/tool_registry.h"

// Constructor: Initialize the hash table with 16 empty buckets
ToolRegistry::ToolRegistry() : buckets_(16) {
}

// Destructor: unique_ptr automatically deletes the stored tools
ToolRegistry::~ToolRegistry() = default;

// FNV-1a hash function: Generates a hash value from a string
std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::size_t hash_value = 14695981039346656037ull;

    // Process every character in the tool name
    for (unsigned char ch : key) {
        hash_value ^= ch;
        hash_value *= 1099511628211;
    }

    return hash_value;
}

// Adds a new tool to the hash table
bool ToolRegistry::add(std::unique_ptr<Tool> tool) {

    // Reject null pointers
    if (tool == nullptr) {
        return false;
    }

    // Reject tools with duplicate names
    if (find(tool->name()) != nullptr) {
        return false;
    }

    // Resize the table if adding this tool exceeds a 75% load factor
    if ((size_ + 1) * 4 > buckets_.size() * 3) {
        rehash(buckets_.size() * 2);
    }

    // Calculate the bucket index
    std::size_t index = hash(tool->name()) % buckets_.size();

    // Move ownership of the tool into its bucket
    buckets_[index].push_back(std::move(tool));

    // Update the number of registered tools
    size_++;

    return true;
}

// Searches for a tool by its name
Tool* ToolRegistry::find(std::string_view name) const {

    // Calculate which bucket the tool belongs to
    std::size_t index = hash(name) % buckets_.size();

    // Search through every tool in that bucket
    for (const auto& tool : buckets_[index]) {

        // Return the tool if its name matches
        if (tool->name() == name) {
            return tool.get();
        }
    }

    // Tool was not found
    return nullptr;
}

// Returns the number of tools stored
std::size_t ToolRegistry::size() const noexcept {
    return size_;
}

// Calls the provided function once for every registered tool
void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {

    // Loop through every bucket
    for (const auto& bucket : buckets_) {

        // Loop through every tool in the bucket
        for (const auto& tool : bucket) {
            fn(*tool);
        }
    }
}

// Returns the number of buckets in the hash table
std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();
}

// Calculates the average number of tools per bucket
double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(size_) / buckets_.size();
}

// Returns the length of the longest bucket chain
std::size_t ToolRegistry::max_probe_length() const {

    std::size_t max_length = 0;

    // Examine every bucket
    for (const auto& bucket : buckets_) {

        // Update the maximum if this bucket contains more tools
        if (bucket.size() > max_length) {
            max_length = bucket.size();
        }
    }

    return max_length;
}

// Resizes the hash table and redistributes all existing tools
void ToolRegistry::rehash(std::size_t new_bucket_count) {

    // Create a new hash table with the requested number of buckets
    std::vector<std::vector<std::unique_ptr<Tool>>> new_buckets(new_bucket_count);

    // Go through every bucket in the original hash table
    for (auto& bucket : buckets_) {

        // Go through every tool in the current bucket
        for (auto& tool : bucket) {

            // Calculate the tool's new bucket index
            std::size_t index = hash(tool->name()) % new_bucket_count;

            // Transfer ownership of the tool into its new bucket
            new_buckets[index].push_back(std::move(tool));
        }
    }

    // Replace the old bucket array with the new one
    buckets_ = std::move(new_buckets);
}
