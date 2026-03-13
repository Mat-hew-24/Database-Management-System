#include "Algebra.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "../BlockAccess/BlockAccess.h"
#include "../Cache/RelCacheTable.h"
#include "../Cache/AttrCacheTable.h"
#include "../Cache/OpenRelTable.h"

#define COL_WIDTH 15

bool isNumber(char *str)
{
  int len;
  float ignore;
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

/**
 * @brief Performs a select operation on a relation based on a condition
 *
 * This function retrieves records from a source relation that satisfy a given
 * condition and displays them in a tabular format. The condition is specified
 * by an attribute name, an operation code, and a comparison value.
 *
 * @param srcRel     The name of the source relation to select from
 * @param targetRel  The name of the target relation (currently unused in implementation)
 * @param attr       The attribute name on which the condition is applied
 * @param op         The operation code for the condition (e.g., equal, greater than, etc.)
 * @param strVal     The value for condition comparison (as string)
 *
 * @note Algorithm Steps:
 * @note 1. Get the relation id for source relation\n
 * @note 2. Get the attribute catalog entry for the condition attribute
 * @note 3. Convert the string value to Attribute type based on attribute type
 * @note 4. Reset the search index for the source relation
 * @note 5. Fetch the relation catalog entry
 * @note 6. Print the attribute names as header
 * @note 7. Perform linear search through the relation for matching records
 * @note 8. For each matching record, fetch and print the record values
 * @note ------------------------------------------------------------
 * @note The function prints results to stdout in a table format with columns
 *       separated by '|' characters
 */

int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE])
{
  if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0)
    return E_NOTPERMITTED;
  int relId = OpenRelTable::getRelId(relName);
  if (relId == E_RELNOTOPEN)
    return E_RELNOTOPEN;
  RelCatEntry Relentry;
  int ret = RelCacheTable::getRelCatEntry(relId, &Relentry);
  if (ret != SUCCESS)
    return ret;
  if (nAttrs != Relentry.numAttrs)
    return E_NATTRMISMATCH;
  union Attribute recordValues[nAttrs];
  for (int i = 0; i < nAttrs; i++)
  {
    AttrCatEntry Attrentry;
    ret = AttrCacheTable::getAttrCatEntry(relId, i, &Attrentry);
    if (ret != SUCCESS)
      return ret;
    int type = Attrentry.attrType;
    if (type == NUMBER)
    {
      if (isNumber(record[i]))
        recordValues[i].nVal = atof(record[i]);
      else
        return E_ATTRTYPEMISMATCH;
    }
    else if (type == STRING)
      strcpy(recordValues[i].sVal, record[i]);
  }
  return BlockAccess::insert(relId, recordValues);
}

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
{

  int srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN)
    return E_RELNOTOPEN;

  AttrCatEntry condAttr;
  if (AttrCacheTable::getAttrCatEntry(srcRelId, attr, &condAttr) != SUCCESS)
    return E_ATTRNOTEXIST;

  Attribute attrVal;
  int type = condAttr.attrType;
  if (type == NUMBER)
  {
    if (!isNumber(strVal))
      return E_ATTRTYPEMISMATCH;
    attrVal.nVal = atof(strVal);
  }
  else if (type == STRING)
    strcpy(attrVal.sVal, strVal);

  RelCacheTable::resetSearchIndex(srcRelId);
  RelCatEntry relCat;
  if (RelCacheTable::getRelCatEntry(srcRelId, &relCat) != SUCCESS)
    return E_RELNOTOPEN;
  int src_nAttrs = relCat.numAttrs;
  char attr_names[src_nAttrs][ATTR_SIZE];
  int attr_types[src_nAttrs];

  for (int i = 0; i < src_nAttrs; i++)
  {
    AttrCatEntry attrEntry;
    if (AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrEntry) != SUCCESS)
      return E_ATTRNOTEXIST;
    strcpy(attr_names[i], attrEntry.attrName);
    attr_types[i] = attrEntry.attrType;
  }

  int ret = Schema::createRel(targetRel, src_nAttrs, attr_names, attr_types);
  if (ret != SUCCESS)
    return ret;
  int targetRelId = OpenRelTable::openRel(targetRel);
  if (targetRelId < 0)
  {
    Schema::deleteRel(targetRel);
    return targetRelId;
  }

  printf("\n|");
  for (int i = 0; i < src_nAttrs; i++)
  {
    for (int j = 0; j < COL_WIDTH + 2; j++)
      printf("-");
    printf("|");
  }
  printf("\n|");

  printf("|");
  for (int i = 0; i < src_nAttrs; i++)
    printf(" %-*s |", COL_WIDTH, attr_names[i]);
  printf("\n");

  printf("|");
  for (int i = 0; i < src_nAttrs; i++)
  {
    for (int j = 0; j < COL_WIDTH + 2; j++)
      printf("-");
    printf("|");
  }
  printf("\n");

  RelCacheTable::resetSearchIndex(srcRelId);
  Attribute record[src_nAttrs];

  while (BlockAccess::search(srcRelId, record, attr, attrVal, op) == SUCCESS)
  {
    printf("|");
    for (int i = 0; i < src_nAttrs; i++)
    {
      if (attr_types[i] == NUMBER)
        printf(" %-*g |", COL_WIDTH, record[i].nVal);
      else
        printf(" %-*s |", COL_WIDTH, record[i].sVal);
    }
    printf("\n");

    ret = BlockAccess::insert(targetRelId, record);
    if (ret != SUCCESS)
    {
      Schema::closeRel(targetRel);
      Schema::deleteRel(targetRel);
      return ret;
    }
  }
  printf("|");
  for (int i = 0; i < src_nAttrs; i++)
  {
    for (int j = 0; j < COL_WIDTH + 2; j++)
      printf("-");
    printf("|");
  }
  printf("\n\n");

  Schema::closeRel(targetRel);
  return SUCCESS;
}

