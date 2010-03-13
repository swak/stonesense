/*
www.sourceforge.net/projects/dfhack
Copyright (c) 2009 Petr Mrázek (peterix), Kenneth Ferland (Impaler[WrG]), dorf

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#include "DFCommonInternal.h"
using namespace DFHack;

class NormalProcess::Private
{
    public:
        Private()
        {
            my_descriptor = NULL;
            my_handle = NULL;
            my_main_thread = NULL;
            my_window = NULL;
            my_pid = 0;
            attached = false;
            suspended = false;
        };
        ~Private(){};
        memory_info * my_descriptor;
        DFWindow * my_window;
        ProcessHandle my_handle;
        HANDLE my_main_thread;
        uint32_t my_pid;
        string memFile;
        bool attached;
        bool suspended;
        bool identified;
};

NormalProcess::NormalProcess(uint32_t pid, vector <memory_info *> & known_versions)
: d(new Private())
{
    HMODULE hmod = NULL;
    DWORD junk;
    HANDLE hProcess;
    bool found = false;
    
    IMAGE_NT_HEADERS32 pe_header;
    IMAGE_SECTION_HEADER sections[16];
    d->identified = false;
    // open process
    hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
    if (NULL == hProcess)
        return;
    
    // try getting the first module of the process
    if(EnumProcessModules(hProcess, &hmod, 1 * sizeof(HMODULE), &junk) == 0)
    {
        CloseHandle(hProcess);
        cout << "EnumProcessModules fail'd" << endl;
        return; //if enumprocessModules fails, give up
    }
    
    // got base ;)
    uint32_t base = (uint32_t)hmod;
    
    // temporarily assign this to allow some checks
    d->my_handle = hProcess;
    // read from this process
    uint32_t pe_offset = readDWord(base+0x3C);
    read(base + pe_offset                   , sizeof(pe_header), (uint8_t *)&pe_header);
    read(base + pe_offset+ sizeof(pe_header), sizeof(sections) , (uint8_t *)&sections );
    d->my_handle = 0;
    
    // see if there's a version entry that matches this process
    vector<memory_info*>::iterator it;
    for ( it=known_versions.begin() ; it < known_versions.end(); it++ )
    {
        // filter by OS
        if(memory_info::OS_WINDOWS != (*it)->getOS())
            continue;
        uint32_t pe_timestamp;
        // filter by timestamp, skip entries without a timestamp
        try
        {
            pe_timestamp = (*it)->getHexValue("pe_timestamp");
        }
        catch(Error::MissingMemoryDefinition& e)
        {
            continue;
        }
        if (pe_timestamp != pe_header.FileHeader.TimeDateStamp)
            continue;
        
        // all went well
        {
            printf("Match found! Using version %s.\n", (*it)->getVersion().c_str());
            d->identified = true;
            // give the process a data model and memory layout fixed for the base of first module
            memory_info *m = new memory_info(**it);
            m->RebaseAll(base);
            // keep track of created memory_info object so we can destroy it later
            d->my_descriptor = m;
            // process is responsible for destroying its data model
            d->my_pid = pid;
            d->my_handle = hProcess;
            d->identified = true;
            
            // TODO: detect errors in thread enumeration
            vector<uint32_t> threads;
            getThreadIDs( threads );
            d->my_main_thread = OpenThread(THREAD_ALL_ACCESS, FALSE, (DWORD) threads[0]);
            
            found = true;
            break; // break the iterator loop
        }
    }
    // close handle of processes that aren't DF
    if(!found)
    {
        CloseHandle(hProcess);
    }
    else
    {
        d->my_window = new DFWindow(this);
    }
}
/*
*/

NormalProcess::~NormalProcess()
{
    if(d->attached)
    {
        detach();
    }
    // destroy our rebased copy of the memory descriptor
    delete d->my_descriptor;
    if(d->my_handle != NULL)
    {
        CloseHandle(d->my_handle);
    }
    if(d->my_main_thread != NULL)
    {
        CloseHandle(d->my_main_thread);
    }
    if(d->my_window)
    {
        delete d->my_window;
    }
    delete d;
}

memory_info * NormalProcess::getDescriptor()
{
    return d->my_descriptor;
}

