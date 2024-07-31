#include <iostream>
#include <thread>
#include <queue>
#include <random>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <functional>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <cstdio>

// Generates a random number between the two bounds given.
int gen_rand_num(unsigned int lower_bound, unsigned int upper_bound) {
    int range = upper_bound - lower_bound;
    int num = rand() % range + lower_bound;
    return num;
}

// Generates a random string of the length passed in.
std::string gen_rand_string(unsigned int length)
{
    const std::string CHARACTERS = "abcdefghijklmnopqrstuvwxyz";
    std::string random_string;
    for (unsigned int i = 0; i < length; ++i)
    {
        int rand_index = gen_rand_num(0, 25);
        random_string += CHARACTERS[rand_index];
    }
    return random_string;
}

// Class for objects storing the data of each Node
class data_item {
    int a;
    std::string b;

public:
    data_item(int new_num, std::string new_string) {
        a = new_num;
        b = new_string;
    }

    int get_num() {return a;}

    void print_vals() {
        std::string output = "string: " + b + ". int: " + std::to_string(a) + "\n";
        std::cout << output;
    }
};

// Class for elements of the double linked list. 
struct Node {
    data_item* data;
    Node* right = NULL;
    Node* left = NULL;
    std::mutex m;

    Node(data_item* d) {
        data = d;
    }
};

// Our "Queue" - implemented as a double linked list
// I imagine it as being arranged from left to right. Reversing the list just involves 
// changing the direction from going right to going left or vice versa.
struct double_linked_list {
    // These pointers store the two 'heads' of the list.
    Node* leftmost = NULL;
    Node* rightmost = NULL;
    // This is our direction variable. 
    bool going_right = true;

    // This mutex is for the direction variable.
    std::mutex dirmut;
    // This mutex is for the 'leftmost' pointer
    std::mutex leftmut;
    // This mutex is for the 'rightmost' pointer.
    std::mutex rightmut;
    int queue_size = 0;
    bool empty = true;

    double_linked_list() {}

    void initialise_queue(int size) {
        // This is generating the first Node to append to the list.
        int string_length = gen_rand_num(3,7);
        std::string new_string = gen_rand_string(string_length);
        int new_num = gen_rand_num(0,255);
        data_item* new_item = new data_item(new_num, new_string);
        Node* newNode = new Node(new_item);
        leftmost = newNode;
        rightmost = newNode;
        queue_size++;
        empty = false;

        // This creates the number of nodes dictated by the 'size' parameter given.
        for (int i = 1; i < size; i++) {
            string_length = gen_rand_num(3,7);
            new_string = gen_rand_string(string_length);
            new_num = gen_rand_num(0,255);
            data_item* new_item = new data_item(new_num, new_string);
            push(new_item);
            queue_size++;
        }
    }

    void push(data_item* new_data) {

        //allocate memory for New node
        Node* newNode = new Node(new_data);
        if(going_right)
        {
            // current rightmost is to the left of the new node
            // NULL is to the right of the new node.
            newNode->left = rightmost;
            newNode->right = NULL;

            //right of current rightmost is new node
            if(rightmost != NULL){
                rightmost->right = newNode;
            }
            
            //rightmost is changed to the New Node.
            rightmost = newNode;
        }
        else
        {
            // current leftmost is to the right of the new node
            // NULL is to the left of the new node.
            newNode->right = leftmost;
            newNode->left = NULL;

            //left of current leftmost is new node
            if(leftmost != NULL){
                leftmost->left = newNode;
            }

            //leftmost is changed to the New Node.
            leftmost = newNode;
        }
    }

    void pop() {
        if(rightmost!=NULL)
        {
            if(going_right)
            {
                Node* nodeRemove = leftmost;
                Node* newLeft = leftmost->right;
                leftmost = newLeft;
                if(leftmost == NULL) 
                {
                    rightmost = NULL;
                    empty = true;
                }
                delete nodeRemove;
            }
            else
            {
                Node* nodeRemove = rightmost;
                Node* newRight = rightmost->left;
                rightmost = newRight;
                if(rightmost==NULL) 
                {
                    leftmost = NULL;
                    empty = true;
                }
                delete nodeRemove;
            }
        }
    }

    bool change_direction() {
        going_right = !going_right;
        return going_right;
    }

    data_item* top() {
        if(going_right)
        {
            return leftmost->data;
        }
        else
        {
            return rightmost->data;
        }
    }
};

