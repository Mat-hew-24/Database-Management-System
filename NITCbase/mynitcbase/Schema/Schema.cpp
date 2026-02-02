#include "Schema.h"
#include <cmath>
#include <cstring>

// ? %
// ! ...........................................................
int Schema::openRel(char relName[ATTR_SIZE]) // ? the OpenRelTable::openRel() function returns the rel-id if successful a valid rel-id will be within the range 0 <= relId < MAX_OPEN and any error codes will be negative
{
  int ret = OpenRelTable::openRel(relName);
  if (ret >= 0)
    return SUCCESS;
  return ret; // ? otherwise it returns an error message
}
// ! ...........................................................

// ! ...........................................................
int Schema::closeRel(char relName[ATTR_SIZE])
// ? this function returns the rel-id of a relation if it is open or E_RELNOTOPEN if it is not. we will implement this later.
{
  if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0) // ? relation is relation catalog or attribute catalog
    return E_NOTPERMITTED;
  int relId = OpenRelTable::getRelId(relName);
  RelCatEntry relCatEntry;
  if (RelCacheTable::getRelCatEntry(relId, &relCatEntry) != SUCCESS) // ? relation is not open
    return E_RELNOTOPEN;
  return OpenRelTable::closeRel(relId);
}
// ! ...........................................................
