/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/discuss/server/acl.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/discuss/server/acl.c,v 1.1 1986-11-10 21:16:00 wesommer Exp $
 *
 *	Copyright (C) 1986 by the Student Information Processing Board
 *
 */

#ifndef lint
static char *rcsid_acl_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/discuss/server/acl.c,v 1.1 1986-11-10 21:16:00 wesommer Exp $";
#endif lint

#include "../include/acl.h"
#include <stdio.h>
#include <strings.h>

char *malloc(), *realloc();
char *acl_union(), *acl_intersection(), *acl_subtract();

Bool acl_check(list, principal, modes)
     Acl *list;
     char *principal;
     char *modes;
{
	register acl_entry *ae;
	register int n;
	for (ae = list->acl_entries, n=list->acl_length;
	     n;
	     ae++, n--) {
		if ((strcmp(principal, ae->principal) == 0) ||
		    (strcmp("*", ae->principal) == 0))
			return(acl_is_subset(modes, ae->modes));
	}
	return (FALSE);
}

Acl *acl_read(fd)
     int fd;
{
	static char buf[128];
	FILE *f=fdopen(fd, "r");
	register char *cp;
	register int n;
	register acl_entry *ae;
	register Acl *list = (Acl *) malloc(sizeof(Acl));
	fgets(buf, 128, f);
	n=list->acl_length=atoi(buf);
	list->acl_entries = (acl_entry *)malloc(n * sizeof(acl_entry));

	for (ae=list->acl_entries; n; --n, ++ae)  {
		buf[0]=0;
		fgets(buf, 128, f);
		if(cp=index(buf, '\n')) *cp='\0';
		if(cp=index(buf, ':')) {
			*cp='\0';
			ae->modes = malloc(strlen(buf)+1);
			strcpy(ae->modes, buf);
			ae->principal = malloc(strlen(cp+1)+1);
			strcpy(ae->principal, cp+1);
		} else { /* skip line */
			list->acl_length--;
			--ae;
		}
	}
	return(list);
}

Bool acl_write(fd, list)
     int fd;
     Acl *list;
{
	FILE *f=fdopen(fd, "w");
	register int n;
	register acl_entry *ae;
	fprintf(f, "%d\n", list->acl_length);
	for (ae=list->acl_entries, n=list->acl_length;
	     n;
	     --n, ++ae) fprintf(f, "%s:%s\n", ae->modes, ae->principal);
	fflush(f);
	/* should really free "f", but life sucks, doesn't it? */
}

acl_add_access(list, principal, modes)
     Acl *list;
     char *principal;
     char *modes;
{
	register int n;
	register acl_entry *ae;
	char *new_modes;
	for(ae=list->acl_entries, n=list->acl_length; n; --n, ++ae) {
		if (!strcmp(ae->principal, principal)) {
			new_modes = acl_union(modes, ae->modes);
			free(ae->modes);
			ae->modes=new_modes;
			return;
		}
	}
	/*
	 * Whoops.. fell through.
	 * Realloc the world - or at least the vector.
	 */
	list->acl_length++;
	list->acl_entries = (acl_entry *) realloc(list->acl_entries, 
				  list->acl_length * sizeof(acl_entry));
	ae = list->acl_entries + list->acl_length - 2;
	/*
	 * Is the last entry "*"?  If so, push it back one.
	 */
	if (!strcmp(ae->principal, "*")) {
		if(!strcmp(principal, "*")) 
			panic("acl broke");
		*(ae+1) = *ae;
		--ae;
	}
	ae++;
	ae->principal = malloc(strlen(principal)+1);
	strcpy(ae->principal, principal);
	ae->modes = malloc(strlen(modes)+1);
	strcpy(ae->modes, modes);
}

acl_delete_access(list, principal, modes)
     Acl *list;
     char *principal;
     char *modes;
{
	register acl_entry *ae, *ae1;
	register int n;
	char *new_modes;
	for(ae=list->acl_entries, n=list->acl_length; n; --n, ++ae) {
		if (!strcmp(ae->principal, principal)) {
			new_modes = acl_subtract(modes, ae->modes);
			free(ae->modes);
			ae->modes=new_modes;
			goto cleanup;
		}
	}
	return;
cleanup:
	ae1 = list->acl_entries + list->acl_length - 1;
	if ((strcmp(ae1->principal, "*")!= 0 && strcmp(ae->modes, "")) ||
	    (!strcmp(ae1->principal, "*") && !strcmp(ae->modes, ae1->modes))) {
		free(ae->principal);
		free(ae->modes);
		for (; ae <ae1; ae++) *ae= *(ae+1);
		list->acl_length--;
		list->acl_entries = (acl_entry *) realloc(list->acl_entries,
				       list->acl_length * sizeof(acl_entry));
	}

}	       

