#include "OpenRelTable.h"
#include <cstring>
#include <stdlib.h>

// ? BLUE ARE COMMENTS FOR STUDYING
// * GREEN ARE FOR HEADINGS
// ! RED ARE FOR SELF IMPLEMENTED CODE

// * STAGE 5 => ONLY UPDATION AND SOME ADDITION
OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN]; // ? %
OpenRelTable::OpenRelTable()
{
  // ? initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; i++)
  {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    tableMetaInfo[i].free = true;
  }
  //  ************ Setting up Relation Cache entries ************/
  // ? (we need to populate relation cache with entries for the relation catalog
  // ? and attribute catalog.)

  // **** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);
  Attribute relCatRecord[RELCAT_NO_ATTRS];

  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT); // ? put the relcatalog record in its slot for cache (that is the 0th slot)

  struct RelCacheEntry relCatCacheEntry; // ? contains a relcatentry and some metadata about it
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCatCacheEntry.relCatEntry);
  relCatCacheEntry.recId.block = RELCAT_BLOCK;             // ? for relcatentry => init the block num
  relCatCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT; // ? for relcatentry => init the slot num inside the block with block num

  // ? allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCatCacheEntry; // ? the allocated cache entry then points to our cache entry

  // **** setting up Attribute Catalog relation in the Relation Cache Table ****/
  // ! ..............................................................................................
  // ? set up the relation cache entry for the attribute catalog similarly
  // RecBuffer attrCatBlock(ATTRCAT_BLOCK);
  // Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

  struct RelCacheEntry attrCatCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &attrCatCacheEntry.relCatEntry);

  // ? from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  attrCatCacheEntry.recId.block = RELCAT_BLOCK;
  attrCatCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  // ? set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatCacheEntry;
  // ! ..............................................................................................

  // ! ..............................................................................................
  // * excercise (RELCACHE)
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
  // ! ..............................................................................................
  // ************ Setting up Attribute cache entries ************/
  // ? (we need to populate attribute cache with entries for the relation catalog
  // ? and attribute catalog.)

  // **** setting up Relation Catalog relation in the Attribute Cache Table ****/
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
  // ! ..............................................................................................

  // ! ..............................................................................................
  // *** setting up Attribute Catalog relation in the Attribute Cache Table  ***/

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

  // ! .............................................................................................

  // ! .............................................................................................
  // * exercise (ATTRCACHE)
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
  // ! .............................................................................................

  // ! .............................................................................................
  // ************ Setting up tableMetaInfo entries ************/
  // ? in the tableMetaInfo array set free = false for RELCAT_RELID and ATTRCAT_RELID set relname for RELCAT_RELID and ATTRCAT_RELID
  tableMetaInfo[RELCAT_RELID].free = false;
  tableMetaInfo[ATTRCAT_RELID].free = false;
  // ! .............................................................................................
}

// ! .............................................................................................
// * DESTRUCTOR
OpenRelTable::~OpenRelTable()
{
  // question: close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 0; i < MAX_OPEN; ++i)
  {
    if (!tableMetaInfo[i].free)
      OpenRelTable::closeRel(i); // ? we will implement this function later
  }

  // ? free all the memory that you allocated in the constructor
  // for (int i = 0; i < MAX_OPEN; i++)
  // {
  //   if (RelCacheTable::relCache[i] != nullptr)
  //   {
  //     free(RelCacheTable::relCache[i]);
  //     RelCacheTable::relCache[i] = nullptr;
  //   }
  //   AttrCacheEntry *curr = AttrCacheTable::attrCache[i];
  //   while (curr != nullptr)
  //   {
  //     AttrCacheEntry *next = curr->next;
  //     free(curr);
  //     curr = next;
  //   }
  //   AttrCacheTable::attrCache[i] = nullptr;
  // }
}
// ! .............................................................................................

// ? $
// ! .............................................................................................
/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
int OpenRelTable::getRelId(char relName[ATTR_SIZE])
{
  // ? $
  // ! ----------------------------------------------------
  // // ? if relname is RELCAT_RELNAME, return RELCAT_RELID
  // if (strcmp(relName, RELCAT_RELNAME) == 0)
  //   return RELCAT_RELID;
  // // ? if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID
  // if (strcmp(relName, ATTRCAT_RELNAME) == 0)
  //   return ATTRCAT_RELID;
  // if (strcmp(relName, "Students") == 0)
  //   return 2;
  // return E_RELNOTOPEN;
  // ! ----------------------------------------------------
  // ? %
  for (int cacheslot = 0; cacheslot < MAX_OPEN; cacheslot++)
  {
    if (!tableMetaInfo[cacheslot].free && strcmp(tableMetaInfo[cacheslot].relName, relName) == 0)
      return cacheslot;
  }
  return E_RELNOTOPEN;
  // ! ----------------------------------------------------
}
// ! .............................................................................................

// ? % new functions
// ! .............................................................................................
int OpenRelTable::getFreeOpenRelTableEntry()
{
  // ? traverse through the tableMetaInfo array, find a free entry in the Open Relation Table. If found return the relation id, else return E_CACHEFULL.
  for (int cacheSlot = 2; cacheSlot < MAX_OPEN; cacheSlot++)
  {
    if (tableMetaInfo[cacheSlot].free)
      return cacheSlot;
  }
  return E_CACHEFULL;
}
// ! .............................................................................................

// ? % =>> stage5 openRel() and closeRel() fns
// ! .............................................................................................
int OpenRelTable::openRel(char relName[ATTR_SIZE])
{
  int existing = getRelId(relName); // ? (checked using OpenRelTable::getRelId())
  if (existing != E_RELNOTOPEN)     // ? the relation `relName` already has an entry in the Open Relation Table
    return existing;                // ? return that relation id;

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
  int relId = getFreeOpenRelTableEntry(); // ? let relId be used to store the free slot.
  if (relId == E_CACHEFULL)               // ? free slot not available */
    return E_CACHEFULL;

  // ****** Setting up Relation Cache entry for the relation ******/
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

  // relcatRecId = BlockAccess::linearSearch(relId, relName, , EQ); // PAUSED
  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/

  // ? relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.

  // if (relcatRecId.block == -1 && relcatRecId.slot == -1)
  //   return E_RELNOTEXIST; // ? (the relation is not found in the Relation Catalog.)

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */

  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
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
  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  {
    /* let attrcatRecId store a valid record id an entry of the relation, relName,
    in the Attribute Catalog.*/

    /* read the record entry corresponding to attrcatRecId and create an
    Attribute Cache entry on it using RecBuffer::getRecord() and
    AttrCacheTable::recordToAttrCatEntry().
    update the recId field of this Attribute Cache entry to attrcatRecId.
    add the Attribute Cache entry to the linked list of listHead .*/
    // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
  }

  // set the relIdth entry of the AttrCacheTable to listHead.

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.

  return relId;
}

int OpenRelTable::closeRel(int relId)
{
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID)
    return E_NOTPERMITTED; // ? can't close relcat and attrcat

  if (relId < 0 || relId >= MAX_OPEN)
    return E_OUTOFBOUND; // ? 0<=relId<MAX_OPEN

  if (tableMetaInfo[relId].free)
    return E_RELNOTOPEN;

  // ? META INFO RESET
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

// ! .............................................................................................
