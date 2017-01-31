
/**********************
  AJ ARGUMENT LIBRARY
***********************

by Andrew Apted, July 2014


The code herein is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this code.  Use at your own risk.

Permission is granted to anyone to use this code for any purpose,
including commercial applications, and to freely redistribute and
modify it.  An acknowledgement would be nice, but is not required.

_____

ABOUT
_____

This is a simple header-file library for handling command-line
arguments.

In the file which you want to contain the implementation, add the
following _before_ the inclusion of this header :

    #define AJ_ARG_LIB_IMPLEMENTATION

____________

DEPENDENCIES
____________

No other libraries.

_____

USAGE
_____

// Prepare to handle the arguments.  You should skip the program
// name when calling this (i.e. pass in argc-1, argv+1).
//
// This may remove or re-order the arguments (into an internal list).
// The string pointers are used 'as-is' -- we assume they will never
// be freed or modified until the program ends.
//
// Returns 0 normally, or -1 if there were more arguments than can
// fit in the internal buffer.

aj_arg_Init(int argc, const char **argv)


// Check if argument at given index is an option.
// Returns:
//    1 : if it _IS_ an option (e.g. begins with '-')
//    0 : if not an option
//   -1 : if given index is out of range

aj_arg_IsOption(arg_index)


// Find an option in the argument list.
// The 'short_name' is a single letter, or 0 for none.
// The 'long_name' is a full word, without any '-' prefix, and
// can be NULL for none.
//
// Returns an index number >= 0 when found, or -1 if not found.
// When 'num_params' is not NULL then it will store the number of
// parameters following the option.

aj_arg_Find(short_name, long_name, int *num_params)


// Use this to retrieve a parameter which follows an option.
// The 'arg_index' is the result of aj_arg_Find(), and 'param' is
// the parameter number between 0 and (num_params - 1).
//
// Result undefined (possibly NULL) if not used correctly
// (e.g. if param >= num_params).

aj_arg_Param(arg_index, param)


// Convenience functions, convert parameter to an integer / float.
// Result undefined if parameter is an invalid number or the
// indices are not used correctly.

aj_arg_IntParam  (arg_index, param)
aj_arg_FloatParam(arg_index, param)


// Convenience function, just checks whether an option is present
// or not.

aj_arg_Exists(short_name, long_name)

*/


/* ---------------- API ------------------ */

#ifndef __AJ_ARGUMENT_LIBRARY_H__
#define __AJ_ARGUMENT_LIBRARY_H__

#ifdef __cplusplus
extern "C" {
#endif

int  aj_arg_Init(int argc, const char **argv);
int  aj_arg_IsOption(int arg_index);
int  aj_arg_Find(char short_name, const char *long_name, int *num_params);
int  aj_arg_Exists(char short_name, const char *long_name);

const char * aj_arg_Param(int arg_index, int param);
int   aj_arg_IntParam(int arg_index, int param);
float aj_arg_FloatParam(int arg_index, int param);

#ifdef __cplusplus
}
#endif

#endif /* __AJ_ARGUMENT_LIBRARY_H__ */


/* -------------- IMPLEMENTATION ---------------- */

#ifdef AJ_ARG_LIB_IMPLEMENTATION

#include <stdlib.h>  // atoi
#include <ctype.h>   // toupper


// case insensitive comparison function
#ifndef AJ_ARG_COMPARE
#define AJ_ARG_COMPARE(s1, s2)  ajarg__CaseCmp(s1, s2)
#endif


// maximum number of arguments stored
#ifndef AJ_ARG_MAXIMUM
#define AJ_ARG_MAXIMUM  1024
#endif

const char *ajarg__list[AJ_ARG_MAXIMUM];

int ajarg__count;


int aj_arg_Init(int argc, const char **argv)
{
	int i;
	int result = 0;

	ajarg__count = 0;

	if (argc > AJ_ARG_MAXIMUM)
	{
		argc = AJ_ARG_MAXIMUM;
		result = -1;
	}

	for (i = 0 ; i < argc ; i++)
	{
		const char *cur = argv[i];

		// a NULL?  WTF?
		if (! cur)
			continue;

#ifdef __APPLE__
		// ignore MacOS X rubbish
		if (cur[0] == '-' && cur[1] == 'p' &&
			cur[2] == 's' && cur[3] == 'n')
			continue;
#endif

		ajarg__list[ajarg__count++] = cur;
	}

	// clear the remaining pointers
	for ( ; i < AJ_ARG_MAXIMUM ; i++)
	{
		ajarg__list[i] = NULL;
	}

	return result;
}


int aj_arg_IsOption(int arg_index)
{
	if (arg_index < 0 || arg_index >= ajarg__count)
		return -1;

	if (ajarg__list[arg_index][0] != '-')
		return 0;
	
	// treat negative numbers as NOT options
	if (isdigit(ajarg__list[arg_index][1]))
		return 0;

	return 1;
}


static int ajarg__CaseCmp(const char *A, const char *B)
{
	for (; *A || *B; A++, B++)
	{
		// this test also catches end-of-string conditions
		if (toupper(*A) != toupper(*B))
			return (toupper(*A) - toupper(*B));
	}

	return 0;
}


int aj_arg_Find(char short_name, const char *long_name, int *num_params)
{
	int i;
	const char *cur;

	if (num_params)
		*num_params = 0;

	// nothing ventured, nothing gained
	if (! short_name && ! long_name)
		return -1;

	short_name = toupper(short_name);

	for (i = 0 ; i < ajarg__count ; i++)
	{
		if (! aj_arg_IsOption(i))
			continue;

		cur = ajarg__list[i];

		if (short_name)
		{
			if (toupper(cur[1]) == short_name && cur[2] == 0)
				break;
		}

		if (long_name)
		{
			// support GNU-style long options
			if (cur[0] == '-' && cur[1] == '-')
				cur += 2;
			else
				cur += 1;

			if (AJ_ARG_COMPARE(cur, long_name) == 0)
				break;
		}
	}

	if (i >= ajarg__count)  // NOT FOUND
		return -1;

	if (num_params)
	{
		int p = i + 1;

		while ((p < ajarg__count) && ! aj_arg_IsOption(p))
			p++;

		*num_params = p - (i + 1);
	}

	return i;
}


const char *aj_arg_Param(int arg_index, int param)
{
	int i = arg_index + param + 1;

	if (arg_index < 0 || param < 0)
		return NULL;

	if (i >= ajarg__count)
		return NULL;
	
	return ajarg__list[i];
}


int aj_arg_IntParam(int arg_index, int param)
{
	const char *str = aj_arg_Param(arg_index, param);

	if (! str)
		return 0;

	return atoi(str);
}


float aj_arg_FloatParam(int arg_index, int param)
{
	const char *str = aj_arg_Param(arg_index, param);

	if (! str)
		return 0;

	return atof(str);
}


int aj_arg_Exists(char short_name, const char *long_name)
{
	return aj_arg_Find(short_name, long_name, NULL) >= 0;
}

#endif  /* AJ_ARG_LIB_IMPLEMENTATION */

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
