#include "OpenRelTable.h"
#include <cstring>
#include <stdlib.h>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];
OpenRelTable::OpenRelTable()
{

  for (int i = 0; i < MAX_OPEN; i++)
  {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    tableMetaInfo[i].free = true;
    tableMetaInfo[i].relName[0] = '\0';
  }

  RecBuffer relCatBlock(RELCAT_BLOCK);
  Attribute relCatRecord[RELCAT_NO_ATTRS];

  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCatCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCatCacheEntry.relCatEntry);
  relCatCacheEntry.recId.block = RELCAT_BLOCK;
  relCatCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCatCacheEntry;

  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

  struct RelCacheEntry attrCatCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &attrCatCacheEntry.relCatEntry);

  attrCatCacheEntry.recId.block = RELCAT_BLOCK;
  attrCatCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatCacheEntry;
  const int STUDENT_RELID = 2;

  RecBuffer relcatBuffer(RELCAT_BLOCK);
  HeadInfo relHead;
  relcatBuffer.getHeader(&relHead);

  for (int i = 0; i < relHead.numEntries; i++)
  {
    Attribute record[RELCAT_NO_ATTRS];
    relcatBuffer.getRecord(record, i);

    if (strcmp(record[RELCAT_REL_NAME_INDEX].sVal, "Students") == 0)
    {
      RelCacheEntry *entry =
          (RelCacheEntry *)malloc(sizeof(RelCacheEntry));

      RelCacheTable::recordToRelCatEntry(
          record, &entry->relCatEntry);

      entry->recId.block = RELCAT_BLOCK;
      entry->recId.slot = i;

      RelCacheTable::relCache[STUDENT_RELID] = entry;
      break;
    }
  }

  RecBuffer attrCatBlock(ATTRCAT_BLOCK);
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  AttrCacheEntry *head = nullptr;
  AttrCacheEntry *prev = nullptr;

  for (int no_relcatattrs = 0; no_relcatattrs < RELCAT_NO_ATTRS; no_relcatattrs++) // ? slots 0-5 are for relation catalog
  {
    attrCatBlock.getRecord(attrCatRecord, no_relcatattrs);
    AttrCacheEntry *entry = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &entry->attrCatEntry);

    entry->recId.block = ATTRCAT_BLOCK;
    entry->recId.slot = no_relcatattrs;
    entry->next = nullptr;
    if (head == nullptr)
      head = entry;
    else
      prev->next = entry;
    prev = entry;
  }
  AttrCacheTable::attrCache[RELCAT_RELID] = head;

  head = nullptr;
  prev = nullptr;
  for (int no_attrcatattrs = 0; no_attrcatattrs < ATTRCAT_NO_ATTRS; no_attrcatattrs++)
  {
    attrCatBlock.getRecord(attrCatRecord, no_attrcatattrs + RELCAT_NO_ATTRS);
    AttrCacheEntry *entry = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &entry->attrCatEntry);
    entry->recId.block = ATTRCAT_BLOCK;
    entry->recId.slot = no_attrcatattrs + RELCAT_NO_ATTRS;
    entry->next = nullptr;
    if (head == nullptr)
      head = entry;
    else
      prev->next = entry;
    prev = entry;
  }
  AttrCacheTable::attrCache[ATTRCAT_RELID] = head;

  AttrCacheEntry *student_head = nullptr, *student_prev = nullptr;
  int block = ATTRCAT_BLOCK;

  while (block != -1)
  {
    RecBuffer buf(block);
    HeadInfo h;
    buf.getHeader(&h);

    for (int i = 0; i < h.numEntries; i++)
    {
      Attribute record[ATTRCAT_NO_ATTRS];
      buf.getRecord(record, i);

      if (strcmp(record[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0)
      {
        AttrCacheEntry *e =
            (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

        AttrCacheTable::recordToAttrCatEntry(
            record, &e->attrCatEntry);

        e->recId.block = block;
        e->recId.slot = i;
        e->next = nullptr;

        if (!student_head)
          student_head = e;
        else
          student_prev->next = e;
        student_prev = e;
      }
    }
    block = h.rblock;
  }

  AttrCacheTable::attrCache[STUDENT_RELID] = student_head;

  tableMetaInfo[RELCAT_RELID].free = false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);

  tableMetaInfo[ATTRCAT_RELID].free = false;
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}

OpenRelTable::~OpenRelTable()
{
  for (int i = 0; i < MAX_OPEN; ++i)
  {
    if (!tableMetaInfo[i].free)
      OpenRelTable::closeRel(i);
  }
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE])
{
  for (int cacheslot = 0; cacheslot < MAX_OPEN; cacheslot++)
  {
    if (!tableMetaInfo[cacheslot].free && strcmp(tableMetaInfo[cacheslot].relName, relName) == 0)
      return cacheslot;
  }
  return E_RELNOTOPEN;
}

