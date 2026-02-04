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
  for (int bufferidx = 0; bufferidx < BUFFER_CAPACITY; bufferidx++) // ? ^
  {
    metainfo[bufferidx].free = true;
    metainfo[bufferidx].dirty = false;
    metainfo[bufferidx].blockNum = -1;
    metainfo[bufferidx].timeStamp = -1;
  }
}

// *  DESTRUCTOR
// ? ^
StaticBuffer::~StaticBuffer()
{
  for (int bufferidx = 0; bufferidx < BUFFER_CAPACITY; bufferidx++)
  {
    if (metainfo[bufferidx].free == false && metainfo[bufferidx].dirty == true)
      Disk::writeBlock(StaticBuffer::blocks[bufferidx], metainfo[bufferidx].blockNum);
  }
}

// ? ^
// *  getFreeBuffer fn for StaticBuffer Class
int StaticBuffer::getFreeBuffer(int blockNum)
{
  if (blockNum < 0 || blockNum >= DISK_BLOCKS) // ? if block does not belong in {0,1,2,...,8191} then exit with error
    return E_OUTOFBOUND;

  // ? increase the timeStamp in metaInfo of all occupied buffers.
  for (int bufferidx = 0; bufferidx < BUFFER_CAPACITY; bufferidx++)
  {
    if (!metainfo[bufferidx].free)
      metainfo[bufferidx].timeStamp++;
  }

  // ? let bufferNum be used to store the buffer number of the free/freed buffer.
  int bufferNum = -1;
  for (int bufferidx = 0; bufferidx < BUFFER_CAPACITY; bufferidx++)
  {
    if (metainfo[bufferidx].free)
    {
      bufferNum = bufferidx;
      break;
    }
  }
  if (bufferNum == -1)
  {
    int maxTime = -1;
    for (int bufferidx = 0; bufferidx < BUFFER_CAPACITY; bufferidx++)
    {
      if (metainfo[bufferidx].timeStamp > maxTime)
      {
        maxTime = metainfo[bufferidx].timeStamp;
        bufferNum = bufferidx;
      }
    }
    if (metainfo[bufferNum].dirty)
      Disk::writeBlock(blocks[bufferNum], metainfo[bufferNum].blockNum);
  }
  metainfo[bufferNum].free = false;
  metainfo[bufferNum].dirty = false;
  metainfo[bufferNum].blockNum = blockNum;
  metainfo[bufferNum].timeStamp = 0;
  return bufferNum;
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

// ? ^
// ! ..........................................
int StaticBuffer::setDirtyBit(int blockNum)
{
  int bufferNum = getBufferNum(blockNum);
  if (bufferNum == E_BLOCKNOTINBUFFER)
    return E_BLOCKNOTINBUFFER;
  else if (bufferNum == E_OUTOFBOUND)
    return E_OUTOFBOUND;
  else
    metainfo[bufferNum].dirty = true;
  return SUCCESS;
}
// ! ..........................................
