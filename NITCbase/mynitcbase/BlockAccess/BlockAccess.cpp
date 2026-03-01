#include "BlockAccess.h"
#include <cstring>

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)
{
  RecId prevRecId;
  RelCacheTable::getSearchIndex(relId, &prevRecId);

  int block, slot;

  if (prevRecId.block == -1 && prevRecId.slot == -1)
  {
    RelCatEntry relCat;
    if (RelCacheTable::getRelCatEntry(relId, &relCat) != SUCCESS)
      return RecId{-1, -1};
    block = relCat.firstBlk;
    slot = 0;
  }
  else
  {
    block = prevRecId.block;
    slot = prevRecId.slot + 1;
  }
  AttrCatEntry attrCat;
  if (AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCat) != SUCCESS)
    return RecId{-1, -1};

  while (block != -1)
  {
    RecBuffer buffer(block);
    HeadInfo head;
    buffer.getHeader(&head);

    unsigned char slotMap[head.numSlots];
    buffer.getSlotMap(slotMap);

    if (slot >= head.numSlots)
    {
      block = head.rblock;
      slot = 0;
      continue;
    }

    while (slot < head.numSlots)
    {
      if (slotMap[slot] == SLOT_UNOCCUPIED)
      {
        slot++;
        continue;
      }

      Attribute RECORD[head.numAttrs];
      buffer.getRecord(RECORD, slot);

      int cmpVal = compareAttrs(RECORD[attrCat.offset], attrVal, attrCat.attrType);

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
        RelCacheTable::setSearchIndex(relId, &point);
        return point;
      }

      slot++; // ? move to next slot
    }
    block = head.rblock;
    slot = 0;
  }
  return RecId{-1, -1};
}

int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
{
  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute newRelationName;
  strcpy(newRelationName.sVal, newName);

  RecId recId1;
  recId1 = linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, newRelationName, EQ);

  if (recId1.block != -1 && recId1.slot != -1)
    return E_RELEXIST;

  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute oldRelationName; // ? set oldRelationName with oldName
  strcpy(oldRelationName.sVal, oldName);

  RecId recId2;
  recId2 = linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, oldRelationName, EQ);

  if (recId2.block == -1 && recId2.slot == -1)
    return E_RELNOTEXIST;

  RecBuffer relBuffer(RELCAT_BLOCK);
  Attribute relRecord[RELCAT_NO_ATTRS];
  relBuffer.getRecord(relRecord, recId2.slot);

  strcpy(relRecord[RELCAT_REL_NAME_INDEX].sVal, newName);
  relBuffer.setRecord(relRecord, recId2.slot);
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

  while (true)
  {
    RecId recId3;
    recId3 = linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, oldRelationName, EQ);

    if (recId3.block == -1 && recId3.slot == -1)
      break;
    RecBuffer attrBuffer(recId3.block);
    Attribute attrRecord[ATTRCAT_NO_ATTRS];
    attrBuffer.getRecord(attrRecord, recId3.slot);
    strcpy(attrRecord[ATTRCAT_REL_NAME_INDEX].sVal, newName);
    attrBuffer.setRecord(attrRecord, recId3.slot);
  }
  return SUCCESS;
}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
{
  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute relNameAttr; // ? set relNameAttr to relName
  strcpy(relNameAttr.sVal, relName);

  RecId recId1;
  recId1 = linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr, EQ);

  if (recId1.block == -1 && recId1.slot == -1)
    return E_RELNOTEXIST;

  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  RecId attrToRenameRecId{-1, -1};
  Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

  while (true)
  {
    RecId recId2;
    recId2 = linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, relNameAttr, EQ);
    if (recId2.block == -1 && recId2.slot == -1)
      break;
    RecBuffer attrBuffer(recId2.block);
    attrBuffer.getRecord(attrCatEntryRecord, recId2.slot);
    if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName) == 0)
      return E_ATTREXIST;
    if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldName) == 0)
      attrToRenameRecId = recId2;
  }

  if (attrToRenameRecId.block == -1 && attrToRenameRecId.slot == -1)
    return E_ATTRNOTEXIST;

  RecBuffer renameBuffer(attrToRenameRecId.block);
  renameBuffer.getRecord(attrCatEntryRecord, attrToRenameRecId.slot);
  strcpy(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName);
  renameBuffer.setRecord(attrCatEntryRecord, attrToRenameRecId.slot);

  return SUCCESS;
}

