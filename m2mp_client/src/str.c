#include <string.h>
#include "memwatcher.h"

#include "str.h"

void str_append( char ** str, const char * add ) {
	
	// We calculate the length of the destination string
	int s1 = strlen( *str );
	int s2 = strlen( add );
	*str = mw_realloc( *str, sizeof(char) * (s1 + s2+1) );
	memcpy( *str + (sizeof(char) * s1), add, s2+1 );
}

char * str_array_implode( const char ** array, const char *separator ) {
	int i = 0;
	
	char * out = strdup("");
	
	while( array[i]) {
		if ( i > 0 )
			str_append( & out, separator );
		str_append( & out, array[i++]);
	}
	return out;
}

char ** str_array_clone(const char ** array) {
    int len = 0;
    while (array[len])
        ++len;

    char ** new_array = mw_malloc(sizeof (char *) * (len + 1));

    int i;
    for (i = 0; i < len; ++i)
        new_array[i] = strdup(array[i]);

    new_array[len] = NULL;

    return new_array;
}

void str_array_free(char ** array) {
    int i = 0;
    while (array[i])
        mw_free(array[i++]);
    mw_free(array);
}
