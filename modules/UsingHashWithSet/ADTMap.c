/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω Hash Table (με Set αντί για λίστες)
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "ADTSet.h"
#include "ADTMap.h"
#include <stdio.h>

int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
    786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};

#define FIXED_SIZE 3

typedef enum{
	EMPTY, OCCUPIED, DELETED
} State;

typedef struct bucket* Bucket;


// Δομή του κάθε κόμβου που έχει το hash table (με το οποίο υλοιποιούμε το map)
struct map_node{
	Pointer key;
	Pointer value;
	State state;
};

struct bucket{
	Set set;
	Pointer bucket_array[3];
	int bucket_array_size;
};


// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct map {
	Bucket array;
	int capacity;
	int size;
	CompareFunc compare;
	HashFunc hash_function;
	DestroyFunc destroy_key;
	DestroyFunc destroy_value;
};


Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value) {
	Map map = malloc(sizeof(*map));
	map->capacity = prime_sizes[0];
	map->array = malloc(map->capacity * sizeof(struct bucket));
	for(int i = 0; i < map->capacity; i++) {
		for(int i = 0; i < 3; i++)
		map->array[i].bucket_array[i] = NULL;
		//map->array[i].bucket_array[i] = NULL;
	}
	map->size = 0;
	map->compare = compare;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;
	//printf("%p\n",map->array[1].bucket_array[0]);
	return map;
}

// Επιστρέφει τον αριθμό των entries του map σε μία χρονική στιγμή.
int map_size(Map map) {
	return map->size;
}

// Εισαγωγή στο hash table του ζευγαριού (key, item). Αν το key υπάρχει,
// ανανέωση του με ένα νέο value, και η συνάρτηση επιστρέφει true.

void map_insert(Map map, Pointer key, Pointer value) {
	bool already_in = false;
	Pointer node = NULL;
	MapNode node_m = NULL;
	uint pos;
	pos = map->hash_function(key) % map->capacity;
	if(map->array[pos].bucket_array_size < FIXED_SIZE) {
	//   if(map->compare(map->array[pos].bucket_array,key) == 0) {
	// 	  already_in = true;
	// 	  node = &map->array[pos].bucket_array[0];
	// 	  node_m = node;
	//   }

	
		if(map->array[pos].bucket_array[0] == NULL) { 
			if(node == NULL) {
 				node = &map->array[pos].bucket_array[0];
				node_m = node;
				node_m->state = OCCUPIED;
				node_m->key = key;
				node_m->value = value;
				map->size++;
				map->array[pos].bucket_array_size++;

			}
		}
		else if(map->array[pos].bucket_array[0] != NULL && map->array[pos].bucket_array[1] == NULL ) {
			if(node == NULL) {
				node = &map->array[pos].bucket_array[1];
				node_m = node;
				node_m->state = OCCUPIED;
				node_m->key = key;
				node_m->value = value;
				map->size++;
				map->array[pos].bucket_array_size++;
			}
		}
		else if(map->array[pos].bucket_array[1] != NULL && map->array[pos].bucket_array[2] == NULL) {
			if(node == NULL) {
				node = &map->array[pos].bucket_array[2];
				node_m = node;
				node_m->state = OCCUPIED;
				node_m->key = key;
				node_m->value = value;
				map->array[pos].bucket_array_size++;
				map->size++;
			}
		}
		if(already_in) {
			if(node_m->key != key && map->destroy_key != NULL) {
				map->destroy_key(node_m->key);
			}
			if(node_m->value != key && map->destroy_value != NULL) {
				map->destroy_value(node_m->value);
			}
		}
	}
		printf(" [0] = %p\n",map->array[45].bucket_array[0]);
		printf(" [1] = %p\n",map->array[45].bucket_array[1]);
		printf(" [2] = %p\n",map->array[45].bucket_array[2]);
		printf(" key = %p\n",key);
		printf(" value = %p\n",value);
		printf(" m_node = %p\n",node_m);
		printf(" state = %p\n",&node_m->state);

		
}
// Διαργραφή απο το Hash Table του κλειδιού με τιμή key
bool map_remove(Map map, Pointer key) {
	return false;
}

// Αναζήτηση στο map, με σκοπό να επιστραφεί το value του κλειδιού που περνάμε σαν όρισμα.

Pointer map_find(Map map, Pointer key) {
	MapNode node = map_find_node(map, key);
	if(node != MAP_EOF)
	 	return node->value;
	else 
		return NULL;
}


DestroyFunc map_set_destroy_key(Map map, DestroyFunc destroy_key) {
	return NULL;
}

DestroyFunc map_set_destroy_value(Map map, DestroyFunc destroy_value) {
	return NULL;
}

// Απελευθέρωση μνήμης που δεσμεύει το map
void map_destroy(Map map) {

}

/////////////////////// Διάσχιση του map μέσω κόμβων ///////////////////////////

MapNode map_first(Map map) {
	return MAP_EOF;
}

MapNode map_next(Map map, MapNode node) {
	return MAP_EOF;
}

Pointer map_node_key(Map map, MapNode node) {
	return node->key;
}

Pointer map_node_value(Map map, MapNode node) {
	return node->value;
}

MapNode map_find_node(Map map, Pointer key) {
	Pointer node = NULL;
	MapNode m_node = NULL;
	uint pos = map->hash_function(key) % map->capacity;
	if(map->array[pos].bucket_array[0] != NULL) {
		if(map->compare(map->array[pos].bucket_array[0],key) == 0) {
		node = &map->array[pos].bucket_array[0];
		m_node = node;
		return m_node;
		}
	}
	else if(map->array[pos].bucket_array[1] != NULL && map->compare(map->array[pos].bucket_array[1],key) == 0) {
		node = &map->array[pos].bucket_array[1];
		m_node = node;
		return m_node;
	}
	else if(map->array[pos].bucket_array[2] != NULL && map->compare(map->array[pos].bucket_array[2],key) == 0) {
		node = &map->array[pos].bucket_array[2];
		m_node = node;
		return m_node;
	}
	return MAP_EOF;
}

// Αρχικοποίηση της συνάρτησης κατακερματισμού του συγκεκριμένου map.
void map_set_hash_function(Map map, HashFunc func) {
	map->hash_function = func;
}

uint hash_string(Pointer value) {
	// djb2 hash function, απλή, γρήγορη, και σε γενικές γραμμές αποδοτική
    uint hash = 5381;
    for (char* s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;			// hash = (hash * 33) + *s. Το foo << 5 είναι γρηγορότερη εκδοχή του foo * 32.
    return hash;
}

uint hash_int(Pointer value) {
	return *(int*)value;
}

uint hash_pointer(Pointer value) {
	return (size_t)value;				// cast σε sizt_t, που έχει το ίδιο μήκος με έναν pointer
}