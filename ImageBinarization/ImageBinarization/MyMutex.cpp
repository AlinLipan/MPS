
/**
 * Developed by Marin Andreea - Lavinia.
 * Code reviewed (including coding style) by Lipan Alin - Ionut.
 */

#include "stdafx.h"
#include "MyMutex.h"


MyMutex::MyMutex() :
    m_handle(0)
{ }


MyMutex::~MyMutex()
{
    if (m_handle)
    {
        CloseHandle(m_handle);

        m_handle = 0;
    }
}


void MyMutex::Create()
{
    m_handle = ::CreateMutex(0, false, 0);
}


DWORD MyMutex::LockMutex()
{
    return WaitForSingleObject(m_handle, INFINITE);
}


BOOL MyMutex::UnlockMutex()
{
    return ReleaseMutex(m_handle);
}