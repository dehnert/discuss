/*
 *
 * auth_krb () -- Authentication procedure for kerberos.  This contains the
 *		  standard authentication for kerberos.
 *
 */

#include <ctype.h>
#include "krb.h"

extern int krb_err_base;

char *index (), *local_host_name ();

#define NULL 0

/*
 *
 * get_authenticator () -- Interface routine to get an authenticator over
 *			   the net.  Input is a service name (for kerberos,
 *			   this is in the form of service@REALM), optional
 *			   checksum.  We return a pointer to the authenticator,
 *			   its length, and a standard error code.
 *
 */
get_authenticator (service_id, checksum, authp, authl, result)
char *service_id;
int checksum;
char **authp;
int *authl;
int *result;
{
     char *realmp,*instancep;
     char serv [SNAME_SZ+INST_SZ];
     int rem;

     static KTEXT_ST ticket;
     static char phost[MAX_HSTNM];


     init_krb_err_tbl();

     realmp = index (service_id, '@');
     if (realmp == NULL || realmp - service_id >= sizeof (serv)) {
	  realmp = "";
	  strncpy (serv, service_id, sizeof (serv));
     } else {
	  bcopy (service_id, serv, realmp - service_id);	/* copy over to serv */
	  serv [realmp - service_id] = '\0';
	  realmp++;
     }

     /* look for service instance */
     instancep = index (serv, '.');
     if (instancep == NULL) {
	  instancep == "";
     } else {
	  *instancep++ = '\0';
     }

     rem = mk_ap_req (&ticket, serv, instancep, realmp, checksum);
     if (rem == KSUCCESS) {
	  *authl = ticket.length;
	  *authp = (char *) ticket.dat;
	  *result = 0;
     } else {
	  *authl = 0;
	  *authp = NULL;
	  *result = rem + krb_err_base;
     }
}
	  