void reversing_thread(double_linked_list* queue) {
    // This thread reverses the list and then sums the integers of all nodes still
    // in the list, which is then outputted. 
    //
    // We first lock the direction and then change it. We keep the direction locked
    // during summing to ensure no more items are deleted. Summing is done by traversing
    // the list using the current and next pointers. No nodes are locked during summing, as
    // we are only reading values, and the delete thread is also paused.

    // These are used for the integer sum. 
    Node* current;
    Node* next;
    bool going_right;
    std::string output;

    while(!queue->empty)
    {
        // This sleep seems neccessary for the direction mutex to be able to hand over. 
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        // lock the direction
        std::lock_guard<std::mutex> lock_dir(queue->dirmut);

        // The queue could have been made empty between the check and locking the direction,
        // so we check again.
        if(queue->empty)
        {
            std::cout << "Reversing Thread Finished.\n \n";
            return;
        }
        // change the direction of the queue.
        going_right = queue->change_direction();


        // sum the integers
        int sum = 0;
        if(going_right)
        {
            // if going right, we start at the left.
            current = queue->leftmost;
            sum += current->data->get_num();
            next = current->right;
            while(next != NULL) // if next is null, we have reached the end of the list.
            {
                current = next;
                sum += current->data->get_num();
                next = current->right;
            }
        }
        else
        {
            // if going left, we start at the right.
            current = queue->rightmost;
            sum += current->data->get_num();
            next = current->left;
            while(next != NULL) // if next is null, we have reached the end of the list.
            {
                current = next;
                sum += current->data->get_num();
                next = current->left;
            }
        }
        output = "Sum: " + std::to_string(sum) + "\n \n";
        std::cout << output;
        // the direction lock goes out of scope, so it unlocks.
    }
    std::cout << "Reversing Thread Finished.\n \n";
    return;
}

void deleting_thread(double_linked_list* queue) {
    // This thread deletes a node in the list every 0.2 seconds. 
    //
    // We first choose the index of the node to the delete, and then lock the direction to 
    // stop reversals. In most cases, we lock the node before, itself, and the node after of
    // the node chosen in that order to delete the node and change the pointers appropriately.
    // This also prevents the printing thread from deadlocking with this thread. 
    //
    // When the node selected is a head (leftmost or rightmost) we lock the appropriate head 
    // pointer as well. This is to prevent a deadlock with the printing thread.
    //
    // Due to the choice I have made in how to reverse the order, I have had to implement similar
    // code for the range of scenarios that can occur. This is one downside to this method that 
    // I wasn't sure how to get past.

    // set the seed
    srand((unsigned)time(NULL));

    Node* prev_node;
    Node* del_node;
    Node* next_node;
    bool going_right;

    while(queue->queue_size > 1)
    {
        // delete a node every 0.2 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        int node_to_delete = gen_rand_num(0, queue->queue_size-1);

        // lock the direction variable. This means it can't be changed by the reverse thread.
        std::lock_guard<std::mutex> lock_dir(queue->dirmut);
        going_right = queue->going_right;
        if(going_right)
        {
            prev_node = queue->leftmost;

            if(node_to_delete == 0) // we are deleting the leftmost node. 
            {
                // we must first lock the pointer to leftmost, so that the 
                // printing thread does not loop to the front of the list again.
                std::lock_guard<std::mutex> lock_left(queue->leftmut);
                prev_node = NULL;
                del_node = queue->leftmost;
                next_node = del_node->right;

                // acquire the relevant node mutexes in the appropriate order. 
                std::lock_guard<std::mutex> lock_del(del_node->m);
                std::lock_guard<std::mutex> lock_next(next_node->m);

                // switch the pointers
                queue->leftmost = next_node;
                next_node->left = NULL;
            }
            else if(node_to_delete == queue->queue_size)
            {
                std::lock_guard<std::mutex> lock_right(queue->rightmut);
                del_node = queue->rightmost;
                prev_node = del_node->left;
                next_node =  NULL;

                std::lock_guard<std::mutex> lock_prev(prev_node->m);
                std::lock_guard<std::mutex> lock_del(del_node->m);
                queue->rightmost = prev_node;
                prev_node->right = NULL;
            }
            else
            {
                // need to loop through pointers depending on the node to delete.
                if(node_to_delete > 1)
                {
                    for(int i = 0; i < node_to_delete-2; i++)
                    {
                        prev_node = prev_node->right;
                    }
                }    
                std::lock_guard<std::mutex> lock_prev(prev_node->m);
                del_node = prev_node->right;
                std::lock_guard<std::mutex> lock_del(del_node->m);
                next_node =  del_node->right;
                std::lock_guard<std::mutex> lock_next(next_node->m);

                // change the pointers for the nodes either side
                prev_node->right = next_node;
                next_node->left = prev_node;
            }
            // delete the middle node
            delete del_node;
            queue->queue_size -= 1;
        }
        else
        {
            prev_node = queue->rightmost;

            if(node_to_delete == 0)
            {
                std::lock_guard<std::mutex> lock_right(queue->rightmut);

                prev_node = NULL;
                del_node = queue->rightmost;
                next_node = del_node->left;

                std::lock_guard<std::mutex> lock_del(del_node->m);
                std::lock_guard<std::mutex> lock_next(next_node->m);

                queue->rightmost = next_node;
                next_node->right = NULL;
            }
            else if(node_to_delete == queue->queue_size)
            {
                std::lock_guard<std::mutex> lock_left(queue->leftmut);
                del_node = queue->leftmost;
                prev_node = del_node->right;
                next_node =  NULL;

                std::lock_guard<std::mutex> lock_prev(prev_node->m);
                std::lock_guard<std::mutex> lock_del(del_node->m);

                queue->leftmost = prev_node;
                prev_node->left = NULL;
            }
            else
            {
                // need to loop through pointers depending on the node to delete.
                if(node_to_delete > 1)
                {
                    for(int i = 0; i < node_to_delete-2; i++)
                    {
                        prev_node = prev_node->left;
                    }
                }    

                std::lock_guard<std::mutex> lock_prev(prev_node->m);
                del_node = prev_node->left;
                std::lock_guard<std::mutex> lock_del(del_node->m);
                next_node =  del_node->left;
                std::lock_guard<std::mutex> lock_next(next_node->m);

                // change the pointers for the nodes either side
                prev_node->left = next_node;
                next_node->right = prev_node;
            }
            // delete the middle node
            delete del_node;
            queue->queue_size -= 1;
        }
    }
    // When queue has only one item, it just needs to be deleted.
    // delete a node every 0.2 seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(200));


    std::lock_guard<std::mutex> lock_dir(queue->dirmut);
    std::lock_guard<std::mutex> lock_right(queue->rightmut);
    std::lock_guard<std::mutex> lock_left(queue->leftmut);
    del_node = queue->rightmost;
    queue->empty = true;

    std::lock_guard<std::mutex> lock_del(del_node->m);
    queue->rightmost = NULL;
    queue->leftmost = NULL;

    queue->queue_size = 0;
    delete del_node;

    std::cout << "Deleting Thread Finished.\n \n";
}

