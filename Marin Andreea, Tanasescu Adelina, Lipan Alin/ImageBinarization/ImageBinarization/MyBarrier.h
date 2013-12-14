
/**
 * Developed by Marin Andreea - Lavinia.
 * Code reviewed (including coding style) by Lipan Alin - Ionut.
*/

#ifndef __MY_BARRIER__
#define __MY_BARRIER__

#include <Windows.h>

// Barrier using windows events !!!
// Each thread has it's own event and will leave the barrier after all threads have entered it !
// After all threads entered the barrier is 'destroyed' and must be 'reconstructed' for further use !
class MyBarrier
{
private:
    // prevent copying and assignment
    MyBarrier(const MyBarrier&);
    MyBarrier& operator=(const MyBarrier&);

    HANDLE* barrier;

    int limit;

public:
    // Constructor
    MyBarrier();

    // Destructor
    ~MyBarrier();

    bool Init(const int& limit);

    // Threads will block until all other threads have entered !
    // After all threads entered, the barrier is 'destroyed' (threads won't block again if they enter it) !
    // After the barrier is destroyed, it must be 'reconstructed', each entered thread must call Prepare(threadID) !
    void EnterBarrier(const int& id);

    // Prepares a barrier, must be called by $limit threads with their own id in order to 'reconstruct' this barrier !
    void Prepare(const int& id);
};

#endif
