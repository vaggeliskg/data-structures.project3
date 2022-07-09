/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω Cuckoo hashing
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "ADTMap.h"


typedef enum {
	EMPTY, OCCUPIED, DELETED
} State;

int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};


// Δομή του κάθε κόμβου που έχει το hash table (με το οποίο υλοιποιούμε το map)
struct map_node{
	Pointer key;		// Το κλειδί που χρησιμοποιείται για να hash-αρουμε
	Pointer value;  	// Η τιμή που αντισοιχίζεται στο παραπάνω κλειδί
	State state;	
};

// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct map {
	MapNode array1;
 	int capacity;				
	MapNode array2;				
 	int size;							
 	CompareFunc compare;		
 	HashFunc hash_function;
 	DestroyFunc destroy_key;	
 	DestroyFunc destroy_value;
};


Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value) {
	// Δεσμεύουμε κατάλληλα τον χώρο που χρειαζόμαστε για το hash table
	Map map = malloc(sizeof(*map));
	map->capacity = prime_sizes[0];
	map->capacity = prime_sizes[0];
	map->array1 = malloc(map->capacity * sizeof(struct map_node));
	map->array2 = malloc(map->capacity * sizeof(struct map_node));
	// Αρχικοποιούμε τους κόμβους που έχουμε σαν διαθέσιμους.
	for (int i = 0; i < map->capacity; i++)
		map->array1[i].state = EMPTY;

	for(int i = 0; i < map->capacity; i++)
		map->array2[i].state = EMPTY;
	
	map->size = 0;
	map->compare = compare;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;
	

	return map;
}

// Επιστρέφει τον αριθμό των entries του map σε μία χρονική στιγμή.
int map_size(Map map) {
	return map->size;
}

// Εισαγωγή στο hash table του ζευγαριού (key, item). Αν το key υπάρχει,
// ανανέωση του με ένα νέο value, και η συνάρτηση επιστρέφει true.

void map_insert(Map map, Pointer key, Pointer value) {
	// Pointer temp_key;
	// Pointer temp_value;

	uint pos1 = map->hash_function(key) % map->capacity;
	//uint pos2 = (map->hash_function(key)^2 / map->capacity) % map->capacity;

	// State current_state1 = map->array1[pos1].state;
	// State current_state2 = map->array2[pos2].state;

	if(map->array1[pos1].state == EMPTY) {
		map->array1[pos1].key = key;
		map->array1[pos1].value = value;
		map->array1[pos1].state = OCCUPIED;
		map->size++;
	}

}

// Διαργραφή απο το Hash Table του κλειδιού με τιμή key
bool map_remove(Map map, Pointer key) {
	MapNode node = map_find_node(map, key);
	if (node == MAP_EOF)
		return false;

	// destroy
	if (map->destroy_key != NULL)
		map->destroy_key(node->key);
	if (map->destroy_value != NULL)
		map->destroy_value(node->value);

	// θέτουμε ως "deleted", ώστε να μην διακόπτεται η αναζήτηση, αλλά ταυτόχρονα να γίνεται ομαλά η εισαγωγή
	node->state = DELETED;
	map->size--;

	return true;
}

// Αναζήτηση στο map, με σκοπό να επιστραφεί το value του κλειδιού που περνάμε σαν όρισμα.

Pointer map_find(Map map, Pointer key) {
	MapNode node = map_find_node(map,key);
	if(node != MAP_EOF)
		return node->value;
	else 
		return NULL;
}


DestroyFunc map_set_destroy_key(Map map, DestroyFunc destroy_key) {
	DestroyFunc old = map->destroy_key;
	map->destroy_key = destroy_key;
	return old;
}

DestroyFunc map_set_destroy_value(Map map, DestroyFunc destroy_value) {
	DestroyFunc old = map->destroy_value;
	map->destroy_value = destroy_value;
	return old;
}

// Απελευθέρωση μνήμης που δεσμεύει το map
//also for array2 (dont forget)
void map_destroy(Map map) {
	for (int i = 0; i < map->capacity; i++) {
		if (map->array1[i].state == OCCUPIED) {
			if (map->destroy_key != NULL)
				map->destroy_key(map->array1[i].key);
			if (map->destroy_value != NULL)
				map->destroy_value(map->array1[i].value);
		}
	}
	free(map->array1);
	free(map);
}

/////////////////////// Διάσχιση του map μέσω κόμβων ///////////////////////////

MapNode map_first(Map map) {
//Ξεκινάμε την επανάληψή μας απο το 1ο στοιχείο, μέχρι να βρούμε κάτι όντως τοποθετημένο
	for (int i = 0; i < map->capacity; i++)
		if (map->array1[i].state == OCCUPIED)
			return &map->array1[i];

	return MAP_EOF;
}

MapNode map_next(Map map, MapNode node) {
// Το node είναι pointer στο i-οστό στοιχείο του array, οπότε node - array == i  (pointer arithmetic!)
	for (int i = node - map->array1 + 1; i < map->capacity; i++)
		if (map->array1[i].state == OCCUPIED)
			return &map->array1[i];

	return MAP_EOF;	
}

Pointer map_node_key(Map map, MapNode node) {
	return node->key;
}

Pointer map_node_value(Map map, MapNode node) {
	return node->value;
}

MapNode map_find_node(Map map, Pointer key) {
	uint pos1 = map->hash_function(key) % map->capacity;
	uint pos2 = (map->hash_function(key)^2 / map->capacity) % map->capacity;

	if (map->array1[pos1].state == OCCUPIED && map->compare(map->array1[pos1].key,key) == 0)
		return &map->array1[pos1];
	else if(map->array1[pos1].state == EMPTY)
		return MAP_EOF;
	if(map->array2[pos2].state == OCCUPIED && map->compare(map->array2[pos2].key,key) == 0)
		return &map->array2[pos2];
	else if(map->array2[pos2].state == EMPTY)
		return MAP_EOF;
	
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