DFWindow * NormalProcess::getWindow()
{
    return d->my_window;
}

int NormalProcess::getPID()
{
    return d->my_pid;
}

bool NormalProcess::isSuspended()
{
    return d->suspended;
}
bool NormalProcess::isAttached()
{
    return d->attached;
}

bool NormalProcess::isIdentified()
{
    return d->identified;
}

bool NormalProcess::asyncSuspend()
{
    return suspend();
}

bool NormalProcess::suspend()
{
    if(!d->attached)
        return false;
    if(d->suspended)
    {
        return true;
    }
    SuspendThread(d->my_main_thread);
    d->suspended = true;
    return true;
}

bool NormalProcess::forceresume()
{
    if(!d->attached)
        return false;
    while (ResumeThread(d->my_main_thread) > 1);
    d->suspended = false;
    return true;
}


bool NormalProcess::resume()
{
    if(!d->attached)
        return false;
    if(!d->suspended)
    {
        return true;
    }
    ResumeThread(d->my_main_thread);
    d->suspended = false;
    return true;
}

bool NormalProcess::attach()
{
    if(g_pProcess != NULL)
    {
        return false;
    }
    d->attached = true;
    g_pProcess = this;
    suspend();

    return true;
}


bool NormalProcess::detach()
{
    if(!d->attached)
    {
        return false;
    }
    resume();
    d->attached = false;
    g_pProcess = NULL;
    return true;
}

bool NormalProcess::getThreadIDs(vector<uint32_t> & threads )
{
    HANDLE AllThreads = INVALID_HANDLE_VALUE; 
    THREADENTRY32 te32;
    
    AllThreads = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
    if( AllThreads == INVALID_HANDLE_VALUE ) 
    {
        return false; 
    }
    te32.dwSize = sizeof(THREADENTRY32 ); 
    
    if( !Thread32First( AllThreads, &te32 ) ) 
    {
        CloseHandle( AllThreads );
        return false;
    }
    
    do 
    { 
        if( te32.th32OwnerProcessID == d->my_pid )
        {
            threads.push_back(te32.th32ThreadID);
        }
    } while( Thread32Next(AllThreads, &te32 ) ); 
    
    CloseHandle( AllThreads );
    return true;
}

//FIXME: use VirtualQuery to probe for memory ranges, cross-reference with base-corrected PE segment entries
void NormalProcess::getMemRanges( vector<t_memrange> & ranges )
{
    // code here is taken from hexsearch by Silas Dunmore.
    // As this IMHO isn't a 'sunstantial portion' of anything, I'm not including the MIT license here
    
    // I'm faking this, because there's no way I'm using VirtualQuery
    
    t_memrange temp;
    uint32_t base = d->my_descriptor->getBase();
    temp.start = base + 0x1000; // more fakery.
    temp.end = base + readDWord(base+readDWord(base+0x3C)+0x50)-1; // yay for magic.
    temp.read = 1;
    temp.write = 1;
    temp.execute = 0; // fake
    strcpy(temp.name,"pants");// that's right. I'm calling it pants. Windows can go to HELL
    ranges.push_back(temp);
}

uint8_t NormalProcess::readByte (const uint32_t offset)
{
    uint8_t result;
    ReadProcessMemory(d->my_handle, (int*) offset, &result, sizeof(uint8_t), NULL);
    return result;
}

void NormalProcess::readByte (const uint32_t offset,uint8_t &result)
{
    ReadProcessMemory(d->my_handle, (int*) offset, &result, sizeof(uint8_t), NULL);
}

uint16_t NormalProcess::readWord (const uint32_t offset)
{
    uint16_t result;
    ReadProcessMemory(d->my_handle, (int*) offset, &result, sizeof(uint16_t), NULL);
    return result;
}

void NormalProcess::readWord (const uint32_t offset, uint16_t &result)
{
    ReadProcessMemory(d->my_handle, (int*) offset, &result, sizeof(uint16_t), NULL);
}

uint32_t NormalProcess::readDWord (const uint32_t offset)
{
    uint32_t result;
    ReadProcessMemory(d->my_handle, (int*) offset, &result, sizeof(uint32_t), NULL);
    return result;
}

