#include "BlockAccess.h"
#include <cstring>

// ? $
// ! ...................................................................................
RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)
{
  RecId prevRecId;                                  // ? To store previous recId
  RelCacheTable::getSearchIndex(relId, &prevRecId); // ? get the previous search index of the relation relId from the relation cache

  // let block and slot denote the record id of the record being currently checked
  int block, slot;

  // if the current search index record is invalid(i.e. both block and slot = -1)
  if (prevRecId.block == -1 && prevRecId.slot == -1)
  {
    RelCatEntry relCat;
    // ? get the first record block of the relation from the relation cache
    if (RelCacheTable::getRelCatEntry(relId, &relCat) != SUCCESS)
      return RecId{-1, -1}; // ? no hits from previous search

    block = relCat.firstBlk;
    slot = 0;
  }
  else
  {
    // ? there is a hit from previous search; search should start from the next record
    block = prevRecId.block;
    slot = prevRecId.slot + 1; // ? take the next slot number linearly
  }

  /* The following code searches for the next record in the relation
     that satisfies the given condition
     We start from the record id (block, slot) and iterate over the remaining
     records of the relation
  */

  AttrCatEntry attrCat;
  // ? get attribute catalog entry for attrName
  if (AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCat) != SUCCESS)
    return RecId{-1, -1};

  while (block != -1)
  {
    /* create a RecBuffer object for block (use RecBuffer Constructor for existing block) */
    RecBuffer buffer(block);

    // ? get header of the block
    HeadInfo head;
    buffer.getHeader(&head);

    // ? get slot map of the block
    unsigned char slotMap[head.numSlots];
    buffer.getSlotMap(slotMap);

    // ? If slot >= the number of slots per block(i.e. no more slots in this block)
    if (slot >= head.numSlots)
    {
      block = head.rblock; // ? update block = right block of block
      slot = 0;            // ? update slot = 0
      continue;            // ? continue to the beginning of this while loop
    }

    while (slot < head.numSlots)
    {
      // ? if slot is free skip the loop
      if (slotMap[slot] == SLOT_UNOCCUPIED)
      {
        slot++;   // ? increment slot
        continue; // ? continue to the next record slot
      }

      // ? get the record with id (block, slot)
      Attribute RECORD[head.numAttrs];
      buffer.getRecord(RECORD, slot);

      // ? compare record's attribute value with given attrVal
      int cmpVal = compareAttrs(RECORD[attrCat.offset], attrVal, attrCat.attrType);

      /* Next task is to check whether this record satisfies the given condition.
         It is determined based on the output of previous comparison and
         the op value received.
      */

      if (
          (op == NE && cmpVal != 0) || // ? not equal to
          (op == LT && cmpVal < 0) ||  // ? less than
          (op == LE && cmpVal <= 0) || // ? less than or equal to
          (op == EQ && cmpVal == 0) || // ? equal to
          (op == GT && cmpVal > 0) ||  // ? greater than
          (op == GE && cmpVal >= 0)    // ? greater than or equal to
      )
      {
        RecId point = {block, slot};

        // ? set the search index in the relation cache
        RelCacheTable::setSearchIndex(relId, &point);

        return point;
      }

      slot++; // ? move to next slot
    }

    // ? move to next block after finishing this block
    block = head.rblock;
    slot = 0;
  }

  // ? no record in the relation with Id relid satisfies the given condition
  return RecId{-1, -1};
}
// ! ...................................................................................