int OpenRelTable::getFreeOpenRelTableEntry()
{

  for (int cacheSlot = 2; cacheSlot < MAX_OPEN; cacheSlot++)
  {
    if (tableMetaInfo[cacheSlot].free)
      return cacheSlot;
  }
  return E_CACHEFULL;
}

int OpenRelTable::openRel(char relName[ATTR_SIZE])
{
  int existing = getRelId(relName);
  if (existing != E_RELNOTOPEN)
    return existing;

  int relId = getFreeOpenRelTableEntry();
  if (relId == E_CACHEFULL)
    return E_CACHEFULL;

  RecBuffer relCatBuffer(RELCAT_BLOCK);
  HeadInfo head;
  relCatBuffer.getHeader(&head);
  Attribute relcatAttribute;
  strcpy(relcatAttribute.sVal, relName);
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char *)RELCAT_ATTR_RELNAME, relcatAttribute, EQ);
  if (relcatRecId.block == -1 && relcatRecId.slot == -1)
    return E_RELNOTEXIST;
  Attribute record[RELCAT_NO_ATTRS];
  relCatBuffer.getRecord(record, relcatRecId.slot);
  RelCacheEntry *entry = (RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  RelCacheTable::recordToRelCatEntry(record, &entry->relCatEntry);
  entry->recId.block = relcatRecId.block;
  entry->recId.slot = relcatRecId.slot;
  entry->dirty = false;
  entry->searchIndex = {-1, -1};
  RelCacheTable::relCache[relId] = entry;

  AttrCacheEntry *listHead = nullptr, *prev = nullptr;
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  while (true)
  {
    RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, (char *)ATTRCAT_ATTR_RELNAME, relcatAttribute, EQ);
    if (attrcatRecId.block == -1 && attrcatRecId.slot == -1)
      break;
    RecBuffer attrbuffer(attrcatRecId.block);
    Attribute record[ATTRCAT_NO_ATTRS];
    attrbuffer.getRecord(record, attrcatRecId.slot);
    AttrCacheEntry *entry = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(record, &entry->attrCatEntry);
    entry->recId.block = attrcatRecId.block;
    entry->recId.slot = attrcatRecId.slot;
    entry->dirty = false;
    entry->searchIndex = {-1, -1};
    entry->next = nullptr;
    if (!listHead)
      listHead = entry;
    else
      prev->next = entry;
    prev = entry;
  }

  tableMetaInfo[relId].free = false;
  strcpy(tableMetaInfo[relId].relName, relName);
  AttrCacheTable::attrCache[relId] = listHead;

  return relId;
}

int OpenRelTable::closeRel(int relId)
{
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID)
    return E_NOTPERMITTED;

  if (relId < 0 || relId >= MAX_OPEN)
    return E_OUTOFBOUND;

  if (tableMetaInfo[relId].free)
    return E_RELNOTOPEN;

  if (RelCacheTable::relCache[relId]->dirty == true)
  {
    RecId recId = RelCacheTable::relCache[relId]->recId;
    union Attribute record[RELCAT_NO_ATTRS];
    RelCacheTable::relCatEntryToRecord(&RelCacheTable::relCache[relId]->relCatEntry, record);
    RecBuffer relCatBlock(recId.block);
    relCatBlock.setRecord(record, recId.slot);
  }

  tableMetaInfo[relId].free = true;
  tableMetaInfo[relId].relName[0] = '\0';

  free(RelCacheTable::relCache[relId]);
  RelCacheTable::relCache[relId] = nullptr;

  AttrCacheEntry *curr = AttrCacheTable::attrCache[relId];
  while (curr)
  {
    AttrCacheEntry *next = curr->next;
    free(curr);
    curr = next;
  }
  AttrCacheTable::attrCache[relId] = nullptr;

  return SUCCESS;
}
