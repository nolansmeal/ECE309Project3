#pragma once

#include "message.h"
#include <cstddef>

using namespace std;

class Conversation {
public:

    // Constructor
    Conversation();

    // Destructor
    ~Conversation();

    // Copy constructor
    Conversation(const Conversation& other);

    // Copy assignment
    Conversation& operator=(const Conversation& other);

    // Move constructor
    Conversation(Conversation&& other) noexcept;

    // Move assignment
    Conversation& operator=(Conversation&& other) noexcept;

    // Add a message
    void append(Message m);

    // Return the number of messages
    size_t size() const noexcept;

    // Return the message at position i
    const Message& at(size_t i) const;

    // Return a pointer to the first message
    const Message* begin() const noexcept;

    // Return a pointer to one position after the last message
    const Message* end() const noexcept;

private:

    // Dynamic array that stores the messages
    Message* data_ = nullptr;  

    // Number of messages currenlty stored
    size_t size_ = 0;      
    
    // Number of messages the current array can hold
    size_t capacity_ = 0;      
};