int BlockAccess::insert(int relId, Attribute *record)
{
  RelCatEntry relEntry;
  int ret = RelCacheTable::getRelCatEntry(relId, &relEntry);
  if (ret != SUCCESS)
    return ret;

  RecId recId = {-1, -1};
  int prevBlockNum = -1;

  int blockNum = relEntry.firstBlk;
  int numOfSlots = relEntry.numSlotsPerBlk;
  int numOfAttributes = relEntry.numAttrs;

  while (blockNum != -1)
  {
    RecBuffer buffer(blockNum);
    struct HeadInfo head1;
    ret = buffer.getHeader(&head1);
    if (ret != SUCCESS)
      return ret;
    unsigned char *slotMap;
    ret = buffer.getSlotMap(slotMap);
    if (ret != SUCCESS)
      return ret;
    for (int i = 0; i < numOfSlots; i++)
    {
      if (slotMap[i] == SLOT_UNOCCUPIED)
      {
        recId = {blockNum, i};
        break;
      }
    }
    if (recId.block != -1)
      break;
    prevBlockNum = blockNum;
    blockNum = head1.rblock;
  }
  if (recId.block == -1)
  {
    if (relId == RELCAT_RELID)
      return E_MAXRELATIONS;
    RecBuffer newBlock(REC);
    int newBlockNum = newBlock.getBlockNum();
    if (newBlockNum == E_DISKFULL)
      return E_DISKFULL;
    recId = {newBlockNum, 0};
    struct HeadInfo head2;
    head2.blockType = REC;
    head2.pblock = -1;
    head2.rblock = -1;
    head2.lblock = prevBlockNum;
    head2.numAttrs = numOfAttributes;
    head2.numSlots = numOfSlots;
    head2.numEntries = 0;
    newBlock.setHeader(&head2);

    unsigned char newSlotMap[numOfSlots];
    for (int i = 0; i < numOfSlots; i++)
      newSlotMap[i] = SLOT_UNOCCUPIED;
    newBlock.setSlotMap(newSlotMap);

    if (prevBlockNum != -1)
    {
      RecBuffer prev(prevBlockNum);
      struct HeadInfo head3;
      ret = prev.getHeader(&head3);
      if (ret != SUCCESS)
        return ret;
      head3.rblock = recId.block;
      prev.setHeader(&head3);
    }
    else
      relEntry.firstBlk = newBlockNum;
    relEntry.lastBlk = newBlockNum;
    ret = RelCacheTable::setRelCatEntry(relId, &relEntry);
    if (ret != SUCCESS)
      return ret;
  }
  RecBuffer target(recId.block);
  ret = target.setRecord(record, recId.slot);
  if (ret != SUCCESS)
    return ret;
  unsigned char targetSlotMap[numOfSlots];
  ret = target.getSlotMap(targetSlotMap);
  if (ret != SUCCESS)
    return ret;
  targetSlotMap[recId.slot] = SLOT_OCCUPIED;
  ret = target.setSlotMap(targetSlotMap);
  if (ret != SUCCESS)
    return ret;

  struct HeadInfo head4;
  ret = target.getHeader(&head4);
  if (ret != SUCCESS)
    return ret;
  head4.numEntries++;
  ret = target.setHeader(&head4);
  if (ret != SUCCESS)
    return ret;

  relEntry.numRecs++;
  ret = RelCacheTable::setRelCatEntry(relId, &relEntry);
  if (ret != SUCCESS)
    return ret;
}