void printing_thread(double_linked_list* queue) {
    // This thread loops through the queue, printing the string and integer of
    // every item in the list. 
    //
    // We lock the direction of the list every time we complete a loop of all the nodes
    // or the list gets reversed. This is to safely start at the beginning of the list.
    // We then lock the appropriate head pointer and unlock our direction to allow deletion and reversal.
    // We can then lock the node itself and print the data. We continue through the list with the
    // hand-over-hand procedure, locking the next node before we unlock the current one. 



    Node* current;
    std::unique_lock<std::mutex> lock_dir(queue->dirmut);
    while(!queue->empty)
    {
        if(queue->going_right)
        {
            std::unique_lock<std::mutex> lock_left(queue->leftmut);
            current = queue->leftmost;
            lock_dir.unlock();

            std::unique_lock<std::mutex> lock_current(current->m);
            current->data->print_vals();
            lock_left.unlock();
        
            while(queue->going_right && current->right != NULL)
            {
                std::unique_lock<std::mutex> lock_next(current->right->m);
                current = current->right;
                current->data->print_vals();
                lock_current.swap(lock_next);
                lock_next.unlock();
            }
        }
        else
        {
            std::unique_lock<std::mutex> lock_right(queue->rightmut);
            current = queue->rightmost;
            lock_dir.unlock();

            std::unique_lock<std::mutex> lock_current(current->m);
            current->data->print_vals();
            lock_right.unlock();

            while(!queue->going_right && current->left != NULL)
            {
                std::unique_lock<std::mutex> lock_next(current->left->m);
                current = current->left;
                current->data->print_vals();
                lock_current.swap(lock_next);
                lock_next.unlock();
            }
        }
        lock_dir.lock();
    }
    std::cout << "Printing Thread Finished.\n\n" << std::endl;
}

int main() {
    // freopen("output.txt","w",stdout);
    srand((unsigned)time(NULL));
    double_linked_list* queue = new double_linked_list();
    int queue_size = 80;
    queue->initialise_queue(queue_size);
    
    std::thread printing(printing_thread, std::ref(queue));
    std::thread deleting(deleting_thread, std::ref(queue));
    std::thread reversing(reversing_thread, std::ref(queue));

    deleting.join();
    printing.join();
    reversing.join();

    return 0;
}