#include "StaticBuffer.h"

// ? BLUE ARE COMMENTS FOR STUDYING
// * GREEN ARE FOR HEADINGS
// ! RED ARE FOR SELF IMPLEMENTED CODE

// ? inside the same class methods u dont have to use CLASS::METHOD

// ? init blocks => BUFFERCAPACITY is 32blocks and nitcbase has 2048 bytes in each block
unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY]; // ? extra info to track status

// * CONSTRUCTOR
// ? => just set all blocks in buffer as free
StaticBuffer::StaticBuffer()
{
  for (int bufferidx = 0; bufferidx < BUFFER_CAPACITY; bufferidx++)
    metainfo[bufferidx].free = true;
}

// *  DESTRUCTOR
StaticBuffer::~StaticBuffer() {} // ? NOT IMPLEMENTED NOW AS WRITEBACK NOT IMPLEMENTED YET

// *  getFreeBuffer fn for StaticBuffer Class
int StaticBuffer::getFreeBuffer(int blockNum)
{
  if (blockNum < 0 || blockNum >= DISK_BLOCKS) // ? if block does not belong in {0,1,2,...,8191} then exit with error
    return E_OUTOFBOUND;
  // ! ............................................................
  // ? iterate through all the blocks in the StaticBuffer
  // ? find the first free block in the buffer (check metainfo)
  // ? assign allocatedBuffer = index of the free block
  int allocatedBuffer;
  for (int bufferidx = 0; bufferidx < BUFFER_CAPACITY; bufferidx++)
  {
    if (metainfo[bufferidx].free)
    {
      allocatedBuffer = bufferidx;
      break;
    }
  }
  // ! .............................................................
  metainfo[allocatedBuffer].free = false;
  metainfo[allocatedBuffer].blockNum = blockNum;

  return allocatedBuffer;
}

// ? Get the buffer index where a particular block is stored or E_BLOCKNOTINBUFFER otherwise
// * getBufferNum in static buffer class
// ! ................................................................
int StaticBuffer::getBufferNum(int blockNum)
{
  if (blockNum < 0 || blockNum >= DISK_BLOCKS) // ? 0<=blockNum<=8191
    return E_OUTOFBOUND;
  //? find and return the bufferIndex which corresponds to blockNum (check metainfo), else exit code
  for (int bufferidx = 0; bufferidx < BUFFER_CAPACITY; bufferidx++)
  {
    if (metainfo[bufferidx].free == false && metainfo[bufferidx].blockNum == blockNum) // ? skip free slots, and check if exists,then blocknums match
      return bufferidx;
  }
  return E_BLOCKNOTINBUFFER;
}
// ! ................................................................
