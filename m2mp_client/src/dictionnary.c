

#include "dictionnary.h"
#include "memwatcher.h"
#include "str.h"
#include "m2mp_client_internal.h"


dictionnary_entry * dictionnary_entry_new(const char * name, const char * value) {
	dictionnary_entry * this = (dictionnary_entry *) mw_malloc(sizeof (dictionnary_entry));
	this->name = str_clone(name);
	this->value = str_clone(value);
	return this;
}

void dictionnary_entry_delete(dictionnary_entry * this) {
	mw_free(this->name);
	mw_free(this->value);
	mw_free(this);
}

void dictionnary_entry_delete_void(void * entry) {
	dictionnary_entry_delete((dictionnary_entry *) entry);
}

dictionnary * dictionnary_new() {
	dictionnary * dic = mw_malloc(sizeof ( dictionnary));
	dictionnary_init(dic);
	return dic;
}

void dictionnary_init(dictionnary* this) {
	linkedlist_init(& this->list);
}

void dictionnary_add(dictionnary * this, const char* name, const char* value) {
	linkedlist_insert_first(& this->list, dictionnary_entry_new(name, value));
}

void dictionnary_put(dictionnary* this, const char* name, const char* value) {
	dictionnary_rmv(this, name);
	dictionnary_add(this, name, value);
}

dictionnary_entry * dictionnary_get_entry(dictionnary* this, const char* name) {
	linkedlist_node * node = this->list.first;
	while (node) {
		dictionnary_entry * entry = (dictionnary_entry *) node->element;
		if (!strcmp(name, entry->name))
			return entry;
		node = node->next;
	}
	return NULL;
}

char * dictionnary_get_value(dictionnary* this, const char* name) {
	dictionnary_entry * entry = dictionnary_get_entry( this, name );
	return entry ? entry->value : NULL;
}

dictionnary_entry ** dictionnary_get_all(dictionnary* this) {
	dictionnary_entry ** array = (dictionnary_entry **) linkedlist_get_array( & this->list );
	
	return array;
}

void dictionnary_rmv(dictionnary * this, const char * name) {
	linkedlist_node * node = this->list.first;
	while (node) {
		dictionnary_entry * entry = (dictionnary_entry *) node->element;
		if (!strcmp(name, entry->name)) {
			linkedlist_remove(& this->list, (void*) entry);
			break;
		}
		node = node->next;
	}
}

void dictionnary_clear(dictionnary* this) {
	linkedlist_empty( & this->list, dictionnary_entry_delete_void );
}

void dictionnary_delete(dictionnary * this) {
	dictionnary_clear(this);
	mw_free(this);
}