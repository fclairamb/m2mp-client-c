/* 
 * File:   dictionnary.h
 * Author: florent
 *
 * Created on April 12, 2012, 2:39 AM
 */

#ifndef DICTIONNARY_H
#define	DICTIONNARY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "linkedlist.h"

	typedef struct st_dictionnary {
		linkedlist list;
	} dictionnary;

	typedef struct st_dictionnary_entry {
		char * name;
		char * value;
	} dictionnary_entry;

	/**
	 * Create a new dictionnay
     * @return Instance of the dictionnary object 
     */
	dictionnary * dictionnary_new();
	
	/**
	 * Initialize a dictionnary 
     * @param this Instance of the dictionnary object 
     */
	void dictionnary_init(dictionnary * this);
	
	/**
	 * Delete a dictionnary
     * @param this Instance of the dictionnary object 
     */
	void dictionnary_delete(dictionnary * this);
	
	/**
	 * Wipeout a dictionnary
     * @param this Instance of the dictionnary object 
     */
	void dictionnary_clear( dictionnary * this );
	
	/**
	 * Add a value to a dictionnary
     * @param this Instance of the dictionnary object 
     * @param name Key to add
     * @param value Value to add
     */
	void dictionnary_add(dictionnary * this, const char * name, const char * value);
	
	/**
	 * Add a value to a dictionnary
     * @param this Instance of the dictionnary object 
     * @param name Key to add
     * @param value Value to add
     */
	void dictionnary_put(dictionnary * this, const char * name, const char * value);
	
	/**
	 * Get a dictionnary entry
     * @param this Instance of the dictionnary object 
     * @param name Key of the entry to get
     * @return Value of the dictionnary entry
     */
	dictionnary_entry * dictionnary_get_entry( dictionnary * this, const char * name );
	
	/**
	 * Get a value for a key
     * @param this Instance of the dictionnary object 
     * @param name
     * @return 
     */
	char * dictionnary_get_value(dictionnary * this, const char * name);
	
	/**
	 * Get all the dictionnary entries
     * @param this Instance of the dictionnary object 
     * @return Array of dictionnary entries
     */
	dictionnary_entry ** dictionnary_get_all( dictionnary * this );
	
	/**
	 * Remove a dictionnary entry
     * @param this Instance of the dictionnary object 
     * @param name Key of the entry to remove
	 * @return 0 if the object waw removed, 1 otherwize
     */
	unsigned char dictionnary_rmv(dictionnary * this, const char * name);

#ifdef	__cplusplus
}
#endif

#endif	/* DICTIONNARY_H */

