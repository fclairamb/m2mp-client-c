/* 
 * File:   str.h
 * Author: florent
 *
 * Created on April 13, 2012, 1:05 AM
 */

#ifndef STR_H
#define	STR_H

#ifdef	__cplusplus
extern "C" {
#endif
	
	/**
	 * Clone an array of strings
     * @param array Source array of strings to clone
     * @return Created array of strings
	 * 
	 * This allocates a new array and each string of the previous array was copied.
     */
	char ** str_array_clone(const char ** array);

	/**
	 * Implode all the cells of an array to a string separated with a separator
     * @param array Array of strings
     * @param separator Separator
     * @return Generated string
     */
	char * str_array_implode(const char ** array, const char *separator);

	/**
	 * Free an array of strings
     * @param array Array of strings
	 * 
	 * This will also free each string within the array.
     */
	void str_array_free(char ** array);
	
	/**
	 * Append a string to an other string.
     * @param str String to realloc and append to
     * @param add String to append from
     */
	void str_append( char ** dest, const char * src);


#ifdef	__cplusplus
}
#endif

#endif	/* STR_H */

