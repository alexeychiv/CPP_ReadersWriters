#include <windows.h>
#include <stdio.h>

#include <atomic>


#define START_DELAY 3000

#define READERS_QUANTITY 2
#define WRITERS_QUANTITY 2

#define READ_TIME 1000
#define READER_DELAY 1000

#define WRITE_TIME 3000
#define WRITER_DELAY 8000


int shared_resource = 0;


HANDLE startEvent;

HANDLE writeMutex;
HANDLE readAllowed;
HANDLE writeAllowed;

std::atomic<int> writerCounter;

std::atomic<int> readerCounter;

DWORD WINAPI ReadThreadProc(LPVOID);
DWORD WINAPI WriteThreadProc(LPVOID);


int main( void )
{
    printf("\n\n===================================================================================================================================\n\n");
    
    startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    writeMutex = CreateMutex(NULL, FALSE, NULL);
    readAllowed = CreateEvent(NULL, TRUE, TRUE, "Read Allowed");
    writeAllowed = CreateEvent(NULL, TRUE, TRUE, "Write Allowed");
    
    writerCounter = 0;
    
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

DWORD WINAPI ReadThreadProc(LPVOID lpParam) 
{
    printf("READER Thread ID#%lu started!\n", GetCurrentThreadId());
    
    WaitForSingleObject(startEvent, INFINITE);
    
    while (true)
    {
        printf("READER Thread ID#%lu wants to read...\n", GetCurrentThreadId());
        
        WaitForSingleObject(readAllowed, INFINITE);
        
        ResetEvent(writeAllowed);
        ++readerCounter;
        
        printf("READER Thread ID#%lu reading... (readerCounter = %d)\n", GetCurrentThreadId(), readerCounter.load());
        
        Sleep(READ_TIME);
        
        printf("READER Thread ID#%lu done reading... Shared_resource = %d\n", GetCurrentThreadId(), shared_resource);
        
        --readerCounter;
        if (readerCounter.load() == 0)
            SetEvent(writeAllowed);
        
        Sleep(READER_DELAY);
    }
    
    ExitThread(0);
}

DWORD WINAPI WriteThreadProc(LPVOID lpParam) 
{
    printf("WRITER Thread ID#%lu started!\n", GetCurrentThreadId());
    
    WaitForSingleObject(startEvent, INFINITE);
    
    while (true)
    {
        Sleep(WRITER_DELAY);
        
        ResetEvent(readAllowed);
        ++writerCounter;
        
        printf("\nWRITER Thread ID#%lu wants to write (writerCounter = %d)...\n\n", GetCurrentThreadId(), writerCounter.load());
        
        WaitForSingleObject(writeAllowed, INFINITE);
        WaitForSingleObject(writeMutex, INFINITE);
        
        printf("\nWRITER Thread ID#%lu writing...\n\n", GetCurrentThreadId());
        
        ++shared_resource;
        
        Sleep(WRITE_TIME);
        
        printf("\nWRITER Thread ID#%lu done writing... Shared_resource = %d\n\n", GetCurrentThreadId(), shared_resource);
        
        ReleaseMutex(writeMutex);
        
        --writerCounter;
        if (writerCounter.load() == 0)
            SetEvent(readAllowed);
    }
    
    ExitThread(0);
}