// project all attributes
int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE])
{
  int srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN)
    return E_RELNOTOPEN;
  RelCatEntry relCatBuf;
  int ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatBuf);
  if (ret != SUCCESS)
    return ret;
  int src_nAttrs = relCatBuf.numAttrs;
  char attrNames[src_nAttrs][ATTR_SIZE];
  int attrTypes[src_nAttrs];

  for (int i = 0; i < src_nAttrs; i++)
  {
    AttrCatEntry attrCatBuf;
    ret = AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatBuf);
    if (ret != SUCCESS)
      return ret;
    strcpy(attrNames[i], attrCatBuf.attrName);
    attrTypes[i] = attrCatBuf.attrType;
  }
  ret = Schema::createRel(targetRel, src_nAttrs, attrNames, attrTypes);
  if (ret != SUCCESS)
    return ret;
  int targetRelId = OpenRelTable::openRel(targetRel);
  if (targetRelId < 0)
  {
    Schema::deleteRel(targetRel);
    return targetRelId;
  }

  RelCacheTable::resetSearchIndex(srcRelId);
  Attribute record[src_nAttrs];

  while (BlockAccess::project(srcRelId, record) == SUCCESS)
  {
    ret = BlockAccess::insert(targetRelId, record);
    if (ret != SUCCESS)
    {
      Schema::closeRel(targetRel);
      Schema::deleteRel(targetRel);
      return ret;
    }
  }
  Schema::closeRel(targetRel);
  return SUCCESS;
}

// project specified attributes

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE])
{
  int srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN)
    return E_RELNOTOPEN;
  RelCatEntry relCatBuf;
  int ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatBuf);
  if (ret != SUCCESS)
    return ret;
  int src_nAttrs = relCatBuf.numAttrs;
  int attr_offset[tar_nAttrs];
  int attr_types[tar_nAttrs];
  for (int i = 0; i < tar_nAttrs; i++)
  {
    AttrCatEntry attrCatBuf;
    ret = AttrCacheTable::getAttrCatEntry(srcRelId, tar_Attrs[i], &attrCatBuf);
    if (ret != SUCCESS)
      return ret;
    attr_offset[i] = attrCatBuf.offset;
    attr_types[i] = attrCatBuf.attrType;
  }
  ret = Schema::createRel(targetRel, tar_nAttrs, tar_Attrs, attr_types);
  if (ret != SUCCESS)
    return ret;
  int targetRelId = OpenRelTable::openRel(targetRel);
  if (targetRelId < 0)
  {
    Schema::deleteRel(targetRel);
    return targetRelId;
  }

  RelCacheTable::resetSearchIndex(srcRelId);
  Attribute record[src_nAttrs];

  while (BlockAccess::project(srcRelId, record) == SUCCESS)
  {
    Attribute project_record[tar_nAttrs];
    for (int i = 0; i < tar_nAttrs; i++)
      project_record[i] = record[attr_offset[i]];
    ret = BlockAccess::insert(targetRelId, project_record);
    if (ret != SUCCESS)
    {
      Schema::closeRel(targetRel);
      Schema::deleteRel(targetRel);
      return ret;
    }
  }
  Schema::closeRel(targetRel);
  return SUCCESS;
}
