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
  /*

  sscanf returns the number of elements read, so if there is no float matching
  the first %f, ret will be 0, else it'll be 1
  %n gets the number of characters read. this scanf sequence will read the
  first float ignoring all the whitespace before and after. and the number of
  characters read that far will be stored in len. if len == strlen(str), then
  the string only contains a float with/without whitespace. else, there's other
  characters.
  */
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
int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
{

  int srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN)
    return E_RELNOTOPEN;

  // get attribute catalog entry for condition attribute
  AttrCatEntry condAttr;
  if (AttrCacheTable::getAttrCatEntry(srcRelId, attr, &condAttr) != SUCCESS)
    return E_ATTRNOTEXIST;

  // convert string value to Attribute
  Attribute attrVal;
  if (condAttr.attrType == NUMBER)
  {
    if (!isNumber(strVal))
      return E_ATTRTYPEMISMATCH;
    attrVal.nVal = atof(strVal);
  }
  else
    strcpy(attrVal.sVal, strVal);

  RelCacheTable::resetSearchIndex(srcRelId);

  RelCatEntry relCat;
  if (RelCacheTable::getRelCatEntry(srcRelId, &relCat) != SUCCESS)
    return E_RELNOTOPEN;

  printf("\n");

  printf("|");
  for (int i = 0; i < relCat.numAttrs; i++)
  {
    for (int j = 0; j < COL_WIDTH + 2; j++)
      printf("-");
    printf("|");
  }
  printf("\n");

  printf("|");
  for (int i = 0; i < relCat.numAttrs; i++)
  {
    AttrCatEntry attrEntry;
    if (AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrEntry) != SUCCESS)
      return E_ATTRNOTEXIST;
    printf(" %-*s |", COL_WIDTH, attrEntry.attrName);
  }
  printf("\n");

  printf("|");
  for (int i = 0; i < relCat.numAttrs; i++)
  {
    for (int j = 0; j < COL_WIDTH + 2; j++)
      printf("-");
    printf("|");
  }
  printf("\n");

  /* fetch and print matching records */
  while (true)
  {
    RecId recId = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);

    if (recId.block == -1 && recId.slot == -1)
      break; // no more records

    RecBuffer rb(recId.block);

    Attribute record[relCat.numAttrs];
    rb.getRecord(record, recId.slot);

    printf("|");
    for (int i = 0; i < relCat.numAttrs; i++)
    {
      AttrCatEntry attrEntry;
      if (AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrEntry) != SUCCESS)
        return E_ATTRNOTEXIST;
      // printf("%d == %d\n",attrEntry.attrType,NUMBER);
      if (attrEntry.attrType == NUMBER)
        printf(" %-*g |", COL_WIDTH, record[i].nVal);
      else
        printf(" %-*s |", COL_WIDTH, record[i].sVal);
    }
    printf("\n");
  }
  printf("|");
  for (int i = 0; i < relCat.numAttrs; i++)
  {
    for (int j = 0; j < COL_WIDTH + 2; j++)
      printf("-");
    printf("|");
  }
  printf("\n\n");

  return SUCCESS;
}

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
