#include "BPlusTree.h"

#include <cstring>

RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op)
{
  IndexId searchIndex;
  AttrCacheTable::getSearchIndex(relId, attrName, &searchIndex);
  AttrCatEntry attrcatentry;
  int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrcatentry);
  int block, index;
  if (searchIndex.block == -1 && searchIndex.index == -1)
  {
    block = attrcatentry.rootBlock;
    index = 0;
    if (block == -1)
      return RecId{-1, -1};
  }
  else
  {
    block = searchIndex.block;
    index = searchIndex.index + 1;
    IndLeaf leaf(block);
    HeadInfo leafHead;
    leaf.getHeader(&leafHead);
    if (index >= leafHead.numEntries)
    {
      block = leafHead.rblock;
      index = 0;
      if (block == -1)
        return RecId{-1, -1};
    }
  }
  while (StaticBuffer::getStaticBlockType(block) == IND_INTERNAL)
  {
    IndInternal internalBlk(block);
    HeadInfo intHead;
    internalBlk.getHeader(&intHead);
    InternalEntry intEntry;
    if (op == NE || op == LT || op == LE)
    {
      internalBlk.getEntry(&intEntry, 0);
      block = intEntry.lChild;
    }
    else
    {
      bool found = false;
      for (int i = 0; i < intHead.numEntries; i++)
      {
        internalBlk.getEntry(&intEntry, i);
        int cmp = compareAttrs(intEntry.attrVal, attrVal, attrcatentry.attrType);
        if (((op == EQ || op == GE) && cmp >= 0) || (op == GT && cmp > 0))
        {
          block = intEntry.lChild;
          found = true;
          break;
        }
      }
      if (!found)
      {
        internalBlk.getEntry(&intEntry, intHead.numEntries - 1);
        block = intEntry.rChild;
      }
    }
  }
  while (block != -1)
  {
    IndLeaf leafblk(block);
    HeadInfo leafHead;
    leafblk.getHeader(&leafHead);
    Index leafEntry;
    while (index < leafHead.numEntries)
    {
      leafblk.getEntry(&leafEntry, index);
      int cmp = compareAttrs(leafEntry.attrVal, attrVal, attrcatentry.attrType);
      if ((op == EQ && cmp == 0) ||
          (op == LE && cmp <= 0) ||
          (op == LT && cmp < 0) ||
          (op == NE && cmp != 0) ||
          (op == GE && cmp >= 0) ||
          (op == GT && cmp > 0))
      {
        IndexId searchIndex{block, index};
        AttrCacheTable::setSearchIndex(relId, attrName, &searchIndex);
        return RecId{leafEntry.block, leafEntry.slot};
      }
      else if ((op == EQ || op == LE || op == LT) && cmp > 0)
        return RecId{-1, -1};
      index++;
    }
    if (op != NE)
      break;
    block = leafHead.rblock;
    index = 0;
  }
  return RecId{-1, -1};
}

int BPlusTree::bPlusCreate(int relId, char attrName[ATTR_SIZE])
{
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID)
    E_NOTPERMITTED;
  AttrCatEntry attrcatentry;
  int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrcatentry);
  if (ret != SUCCESS)
    return ret;
  int rootBlockCheck = attrcatentry.rootBlock;
  if (rootBlockCheck != -1)
    return SUCCESS;

  //* create bplustree
  IndLeaf rootBlockBuf;
  int rootBlock = rootBlockBuf.getBlockNum();
  if (rootBlock = E_DISKFULL)
    return E_DISKFULL;
  RelCatEntry relcatentry;
  ret = RelCacheTable::getRelCatEntry(relId, &relcatentry);
  if (ret != SUCCESS)
    return ret;
  int block = relcatentry.firstBlk;
  while (block != -1)
  {
    RecBuffer buffer(block);
    unsigned char slotMap[relcatentry.numSlotsPerBlk];
    ret = buffer.getSlotMap(slotMap);
    if (ret != SUCCESS)
      return ret;
    for (int slot = 0; slot < relcatentry.numSlotsPerBlk; slot++)
    {
      if (slotMap[slot] == SLOT_OCCUPIED)
      {
        Attribute record[relcatentry.numAttrs];
        ret = buffer.getRecord(record, slot);
        if (ret != SUCCESS)
          return ret;
        RecId recId{block, slot};
        ret = bPlusInsert(relId, attrName, record[ATTRCAT_ATTR_NAME_INDEX], recId);
        if (ret != SUCCESS)
          return ret;
      }
    }
    struct HeadInfo head;
    int ret = buffer.getHeader(&head);
    if (ret != SUCCESS)
      return ret;
    block = head.rblock;
  }
  return SUCCESS;
}

