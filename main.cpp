#include <windows.h>
#include <stdio.h>

//========================================================================================

#define START_DELAY 1000

#define READERS_QUANTITY 2
#define WRITERS_QUANTITY 2

#define READ_TIME 1000
#define READER_DELAY 1000

#define WRITE_TIME 3000
#define WRITER_DELAY 8000

//========================================================================================

HANDLE startEvent;

HANDLE writeMutex;
HANDLE readAllowed;
HANDLE writeAllowed;

HANDLE writeCounterMutex;
int writerCounter;

HANDLE readCounterMutex;
int readerCounter;

//========================================================================================

DWORD WINAPI ReadThreadProc(LPVOID);
DWORD WINAPI WriteThreadProc(LPVOID);

//========================================================================================
//========================================================================================
//========================================================================================

int main( void )
{
    printf("\n\n===================================================================================================================================\n\n");
    
    startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    writeMutex = CreateMutex(NULL, FALSE, NULL);
    readAllowed = CreateEvent(NULL, TRUE, TRUE, "Read Allowed");
    writeAllowed = CreateEvent(NULL, TRUE, TRUE, "Write Allowed");
    
    writeCounterMutex = CreateMutex(NULL, FALSE, NULL);
    writerCounter = 0;
    
    readCounterMutex = CreateMutex(NULL, FALSE, NULL);
    readerCounter = 0;
    
    
    HANDLE hnd_readerThread[READERS_QUANTITY];
    HANDLE hnd_writerThread[WRITERS_QUANTITY];
    
    for (int i = 0; i < READERS_QUANTITY; ++i)
        hnd_readerThread[i] = CreateThread(NULL, 0, ReadThreadProc, NULL, 0, NULL);
        
    for (int i = 0; i < WRITERS_QUANTITY; ++i)
        hnd_writerThread[i] = CreateThread(NULL, 0, WriteThreadProc, NULL, 0, NULL);
    
    Sleep(START_DELAY);
    printf("\n>> START <<\n\n");
    SetEvent(startEvent);
    
    WaitForMultipleObjects(READERS_QUANTITY, hnd_readerThread, true, INFINITE);
    WaitForMultipleObjects(WRITERS_QUANTITY, hnd_writerThread, true, INFINITE);
    
    return 0;
}

//========================================================================================
//========================================================================================
//========================================================================================

DWORD WINAPI ReadThreadProc(LPVOID lpParam) 
{
    printf("READER Thread ID#%lu started!\n", GetCurrentThreadId());
    
    WaitForSingleObject(startEvent, INFINITE);
    
    DWORD dwWaitResult;
    
    while (true)
    {
        printf("READER Thread ID#%lu wants to read...\n", GetCurrentThreadId());
        
        WaitForSingleObject(readAllowed, INFINITE);
        
        WaitForSingleObject(readCounterMutex, INFINITE);
        ++readerCounter;
        if (readerCounter == 1)
            ResetEvent(writeAllowed);
        ReleaseMutex(readCounterMutex);
        
        printf("READER Thread ID#%lu reading... (readerCounter = %d)\n", GetCurrentThreadId(), readerCounter);
        
        Sleep(READ_TIME);
        
        printf("READER Thread ID#%lu done reading...\n", GetCurrentThreadId());
        
        WaitForSingleObject(readCounterMutex, INFINITE);
        --readerCounter;
        if (readerCounter == 0)
            SetEvent(writeAllowed);
        ReleaseMutex(readCounterMutex);
        
        Sleep(READER_DELAY);
    }
    
    ExitThread(0);
}

//========================================================================================

DWORD WINAPI WriteThreadProc(LPVOID lpParam) 
{
    printf("WRITER Thread ID#%lu started!\n", GetCurrentThreadId());
    
    WaitForSingleObject(startEvent, INFINITE);
    
    
    while (true)
    {
        Sleep(WRITER_DELAY);
        
        WaitForSingleObject(writeCounterMutex, INFINITE);
        ++writerCounter;
        if (writerCounter == 1)
            ResetEvent(readAllowed);
        ReleaseMutex(writeCounterMutex);
        
        printf("\nWRITER Thread ID#%lu wants to write (writerCounter = %d)...\n\n", GetCurrentThreadId(), writerCounter);
        
        WaitForSingleObject(writeAllowed, INFINITE);
        WaitForSingleObject(writeMutex, INFINITE);
        
        printf("\nWRITER Thread ID#%lu writing...\n\n", GetCurrentThreadId());
        
        Sleep(WRITE_TIME);
        
        printf("\nWRITER Thread ID#%lu done writing...\n\n", GetCurrentThreadId());
        
        ReleaseMutex(writeMutex);
        
        WaitForSingleObject(writeCounterMutex, INFINITE);
        --writerCounter;
        if (writerCounter == 0)
            SetEvent(readAllowed);
        ReleaseMutex(writeCounterMutex);
    }
    
    ExitThread(0);
}
