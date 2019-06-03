/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Jordan Hamilton
 * Date: June 2, 2019
 */

#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char* key)
{
	int r = 0;
	for (int i = 0; key[i] != '\0'; i++)
	{
		r += key[i];
	}
	return r;
}

int hashFunction2(const char* key)
{
	int r = 0;
	for (int i = 0; key[i] != '\0'; i++)
	{
		r += (i + 1) * key[i];
	}
	return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
	HashLink* link = malloc(sizeof(HashLink));
	link->key = malloc(sizeof(char) * (strlen(key) + 1));
	strcpy(link->key, key);
	link->value = value;
	link->next = next;
	return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink* link)
{
	free(link->key);
	free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
	map->capacity = capacity;
	map->size = 0;
	map->table = malloc(sizeof(HashLink*) * capacity);
	for (int i = 0; i < capacity; i++)
	{
		map->table[i] = NULL;
	}
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map
 */
void hashMapCleanUp(HashMap* map)
{
	assert(map != NULL);
	// Keep track of the current link and next link in each bucket
	HashLink *currentLink, *nextLink;
	// Loop through the table
	for (int i = 0; i < hashMapCapacity(map); i++) {
		// Look at the first link in each bucket, then loop through
		// and free the links until there are no additional
		// links to free in this bucket.
		currentLink = map->table[i];
		while (currentLink != NULL) {
			nextLink = currentLink->next;
			hashLinkDelete(currentLink);
			currentLink = nextLink;
		}

	}
	free(map->table);
	map->size = 0;
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
	HashMap* map = malloc(sizeof(HashMap));
	hashMapInit(map, capacity);
	return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap* map)
{
	hashMapCleanUp(map);
	free(map);
}

/**
 * Returns a pointer to the value of the link with the given key  and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 *
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 *
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
	assert(map != NULL);
	assert(key != NULL);

	int idx = HASH_FUNCTION(key) % hashMapCapacity(map);
	if (idx < 0) {
		idx += hashMapCapacity(map);
	}

	HashLink* currentLink = map->table[idx];

	while (currentLink != NULL) {
		if (strcmp(currentLink->key, key) == 0) {
			return &(currentLink->value);
		}
		currentLink = currentLink->next;
	}

	return NULL;
}

/**
 * Resizes the hash table to have a number of buckets equal to the given
 * capacity (double of the old capacity). After allocating the new table,
 * all of the links need to rehashed into it because the capacity has changed.
 *
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 *
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
	assert(map != NULL);
	assert(capacity > hashMapCapacity(map));
	HashMap* tempMap = hashMapNew(capacity);
	assert(tempMap != NULL);

	// Rehash the links from the table into a temporary table
	for (int i = 0; i < hashMapCapacity(map); i++) {
		HashLink* currentLink = map->table[i];
		while (currentLink != NULL) {
			hashMapPut(tempMap, currentLink->key, currentLink->value);
		}
	}

	// Free up memory in the table before assigning data from our temporary table
	hashMapCleanUp(map);

	map->table = tempMap->table;
	map->size = tempMap->size;
	map->capacity = tempMap->capacity;

	// Set the table pointer of our temporary table to NULL, then free memory
	// we created for the map.
	tempMap->table = NULL;
	free(tempMap);

}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 *
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 *
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
	assert(map != NULL);
	assert(key != NULL);

	int* valueToUpdate = hashMapGet(map, key);

	if (valueToUpdate == NULL) {
		// Make a new link if there was no pointer to a value returned after we
		// looked in the bucket that should have contained our given key
		int idx = HASH_FUNCTION(key) % hashMapCapacity(map);
		if (idx < 0) {
			idx += hashMapCapacity(map);
		}

		//Create a new link and put it at the front of the linked list
		HashLink* currentLink = map->table[idx];
		HashLink* newLink = hashLinkNew(key, value, currentLink);
		assert(newLink != NULL);

		map->table[idx] = newLink;

		// Increase the size of the hash table
		map->size++;

		// Check if we need to resize after adding a value
		if (hashMapTableLoad(map) >= MAX_TABLE_LOAD) {
			resizeTable(map, 2 * hashMapCapacity(map));
		}

	} else {
		// Update the value if we already found this key in a bucket
		*valueToUpdate = value;
	}

}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap* map, const char* key)
{
	assert(map != NULL);
	assert(key != NULL);

	int idx = HASH_FUNCTION(key) % hashMapCapacity(map);
	if (idx < 0) {
		idx += hashMapCapacity(map);
	}

	// Start by tracking the current link at the beginning of the bucket and
	// initialize the previous link to null, we'll assign this to be the currentLink
	// each time we compare keys so we can update its next pointer correctly
	// before calling hashLinkDelete on it.
	HashLink* currentLink = map->table[idx];
	HashLink* prevLink = NULL;

	while(currentLink != NULL) {
		if (strcmp(currentLink->key, key) == 0) {
			if (prevLink != NULL) {
				// Update the next pointer of the previous link to maintain the linked
				// list before deleting the current link.
				prevLink->next = currentLink->next;
			} else {
				// If we were looking at the first link in the bucket, make sure the
				// first link points to the next link
				map->table[idx] = currentLink->next;
			}

			// Delete the link and decrease the size of the hash map
			hashLinkDelete(currentLink);
			map->size--;
			return;
		}
		prevLink = currentLink;
		currentLink = currentLink->next;
	}

}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 *
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 *
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
	assert(map != NULL);
	assert(key != NULL);

	int idx = HASH_FUNCTION(key) % hashMapCapacity(map);
	if (idx < 0) {
		idx += hashMapCapacity(map);
	}

	HashLink* currentLink = map->table[idx];
	// Look through all links in the bucket at the determined index, returning
	// 1 if the key was found. Otherwise, return 0 after looking at all links.
	while(currentLink != NULL) {
		if (strcmp(currentLink->key, key) == 0) {
			return 1;
		}
		currentLink = currentLink->next;
	}

	return 0;
}

/**
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
	assert(map != NULL);
	return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
	assert(map != NULL);
	return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
	assert(map != NULL);
	int emptyBuckets = 0;
	for (int i = 0; i < hashMapCapacity(map); i++) {
		if (map->table[i] == NULL) {
			emptyBuckets++;
		}
	}
	return emptyBuckets;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
	assert(map != 0);
	return ((float) hashMapSize(map)) / hashMapCapacity(map);
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap* map)
{
	assert(map != NULL);
	HashLink* currentLink;
	// Loop through the table
	for (int i = 0; i < hashMapCapacity(map); i++) {
		// Look at the first link in each bucket, then loop through
		// and print the linked list until there are no additional
		// links to print in this bucket.
		currentLink = map->table[i];
		printf("\nBucket #%d of %d:\n", i + 1, hashMapCapacity(map));

		if (currentLink != NULL) {
			while(currentLink != NULL) {
				printf("Key: %s - Value: %d\n", currentLink->key, currentLink->value);
				currentLink = currentLink->next;
			}
		} else {
			printf("Empty\n");
		}

	}


}
