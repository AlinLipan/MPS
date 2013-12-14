
/**
 * Developed by Marin Andreea - Lavinia.
 * Code reviewed (including coding style) by Lipan Alin - Ionut.
 */

#include "stdafx.h"
#include "MyBarrier.h"


MyBarrier::MyBarrier():
    barrier(NULL)
{
    // Nothing to do here...
}


MyBarrier::~MyBarrier()
{
    if (barrier)
    {
        for (int i = 0; i < limit; ++i)
        {
            if (barrier[i])
                CloseHandle(barrier[i]);
        }

        delete[] barrier;
        barrier = NULL;
    }
}


bool MyBarrier::Init(const int& limit)
{
    this->limit = limit;

    barrier = new HANDLE[limit];

    for (int i = 0; i < limit; ++i)
    {
        barrier[i] = CreateEvent(NULL, true, false, NULL);

        if (barrier[i] == NULL)
        {
            for (int j = 0; j < i; ++j)
                CloseHandle(barrier[j]);

            delete[] barrier;

            return false;
        }
    }

    return true;
}


void MyBarrier::EnterBarrier(const int& id)
{
    SetEvent(barrier[id]);
    WaitForMultipleObjects(limit, barrier, true, INFINITE);
}


void MyBarrier::Prepare(const int& id)
{
    ResetEvent(barrier[id]);
}
