# Project 3 Design Log
**Author:** Nolan Smeal · **P2 code used:** Nolan Smeal
**Tools used:** ChatGPT to help review and revise code, troubleshoot compilation and testing issues, and improve documentation

## 1. Hash table
For the ToolRegistry class, I implemented a hash table using separate chaining. Each bucket contains a vector of unique_ptr<Tool> objects, allowing multiple tools to share a bucket when collisions occur. I chose separate chaining instead of open addressing because it makes collisions easier to handle. I used the FNV-1a hash function to distribute tool names across the buckets. The table starts with 16 buckets and doubles its capacity whenever the load factor would exceed 0.75.
To test the hash table, I inserted 10,000 tools named tool_0 through tool_9999. The final bucket count was 16,384, the load factor was 0.6104, and the maximum chain length was 6. I also compared FNV-1a against a weaker hash function using the first character of the name plus its length. The weaker function produced a maximum chain length of 9,000, showing that FNV-1a distributed the tools more evenly.
Although resizing requires rehashing all tools and takes O(n) time, it does not happen on every insertion. Since capacity doubles, resizing happens less frequently as the table grows. Therefore, add() has an expected amortized cost of O(1).

## 2. Pointer stability
One requirement for ToolRegistry was ensuring that pointers returned by find() remain valid after resizing. I used unique_ptr<Tool> to manage ownership of the tools. When the hash table resizes, the unique_ptr objects move between buckets, but the actual Tool objects remain at their original memory addresses. Therefore, a Tool* returned by find() remains valid after resizing.
I tested this by saving tool pointers, inserting additional tools, and comparing the addresses afterward. My test with 10,000 tools confirmed that the original pointers remained valid. Using unique_ptr also simplifies memory management because the tools are automatically deleted when the registry is destroyed.

## 3. Calculator trace
For the calculator, I used two stacks: one for numerical values and another for operators. The calculator processes tokens from left to right while following operator precedence.
For the expression 2 times (3 + 4) minus 5, the calculator first pushes 2 onto the values stack, giving [2]. It then pushes multiplication onto the operators stack. The opening parenthesis is pushed next. The number 3 is added to the values stack, giving [2, 3]. The addition operator is then pushed onto the operators stack. Next, 4 is added to the values stack, giving [2, 3, 4].
When the closing parenthesis is encountered, the calculator evaluates 3 + 4. The values stack becomes [2, 7], and the multiplication operator remains on the operators stack. When subtraction is encountered, multiplication is evaluated first, producing 14. The values stack becomes [14], and the subtraction operator is pushed onto the operators stack. Finally, 5 is pushed onto the values stack, giving [14, 5]. At the end, the calculator evaluates 14 - 5, producing 9.

## 4. Errors as values
For error handling, I used return values and error messages instead of exceptions for expected failures. For example, Workspace::resolve() returns false when a path is invalid, while calc_evaluate() returns false when an expression cannot be evaluated. Division by zero produces the error message "division by zero."
I chose this approach because invalid inputs are expected during normal program operation. Returning errors allows the program to explain problems without terminating. Exceptions are more appropriate for unexpected programming errors or situations where normal execution cannot continue safely.

## 5. What I would design differently
If I improved this project, I would write more edge-case tests before implementing the classes. Initially, I focused on basic operations before testing invalid paths, negative numbers, and large expressions. Testing these cases earlier would have made debugging easier and helped identify problems sooner. Overall, this project improved my understanding of hash tables, smart pointers, stacks, and error handling in C++.