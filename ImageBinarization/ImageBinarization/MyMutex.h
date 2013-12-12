
/**
 * Developed by Marin Andreea - Lavinia.
 * Code reviewed (including coding style) by Lipan Alin - Ionut.
 */

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <Windows.h>

class MyMutex
{
public:
    // constructor
    MyMutex();

    // destructor
    ~MyMutex();

    // create
    void Create();

    // lock
    virtual DWORD LockMutex();

    // unlock
    virtual BOOL UnlockMutex();

    HANDLE m_handle;
};

#endif
