/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω Cuckoo hashing
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include "ADTMap.h"


typedef enum {
	EMPTY, OCCUPIED, DELETED
} State;

#define MAX_LOAD_FACTOR 0.5

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
	int cuckoo_counter;
};


Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value) {
	// Δεσμεύουμε κατάλληλα τον χώρο που χρειαζόμαστε για τα hash tables
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
	map->cuckoo_counter = 0;

	return map;
}

// Επιστρέφει τον αριθμό των entries του map σε μία χρονική στιγμή.
int map_size(Map map) {
	return map->size;
}

// Συνάρτηση για την επέκταση του Hash Table σε περίπτωση που ο load factor μεγαλώσει πολύ.
static void rehash(Map map) {
	// Αποθήκευση των παλιών δεδομένων
	int old_capacity = map->capacity;
	MapNode old_array1 = map->array1;
	MapNode old_array2 = map->array2;

	// Βρίσκουμε τη νέα χωρητικότητα, διασχίζοντας τη λίστα των πρώτων ώστε να βρούμε τον επόμενο. 
	int prime_no = sizeof(prime_sizes) / sizeof(int);	// το μέγεθος του πίνακα
	for (int i = 0; i < prime_no; i++) {					// LCOV_EXCL_LINE
		if (prime_sizes[i] > old_capacity) {
			map->capacity = prime_sizes[i]; 
			break;
		}
	}
	// Αν έχουμε εξαντλήσει όλους τους πρώτους, διπλασιάζουμε
	if (map->capacity == old_capacity)					// LCOV_EXCL_LINE
		map->capacity *= 2;								// LCOV_EXCL_LINE

	// Δημιουργούμε ένα μεγαλύτερο hash table
	map->array1 = malloc(map->capacity * sizeof(struct map_node));
	for (int i = 0; i < map->capacity; i++)
		map->array1[i].state = EMPTY;

	map->array2 = malloc(map->capacity * sizeof(struct map_node));
	for (int i = 0; i < map->capacity; i++)
		map->array2[i].state = EMPTY;

	// Τοποθετούμε ΜΟΝΟ τα entries που όντως περιέχουν ένα στοιχείο (το rehash είναι και μία ευκαιρία να ξεφορτωθούμε τα deleted nodes)
	map->size = 0;
	for (int i = 0; i < old_capacity; i++) {
		if (old_array1[i].state == OCCUPIED )
			map_insert(map, old_array1[i].key, old_array1[i].value);
		if(old_array2[i].state == OCCUPIED)
			map_insert(map, old_array2[i].key, old_array2[i].value);
	}
	//Αποδεσμεύουμε τον παλιό πίνακα ώστε να μήν έχουμε leaks
	free(old_array1);
	free(old_array2);
}

//συνάρτηση που επεξεργάζεται την περίπτωση των colissions(όχι ολοκληρωμένη)
void edit_arrays(Map map, Pointer key, Pointer value, bool occupied_array1, bool occupied_array2) {
	if(map->cuckoo_counter == map->size) { //σε περίπτωση κύκλου
		rehash(map);
	}
	uint pos1 = map->hash_function(key) % map->capacity;
	uint pos2 = (map->hash_function(key)^2 / map->capacity) % map->capacity;

	map->cuckoo_counter++;
	//αν έχω στοιχεία στον 1ο πίνακα τότε πάμε στον 2ο ο οποίος αν είναι γεμάτος αντικαθιστούμε 
	if(occupied_array1) { // τα παλιά key/value και τα κάνουμε destroy
		if(map->array2[pos2].state == OCCUPIED) {
			if(map->compare(map->array2[pos2].key,key) == 0) {
				if(map->array2[pos2].key != key && map->destroy_key != NULL)
					map->destroy_key(map->array2[pos2].key);
				if(map->array2[pos2].value != key && map->destroy_value != NULL)
					map->destroy_value(map->array2[pos2].value);

				map->array2[pos2].key = key;    //βάζουμε τα νεα key/value
				map->array2[pos2].value = value;
			}
		}
		else if(map->array2[pos2].state == EMPTY) { //αν ο 2ος είναι άδειος βάζουμε τα key/value
			map->array2[pos2].key = key;
			map->array2[pos2].value = value;
			map->array2[pos2].state = OCCUPIED;
			map->size++;
		}
	}
	else if(occupied_array2) { //αν έχω στοιχεία στον 2ο πίνακα τότε πάμε στον 1ο ο οποίος αν είναι γεμάτος αντικαθιστούμε 
		if(map->array1[pos1].state == OCCUPIED) {  // τα παλιά key/value και τα κάνουμε destroy
			if(map->compare(map->array1[pos1].key,key) == 0) {
				if(map->array1[pos1].key != key && map->destroy_key != NULL)
					map->destroy_key(map->array1[pos1].key);
				if(map->array1[pos1].value != key && map->destroy_value != NULL)
					map->destroy_value(map->array1[pos1].value);
				
				map->array1[pos1].key = key;
				map->array1[pos1].value = value;
			}
		}
		else if(map->array1[pos1].state == EMPTY) { //αν ο 1ος είναι άδειος βάζουμε τα key/value
			map->array1[pos1].key = key;
			map->array1[pos1].value = value;
			map->array1[pos1].state = OCCUPIED;
			map->size++;
		}
	}
}
//προσθέτει στο map τον κόμβο με κλειδί key και τιμή value
void map_insert(Map map, Pointer key, Pointer value) {
	Pointer backup_key;
	Pointer backup_value;

	uint pos1 = map->hash_function(key) % map->capacity; //απλή περίπτωση, αν έχει χώρο ο πρώτος πίνακας 
		if(map->array1[pos1].state == EMPTY) {			 //βάλε το στη θέση που του αντιστοιχεί
		map->array1[pos1].key = key;
		map->array1[pos1].value = value;
		map->array1[pos1].state = OCCUPIED;
		map->size++;
	}
	else if(map->array1[pos1].state == OCCUPIED) { // αν δεν χωράει εκεί τότε πάρε αυτά που έχει ο πρώτος πίνακας
		backup_key = map->array1[pos1].key = key;		// δηλαδη τα temp και στέιλε τα στην edit_arrays
		backup_value = map->array1[pos1].value = value;	// και βάλε στον 1ο πίνακα τα νέα στοιχεία
		
		map->array1[pos1].key = key;
		map->array1[pos1].value = value;
		map->array1[pos1].state = OCCUPIED;

		bool occupied_array1 = true; 
		bool occupied_array2 = false;
		edit_arrays(map,backup_key,backup_value,occupied_array1,occupied_array2);
		map->cuckoo_counter = 0;
	}

	float load_factor = (float)map->size / map->capacity; //in case load_Factor exceeds 0.5
	if(load_factor > MAX_LOAD_FACTOR)
		rehash(map);
	//printf("pos:%d\n",pos1);
	//printf("map size :%d\n", map->size);
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
//επιστρέφει το value του κόμβου με κλειδί key
MapNode map_find_node(Map map, Pointer key) {
	uint pos1 = map->hash_function(key) % map->capacity;
	uint pos2 = (map->hash_function(key)^2 / map->capacity) % map->capacity;

	if (map->array1[pos1].state == OCCUPIED) 
		return &map->array1[pos1];
	else if(map->array1[pos1].state == EMPTY)
		return MAP_EOF;
	if(map->array2[pos2].state == OCCUPIED)
		return &map->array2[pos2];
	else if(map->array2[pos2].state == EMPTY)
		return MAP_EOF;
	else 
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