void NormalProcess::readDWord (const uint32_t offset, uint32_t &result)
{
    ReadProcessMemory(d->my_handle, (int*) offset, &result, sizeof(uint32_t), NULL);
}

void NormalProcess::read (const uint32_t offset, uint32_t size, uint8_t *target)
{
    ReadProcessMemory(d->my_handle, (int*) offset, target, size, NULL);
}

// WRITING
void NormalProcess::writeDWord (const uint32_t offset, uint32_t data)
{
    WriteProcessMemory(d->my_handle, (int*) offset, &data, sizeof(uint32_t), NULL);
}

// using these is expensive.
void NormalProcess::writeWord (uint32_t offset, uint16_t data)
{
    WriteProcessMemory(d->my_handle, (int*) offset, &data, sizeof(uint16_t), NULL);
}

void NormalProcess::writeByte (uint32_t offset, uint8_t data)
{
    WriteProcessMemory(d->my_handle, (int*) offset, &data, sizeof(uint8_t), NULL);
}

void NormalProcess::write (uint32_t offset, uint32_t size, uint8_t *source)
{
    WriteProcessMemory(d->my_handle, (int*) offset, source, size, NULL);
}



///FIXME: reduce use of temporary objects
const string NormalProcess::readCString (const uint32_t offset)
{
    string temp;
    char temp_c[256];
    DWORD read;
    ReadProcessMemory(d->my_handle, (int *) offset, temp_c, 255, &read);
    temp_c[read+1] = 0;
    temp = temp_c;
    return temp;
}

DfVector NormalProcess::readVector (uint32_t offset, uint32_t item_size)
{
    /*
        MSVC++ vector is four pointers long
        ptr allocator
        ptr start
        ptr end
        ptr alloc_end
     
        we don't care about alloc_end because we don't try to add stuff
        we also don't care about the allocator thing in front
    */
    uint32_t start = g_pProcess->readDWord(offset+4);
    uint32_t end = g_pProcess->readDWord(offset+8);
    uint32_t size = (end - start) /4;
    return DfVector(start,size,item_size);
}

size_t NormalProcess::readSTLString (uint32_t offset, char * buffer, size_t bufcapacity)
{
    /*
    MSVC++ string
    ptr allocator
    union
    {
        char[16] start;
        char * start_ptr
}
Uint32 length
Uint32 capacity
*/
    uint32_t start_offset = offset + 4;
    size_t length = g_pProcess->readDWord(offset + 20);
    
    size_t capacity = g_pProcess->readDWord(offset + 24);
    size_t read_real = min(length, bufcapacity-1);// keep space for null termination
    
    // read data from inside the string structure
    if(capacity < 16)
    {
        g_pProcess->read(start_offset, read_real , (uint8_t *)buffer);
    }
    else // read data from what the offset + 4 dword points to
    {
        start_offset = g_pProcess->readDWord(start_offset);// dereference the start offset
        g_pProcess->read(start_offset, read_real, (uint8_t *)buffer);
    }
    
    buffer[read_real] = 0;
    return read_real;
}

const string NormalProcess::readSTLString (uint32_t offset)
{
    /*
        MSVC++ string
        ptr allocator
        union
        {
            char[16] start;
            char * start_ptr
        }
        Uint32 length
        Uint32 capacity
    */
    uint32_t start_offset = offset + 4;
    uint32_t length = g_pProcess->readDWord(offset + 20);
    uint32_t capacity = g_pProcess->readDWord(offset + 24);
    char * temp = new char[capacity+1];
    
    // read data from inside the string structure
    if(capacity < 16)
    {
        g_pProcess->read(start_offset, capacity, (uint8_t *)temp);
    }
    else // read data from what the offset + 4 dword points to
    {
        start_offset = g_pProcess->readDWord(start_offset);// dereference the start offset
        g_pProcess->read(start_offset, capacity, (uint8_t *)temp);
    }
    
    temp[length] = 0;
    string ret = temp;
    delete temp;
    return ret;
}

string NormalProcess::readClassName (uint32_t vptr)
{
    int rtti = readDWord(vptr - 0x4);
    int typeinfo = readDWord(rtti + 0xC);
    string raw = readCString(typeinfo + 0xC); // skips the .?AV
    raw.resize(raw.length() - 4);// trim st@@ from end
    return raw;
}