acl_replace_access(list, principal, modes)
     Acl *list;
     char *principal;
     char *modes;
{

}

/*
 * Return empty ACL
 */

Acl *acl_create()
{
	register Acl *result = (Acl *) malloc(sizeof(Acl));
	result->acl_length=0;
	result->acl_entries=(acl_entry *) malloc(sizeof(acl_entry));
	return(result);
}

char *acl_get_access(list, principal)
	Acl *list;
	char *principal;
{
	register acl_entry *ae;
	register int n;

	for(ae=list->acl_entries, n=list->acl_length; n; --n, ++ae) {
		if (!strcmp(ae->principal, principal) ||
		    !strcmp(ae->principal, "*")) {
			return(ae->modes);
		}
	}
	return("");

}
/*
 * Destroy an ACL.     
 */

acl_destroy(list)
     Acl *list;
{
	register acl_entry *ae;
	register int n;
	if(!list) return;
	for (ae=list->acl_entries, n=list->acl_length;
	     n;
	     --n, ++ae) {
		free(ae->principal);
		free(ae->modes);
	}
	free(list->acl_entries);
	free(list);
}

/*
 * Returns true if every character of s1 occurs in s2.
 */
Bool acl_is_subset(s1, s2)
     register char *s1, *s2;
{
	register char *last;
	while(*s1 && (last = index(s2, *s1))) s1++;

	return(last != NULL);
}

/*
 * Returns the intersection of the two strings s1 and s2.
 * This function allocates a new string, which should be freed 
 * when not needed.
 */
char *acl_intersection(s1, s2)
     register char *s1, *s2;
{
	register char *result=malloc(1);
	register int resp=0;

	while(*s1) {
		if(index(s2, *s1)) {
			result[resp++] = *s1;
			result=realloc(result, resp+1);
		}
		s1++;
	}
	result[resp] = '\0';
	return(result);
}

char *acl_union(s1, s2)
     register char *s1, *s2;
{
	register int resp=strlen(s2);
	register char *result=malloc(resp+1);
	strcpy(result, s2);
	while(*s1) {
		if (!index(result, *s1)) {
			result[resp++]= *s1;
			result=realloc(result, resp+1);
		}
		s1++;
	}
	result[resp]='\0';
	return(result);
}

char *acl_subtract(s1, s2)
     register char *s1, *s2;
{
	register char *result = malloc(1);
	register int len=0;
	for (; *s2; s2++) {
		if (!index(s1, *s2)) {
			result = realloc(result, len+2);
			result[len++]= *s2;
		}
	}
	result[len]='\0';
	return(result);
}


#ifdef TESTS
#include <sys/file.h>
main() 
{
	int fd;
	Acl *a;

	printf("%s * %s = %s\n", "eabce", "abcd", 
	       acl_intersection("eabce", "abcd"));
	printf("%s + %s = %s\n", "abc", "cbade", acl_union("abc", "cbade"));
	printf("%s < %s = %d\n", "ab", "bcde", acl_is_subset("ab", "bcde"));
	printf("%s < %s = %d\n", "ab", "abcde", acl_is_subset("ab", "abcde"));

	fd = open ("foo.acl", O_RDONLY);
	a=acl_read(fd);
	close(fd);
	printf("a for wesommer: %d\n", acl_check(a, "wesommer", "a"));
	printf("a for foobar: %d\n", acl_check(a, "foobar", "a"));
	printf("a for spook: %d\n", acl_check(a, "spook", "a"));
	printf("g for foobar: %d\n", acl_check(a, "foobar", "g"));
	printf("g for wesommer: %d\n", acl_check(a, "wesommer", "g"));
	printf("g for spook: %d\n", acl_check(a, "spook", "g"));
	printf("d for spook: %d\n", acl_check(a, "spook", "d"));
	printf("d for wesommer: %d\n", acl_check(a, "wesommer", "d"));
	acl_add_access(a, "zot", "qna");
	acl_add_access(a, "spook", "d");
	acl_add_access(a, "*", "w");
	fd = open("bar.acl", O_WRONLY+O_CREAT+O_TRUNC);
	acl_write(fd, a);
	close(fd);
}
panic(s)
	char *s;
{
	printf(*s);
	fflush(stdout);
	abort();
}

#endif TESTS