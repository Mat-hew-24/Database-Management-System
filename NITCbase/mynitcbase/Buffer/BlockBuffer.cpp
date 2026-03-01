#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>

BlockBuffer::BlockBuffer(char blockType)
{
}

RecBuffer::RecBuffer() : BlockBuffer('R') {}

BlockBuffer::BlockBuffer(int blockNum)
{
  this->blockNum = blockNum;
}

RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

int BlockBuffer::setHeader(struct HeadInfo *head)
{
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;
  struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;
  bufferHeader->blockType = head->blockType;
  bufferHeader->lblock = head->lblock;
  bufferHeader->rblock = head->rblock;
  bufferHeader->numAttrs = head->numAttrs;
  bufferHeader->numEntries = head->numEntries;
  bufferHeader->numSlots = head->numSlots;
  bufferHeader->pblock = head->pblock;
  int dirtyBitSetter = StaticBuffer::setDirtyBit(this->blockNum);
  if (dirtyBitSetter != SUCCESS)
    return dirtyBitSetter;
  return SUCCESS;
}

int BlockBuffer::getHeader(struct HeadInfo *head)
{
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;
  memcpy(&head->blockType, bufferPtr, 4);
  memcpy(&head->pblock, bufferPtr + 4, 4);
  memcpy(&head->lblock, bufferPtr + 8, 4);
  memcpy(&head->rblock, bufferPtr + 12, 4);
  memcpy(&head->numEntries, bufferPtr + 16, 4);
  memcpy(&head->numAttrs, bufferPtr + 20, 4);
  memcpy(&head->numSlots, bufferPtr + 24, 4);
  return SUCCESS;
}

int BlockBuffer::setBlockType(int blockType)
{
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;
  *((int32_t *)bufferPtr) = blockType;
  StaticBuffer::blockAllocMap[this->blockNum] = blockType;
  int dirtyBitChecker = StaticBuffer::setDirtyBit(this->blockNum);
  if (dirtyBitChecker != SUCCESS)
    return dirtyBitChecker;
  return SUCCESS;
}

int RecBuffer::getRecord(union Attribute *rec, int slotNum)
{
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;
  struct HeadInfo head;
  this->getHeader(&head);
  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;
  if (slotNum < 0 || slotNum >= slotCount)
    return E_OUTOFBOUND;
  int recordSize = attrCount * ATTR_SIZE;
  int offset = HEADER_SIZE + slotCount + (slotNum * recordSize);
  unsigned char *slotPointer = bufferPtr + offset;
  memcpy(rec, slotPointer, recordSize);
  return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum)
{
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;
  struct HeadInfo head;
  this->getHeader(&head);
  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;
  if (slotNum < 0 || slotNum >= slotCount)
    return E_OUTOFBOUND;
  int recordSize = attrCount * ATTR_SIZE;
  int offset = HEADER_SIZE + slotCount + (slotNum * recordSize);
  unsigned char *slotPointer = bufferPtr + offset;
  memcpy(slotPointer, rec, recordSize);
  StaticBuffer::setDirtyBit(this->blockNum);
  return SUCCESS;
}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr)
{
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
  if (bufferNum != E_BLOCKNOTINBUFFER && bufferNum != E_OUTOFBOUND)
  {
    StaticBuffer::metainfo[bufferNum].timeStamp = 0;
    for (int bufferidx = 0; bufferidx < BUFFER_CAPACITY; bufferidx++)
    {
      if (bufferidx != bufferNum && !StaticBuffer::metainfo[bufferidx].free)
        StaticBuffer::metainfo[bufferidx].timeStamp++;
    }
  }
  else
  {
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
    if (bufferNum == E_OUTOFBOUND)
      return E_OUTOFBOUND;
    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }
  *buffPtr = StaticBuffer::blocks[bufferNum];
  return SUCCESS;
}

int RecBuffer::getSlotMap(unsigned char *slotMap)
{
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;
  HeadInfo head;
  getHeader(&head);

  int slotCount = head.numSlots;
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;
  for (int slots = 0; slots < slotCount; slots++)
    slotMap[slots] = slotMapInBuffer[slots];
  return SUCCESS;
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType)
{

  double diff;
  if (attrType == STRING)
    diff = strcmp(attr1.sVal, attr2.sVal); // ? compare strings and keep difference
  else
    diff = attr1.nVal - attr2.nVal; // ? compare numbers and keep difference
  if (diff > 0)
    return 1;
  if (diff < 0)
    return -1;
  return 0;
}