int BPlusTree::bPlusDestroy(int rootBlockNum)
{
  if (rootBlockNum < 0 || rootBlockNum >= DISK_BLOCKS)
    return E_OUTOFBOUND;
  int type = StaticBuffer::getStaticBlockType(rootBlockNum);
  if (type == IND_LEAF)
  {
    IndLeaf leafBlock(rootBlockNum);
    leafBlock.releaseBlock();
    return SUCCESS;
  }
  else if (type == IND_INTERNAL)
  {
    IndInternal internalBlock(rootBlockNum);
    struct HeadInfo head;
    int ret = internalBlock.getHeader(&head);
    if (ret != SUCCESS)
      return SUCCESS;
    for (int i = 0; i < head.numEntries; i++)
    {
      struct InternalEntry entry;
      ret = internalBlock.getEntry(&entry, i);
      if (ret != SUCCESS)
        return ret;
      if (i == 0)
        bPlusDestroy(entry.lChild);
      bPlusDestroy(entry.rChild);
    }
    internalBlock.releaseBlock();
    return SUCCESS;
  }
  else
  {
    return E_INVALIDBLOCK;
  }
}

int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE], Attribute attrVal, RecId recId)
{
  AttrCatEntry attrcatentry;
  int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrcatentry);
  if (ret != SUCCESS)
    return ret;
  int blockNum = attrcatentry.rootBlock;
  if (blockNum == -1)
    return E_NOINDEX;
  int leafBlkNum = findLeafToInsert(blockNum, attrVal, attrcatentry.attrType);
  struct Index leafData;
  leafData.attrVal = attrVal;
  leafData.block = recId.block;
  leafData.slot = recId.slot;
  ret = insertIntoLeaf(relId, attrName, leafBlkNum, leafData);
  if (ret == E_DISKFULL)
  {
    bPlusDestroy(blockNum);
    attrcatentry.rootBlock = -1;
    AttrCacheTable::setAttrCatEntry(relId, attrName, &attrcatentry);
    return E_DISKFULL;
  }
  return SUCCESS;
}

int BPlusTree::findLeafToInsert(int rootBlock, Attribute attrVal, int attrType)
{
  int blockNum = rootBlock;
  while (StaticBuffer::getStaticBlockType(blockNum) != IND_LEAF)
  {
    IndInternal internalBlock(blockNum);
    struct HeadInfo head;
    int ret = internalBlock.getHeader(&head);
    if (ret != SUCCESS)
      return ret;
    int found = 0;
    struct InternalEntry entry;
    for (int i = 0; i < head.numEntries; i++)
    {
      internalBlock.getEntry(&entry, i);
      if (compareAttrs(entry.attrVal, attrVal, attrType) >= 0)
      {
        found = 1;
        break;
      }
    }
    if (!found)
    {
      internalBlock.getEntry(&entry, head.numEntries - 1);
      blockNum = entry.rChild;
    }
    else
      blockNum = entry.lChild;
  }
  return blockNum;
}

int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index indexEntry)
{
  AttrCatEntry attrcatentry;
  int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrcatentry);
  if (ret != SUCCESS)
    return ret;
  IndLeaf leafBlock(blockNum);
  struct HeadInfo blockHeader;
  ret = leafBlock.getHeader(&blockHeader);
  if (ret != SUCCESS)
    return ret;
  Index indices[blockHeader.numEntries + 1];

  bool inserted = false;
  int idx = 0;
  for (int i = 0; i < blockHeader.numEntries; i++)
  {
    Index entry;
    ret = leafBlock.getEntry(&entry, i);
    if (ret != SUCCESS)
      return ret;
    if (!inserted && compareAttrs(indexEntry.attrVal, entry.attrVal, attrcatentry.attrType) <= 0)
    {
      indices[idx++] = indexEntry;
      inserted = true;
    }
    indices[idx++] = entry;
  }
  if (!inserted)
    indices[idx] = indexEntry;

  if (blockHeader.numEntries != MAX_KEYS_LEAF)
  {
    blockHeader.numEntries++;
    ret = leafBlock.setHeader(&blockHeader);
    if (ret != SUCCESS)
      return ret;
    for (int i = 0; i < blockHeader.numEntries; i++)
    {
      ret = leafBlock.setEntry(&indices[i], i);
      if (ret != SUCCESS)
        return ret;
    }
    return SUCCESS;
  }

  int newRightBlk = splitLeaf(blockNum, indices);
  if (newRightBlk == E_DISKFULL)
    return E_DISKFULL;

  if (blockHeader.pblock != -1)
  {
    InternalEntry middleEntry;
    middleEntry.attrVal = indices[MIDDLE_INDEX_LEAF].attrVal;
    middleEntry.lChild = blockNum;
    middleEntry.rChild = newRightBlk;
    int ret = insertIntoInternal(relId, attrName, blockHeader.pblock, middleEntry);
    if (ret != SUCCESS)
      return ret;
  }
  else
  {
    ret = createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal, blockNum, newRightBlk);
    if (ret != SUCCESS)
      return ret;
  }
  return SUCCESS;
}
