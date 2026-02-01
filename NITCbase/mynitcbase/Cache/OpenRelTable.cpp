#include "OpenRelTable.h"
#include <cstring>
#include <stdlib.h>

// ? BLUE ARE COMMENTS FOR STUDYING
// * GREEN ARE FOR HEADINGS
// ! RED ARE FOR SELF IMPLEMENTED CODE

OpenRelTable::OpenRelTable()
{
  // ? initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; i++)
  {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
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
}

// ! .............................................................................................
// * DESTRUCTOR
OpenRelTable::~OpenRelTable()
{
  // ? free all the memory that you allocated in the constructor
  for (int i = 0; i < MAX_OPEN; i++)
  {
    if (RelCacheTable::relCache[i] != nullptr)
    {
      free(RelCacheTable::relCache[i]);
      RelCacheTable::relCache[i] = nullptr;
    }
    AttrCacheEntry *curr = AttrCacheTable::attrCache[i];
    while (curr != nullptr)
    {
      AttrCacheEntry *next = curr->next;
      free(curr);
      curr = next;
    }
    AttrCacheTable::attrCache[i] = nullptr;
  }
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
  // ? if relname is RELCAT_RELNAME, return RELCAT_RELID
  if (strcmp(relName, RELCAT_RELNAME) == 0)
    return RELCAT_RELID;
  // ? if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID
  if (strcmp(relName, ATTRCAT_RELNAME) == 0)
    return ATTRCAT_RELID;
  return E_RELNOTOPEN;
}
// ! .............................................................................................
