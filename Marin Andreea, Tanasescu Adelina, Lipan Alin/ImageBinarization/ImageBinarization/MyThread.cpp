
/**
 * Developed by Marin Andreea - Lavinia.
 * Code reviewed (including coding style) by Lipan Alin - Ionut.
 */

#include "stdafx.h"
#include "MyThread.h"


MyThread::MyThread() :
    m_handle(0),
    m_systemID(0),
    m_id(0),
    m_fp(NULL)
{ }


MyThread::~MyThread()
{
    if (m_handle)
    {
        CloseHandle(m_handle);
        m_handle = 0;
    }
}


void MyThread::InitThread(void(*f)(void*, int),
                          void* object,
                          int id)
{
    m_id = id;
    m_object = object;
    m_fp = f;
}


void MyThread::CreateThread()
{
    m_handle = ::CreateThread(NULL, 0, ThreadProc,
                              this, 0, &m_systemID);
}


DWORD WINAPI MyThread::ThreadProc(_In_ LPVOID lpParameter)
{
    // get the MyThread object (above 'this' parameter) and call its function through its pointer
    MyThread* thisObject = static_cast<MyThread*>(lpParameter);

    // now call it's function and pass the ChristianBinarization object to it because it's static
    thisObject->m_fp(thisObject->m_object, thisObject->m_id);

    return 0;
}