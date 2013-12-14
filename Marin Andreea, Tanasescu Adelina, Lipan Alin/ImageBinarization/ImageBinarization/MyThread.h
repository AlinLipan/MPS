
/**
 * Developed by Marin Andreea - Lavinia.
 * Code reviewed (including coding style) by Lipan Alin - Ionut.
 */

#ifndef __MY_THREAD_H__
#define __MY_THREAD_H__

#include <Windows.h>

class MyThread
{
private:
    // prevent copying
    MyThread(const MyThread&);
    MyThread& operator=(const MyThread&);

public:
    // constructor
    MyThread();

    // destructor
    ~MyThread();

    // init thread with his entry point, ChristianBinarization object and id
    void InitThread(void(*f)(void *, int),
                    void* object,
                    int id);

    // create thread
    void CreateThread();

    // get handle
    HANDLE GetHandle() { return m_handle; }

private:
    // entry point for the new thread
    static DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter);

    // DATA MEMBERS
    // handle
    HANDLE m_handle;

    // operating system thread id
    DWORD m_systemID;

    // id
    int m_id;

    // this will represent the ChristianBinarization object (this)
    void* m_object;

    // function pointer... thread will eventually get here :D
    // first argument represents the ChristianBinarization object (this)
    // second represents the thread id number (used to distribute work)
    void (*m_fp)(void*, int);
};

#endif
