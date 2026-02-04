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

// ? ^
// ! ...................................................................................
int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
{
  // ? reset the searchIndex of the relation catalog(RELCAT_RELID) using RelCacheTable::resetSearchIndex()
  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute newRelationName; // ? set newRelationName with newName
  strcpy(newRelationName.sVal, newName);

  // ? search the relation catalog for an entry with "RelName" = newRelationName
  RecId recId1;
  recId1 = linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, newRelationName, EQ);

  // ? If relation with name newName already exists (result of linearSearch is not {-1, -1}) return E_RELEXIST;
  if (recId1.block != -1 && recId1.slot != -1)
    return E_RELEXIST;

  // ? reset the searchIndex of the relation catalog using RelCacheTable::resetSearchIndex()
  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute oldRelationName; // ? set oldRelationName with oldName
  strcpy(oldRelationName.sVal, oldName);

  //? search the relation catalog for an entry with "RelName" = oldRelationName
  RecId recId2;
  recId2 = linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, oldRelationName, EQ);

  // ? If relation with name oldName does not exist (result of linearSearch is {-1, -1}) return E_RELNOTEXIST;
  if (recId2.block == -1 && recId2.slot == -1)
    return E_RELNOTEXIST;

  // ? get the relation catalog record of the relation to rename using a RecBuffer on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
  RecBuffer relBuffer(RELCAT_BLOCK);
  Attribute relRecord[RELCAT_NO_ATTRS];
  relBuffer.getRecord(relRecord, recId2.slot);

  // ? update the relation name attribute in the record with newName.(use RELCAT_REL_NAME_INDEX)
  strcpy(relRecord[RELCAT_REL_NAME_INDEX].sVal, newName);

  // ? set back the record value using RecBuffer.setRecord
  relBuffer.setRecord(relRecord, recId2.slot);

  // ? update all the attribute catalog entries in the attribute catalog corresponding to the relation with relation name oldName to the relation name newName

  RelCacheTable::resetSearchIndex(ATTRCAT_RELID); // ? reset the searchIndex of the attribute catalog using RelCacheTable::resetSearchIndex()

  while (true)
  {
    RecId recId3;
    recId3 = linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, oldRelationName, EQ); // ? linearSearch on the attribute catalog for relName = oldRelationName

    if (recId3.block == -1 && recId3.slot == -1)
      break;
    RecBuffer attrBuffer(recId3.block); // attribute catalog can span across many blocks
    Attribute attrRecord[ATTRCAT_NO_ATTRS];

    attrBuffer.getRecord(attrRecord, recId3.slot); // ? get the record using RecBuffer.getRecord

    strcpy(attrRecord[ATTRCAT_REL_NAME_INDEX].sVal, newName); // ? update the relName field in the record to newName

    attrBuffer.setRecord(attrRecord, recId3.slot); // ? set back the record using RecBuffer.setRecord
  }
  return SUCCESS;
}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
{
  // ? reset the searchIndex of the relation catalog(RELCAT_RELID) using RelCacheTable::resetSearchIndex()
  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute relNameAttr; // ? set relNameAttr to relName
  strcpy(relNameAttr.sVal, relName);

  RecId recId1;
  recId1 = linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr, EQ); // ? Search for the relation with name relName in relation catalog using linearSearch()

  if (recId1.block == -1 && recId1.slot == -1) // ? If relation relName does not exist (search returns {-1,-1})
    return E_RELNOTEXIST;                      // ? no such relation in relCatalog

  // ? reset the searchIndex of the attribute catalog using RelCacheTable::resetSearchIndex()
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  RecId attrToRenameRecId{-1, -1}; // ? this is how you init a list in cpp
  Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];
  // ? iterate over all Attribute Catalog Entry record corresponding to the relation to find the required attribute
  while (true)
  {
    RecId recId2;
    recId2 = linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, relNameAttr, EQ);
    // linear search on the attribute catalog for RelName = relNameAttr

    // ? if there are no more attributes left to check (linearSearch returned {-1,-1}), break;
    if (recId2.block == -1 && recId2.slot == -1)
      break;

    RecBuffer attrBuffer(recId2.block);
    attrBuffer.getRecord(attrCatEntryRecord, recId2.slot); // ? Get the record from the attribute catalog using RecBuffer.getRecord into attrCatEntryRecord

    // ? if attrCatEntryRecord.attrName = newName
    if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName) == 0)
      return E_ATTREXIST;

    // ? if attrCatEntryRecord.attrName = oldName,attrToRenameRecId = block and slot of this record
    if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldName) == 0)
      attrToRenameRecId = recId2;
  }

  if (attrToRenameRecId.block == -1 && attrToRenameRecId.slot == -1) // ? if attrToRenameRecId == {-1, -1}
    return E_ATTRNOTEXIST;

  // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
  /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
       attrToRenameRecId.slot */
  //   update the AttrName of the record with newName

  RecBuffer renameBuffer(attrToRenameRecId.block);
  renameBuffer.getRecord(attrCatEntryRecord, attrToRenameRecId.slot);
  strcpy(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName);
  renameBuffer.setRecord(attrCatEntryRecord, attrToRenameRecId.slot);

  return SUCCESS;
}
// ! ...................................................................................
