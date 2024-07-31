# cpp_concurrency
Using C++ to solve a novel concurrency problem.
To solidify learnings of concurrency and deadlocking in C++, this repository shows the solution to solving the following task:

a. Implement a queue of items consisting of a single std::string and an integer.

b. Instantiate the queue and populate it with 80 items with strings of 3 to 7 lower-case alphabet characters inclusive randomly chosen, and integers randomly chosen between 0 and 255.

c. Start a background thread that reverses the ordering of all the items present in the queue and then outputting the sum of all the present items in the queue. This thread should always run whilst there are items remaining in the queue.

d. Start another background thread that sequentially prints all string and integer values for all items currently present in the queue. 

e. Start a third background thread that randomly deletes an item from the queue every 0.2 seconds.

## Running Solution
To build, run `g++ -Wall --std=c++11 solution.cc -o solution`.
To run, run `./solution`.