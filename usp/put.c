/* UNIX Unified Stream Protocol 

   Copyright 1986 by the Massachusetts Institute of Technology 
   See permission and disclaimer notice in file "notice.h" 
*/

#include <sys/types.h>
#include <stdio.h>
#include "gen.h"
#include "usp.h"

/* output operations */

USP_put_boolean(us, bo)

USPStream    *us;
USPBoolean   bo;
{
    errno = 0;
    if(! us->us_out_sending_p) {
	errno = UENOTSENDING;
	return(ERROR);
    }
    switch (bo)	{ 
      case TRUE:
      case FALSE:
	break;
      default: 
	errno = UEBADATA;
	return(ERROR);
    }
    bo = (USPBoolean) htons(bo);
    if(put_into_sub_block(us, (char *) &bo, sizeof(USPBoolean)) == ERROR) {
	return(ERROR);
    }
    return(SUCCESS);
}

USP_put_integer(us, ui) 

USPStream    *us;
USPInteger   ui;
{
    errno = 0;
    if(! us->us_out_sending_p) {
	errno = UENOTSENDING;
	return(ERROR);
    }
    ui = (USPInteger) htons((u_short) ui);	
    if(put_into_sub_block(us, (char *) &ui, sizeof(USPInteger)) == ERROR) {
	return(ERROR);
    }
    return(SUCCESS);
}

USP_put_cardinal(us, ca) 

USPStream    *us;
USPCardinal  ca;
{
    errno = 0;
    if(! us->us_out_sending_p) {
	errno = UENOTSENDING;
	return(ERROR);
    }
    ca = (USPCardinal) htons((u_short) ca); 
    if(put_into_sub_block(us, (char *) &ca, sizeof(USPCardinal)) == ERROR) {
	return(ERROR);
    }
    return(SUCCESS);
}

USP_put_long_integer(us, li) 

USPStream       *us;
USPLong_integer li;
{
    errno = 0;
    if(! us->us_out_sending_p) { 
	errno = UENOTSENDING;
	return(ERROR);
    }
    li = (USPLong_integer) htonl((u_long) li); 
    if(put_into_sub_block(us, (char *) &li, sizeof(USPLong_integer)) == ERROR){
	return(ERROR);
    }
    return(SUCCESS);
}

USP_put_long_cardinal(us, lc)

USPStream        *us;
USPLong_cardinal lc;
{
    errno = 0;
    if(! us->us_out_sending_p) { 
	errno = UENOTSENDING;
	return(ERROR);
    }
    lc = (USPLong_cardinal) htonl((u_long) lc); 
    if(put_into_sub_block(us, (char *)&lc, sizeof(USPLong_cardinal)) == ERROR){
	return(ERROR);
    }
    return(SUCCESS);
}

USP_put_string(us, str) 

USPStream    *us;
USPString    str;
{
    USPCardinal	sl = 0, ssl; 
    char 	*sptr = str, nl = '\012', cr = '\r', zero = '\0';

    errno = 0;
    if(! us->us_out_sending_p) { 
	errno = UENOTSENDING;
	return(ERROR);
    }

    /* Process string one byte at a time in order to netasciify */

    /* first find netasciified length by adding 1 extra for every CR or NL */

    while(*sptr) {
	++sl;
	if(*sptr == '\r' || *sptr == '\n') {
	    ++sl;
	}
	++sptr;
    }
    ssl = (USPCardinal) htons((u_short) sl); 
    if(put_into_sub_block(us, (char *) &ssl, sizeof(USPCardinal)) == ERROR) {
	return(ERROR);
    }
    sptr = str;
    while(*sptr) {
	if(*sptr == '\n') {
	    if(put_into_sub_block(us, &cr, (unsigned) 1) == ERROR) { 
		return(ERROR);
	    }
	    if(put_into_sub_block(us, &nl, (unsigned) 1) == ERROR) { 
		return(ERROR);
	    }
	}
	else {
	    if(put_into_sub_block(us, sptr, (unsigned) 1) == ERROR) { 
		return(ERROR);
	    }
	    if(*sptr == '\r') {
		if(put_into_sub_block(us, &zero, (unsigned) 1) == ERROR) { 
		    return(ERROR);
		}
	    }
	}
	++sptr;
    }
    
    /* pad odd length strings */
    
    if(sl & 1) {
	if(put_into_sub_block(us, &zero, sizeof(char)) == ERROR) {
	    return(ERROR);
	}
    }
    return(SUCCESS);
}

/* this puts a raw block of bytes into a USP block.  For those of us who
   have no need of USP's data types... */

USP_put_byte_block(us, buf, len)

USPStream	*us;
Byte		*buf;
unsigned	len;
{
    return(put_into_sub_block(us, buf, len));
}
