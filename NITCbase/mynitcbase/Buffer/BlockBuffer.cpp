#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum)
{
  this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head)
{
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;

  // populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->blockType, bufferPtr, 4);
  memcpy(&head->pblock, bufferPtr + 4, 4);
  memcpy(&head->lblock, bufferPtr + 8, 4);
  memcpy(&head->rblock, bufferPtr + 12, 4);
  memcpy(&head->numEntries, bufferPtr + 16, 4);
  memcpy(&head->numAttrs, bufferPtr + 20, 4);
  memcpy(&head->numSlots, bufferPtr + 24, 4);

  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) // copies disk → memory
{
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;

  struct HeadInfo head;
  this->getHeader(&head); // get the header using this.getHeader() function

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
  int recordSize = attrCount * ATTR_SIZE;
  int offset = HEADER_SIZE + slotCount + (slotNum * recordSize);
  unsigned char *slotPointer = bufferPtr + offset;

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) // copies memory → disk
{
  struct HeadInfo head;
  unsigned char buffer[BLOCK_SIZE];

  this->getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  Disk::readBlock(buffer, this->blockNum);

  int recordSize = attrCount * ATTR_SIZE;
  int offset = HEADER_SIZE + slotCount + (slotNum * recordSize);
  unsigned char *slotPointer = buffer + offset;

  memcpy(slotPointer, rec, recordSize);
  Disk::writeBlock(buffer, this->blockNum);

  return SUCCESS;
}

// stage-3 part
//  LOAD A BLOCK AND GET A BUFFERPOINTER,
// STATICBUFFER FNS => getBufferNum,getFreeBuffer,blocks
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr)
{
  // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
  if (bufferNum == E_BLOCKNOTINBUFFER)
  {
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
    if (bufferNum == E_OUTOFBOUND)
      return E_OUTOFBOUND;
    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }
  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];
  return SUCCESS;
}

// ? $
// ?  read the slotmap so that we can iterate through all the valid records of the relation
/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
// ! ...........................................................................................

int RecBuffer::getSlotMap(unsigned char *slotMap)
{
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;

  HeadInfo head;
  getHeader(&head); // ? get the header of the block using getHeader() function

  int slotCount = head.numSlots; // ? number of slots (metadata obtained from the block-header)
  // ? get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  for (int slots = 0; slots < slotCount; slots++)
    slotMap[slots] = slotMapInBuffer[slots];
  return SUCCESS;
}
// ! ...........................................................................................

// ! ...........................................................................................
// ? $
// ? compareAttrs function compares two union Attribute values on the basis of the input attribute type.
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
// ! ...........................................................................................
