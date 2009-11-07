/*-
 * Copyright (c) 2009 Juan Romero Pardines.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <xbps_api.h>
#include "../xbps-repo/util.h"
#include "defs.h"

static int
list_deps(prop_object_t obj, void *arg, bool *loop_done)
{
	char *pkgname;
	const char *version;

	(void)arg;
	(void)loop_done;

	assert(prop_object_type(obj) == PROP_TYPE_STRING);

	pkgname = xbps_get_pkgdep_name(prop_string_cstring_nocopy(obj));
	version = xbps_get_pkgdep_version(prop_string_cstring_nocopy(obj));
	if (strcmp(version, ">=0") == 0)
		printf("%s\n", pkgname);
	else
		printf("%s%s\n", pkgname, version);

	free(pkgname);
	
	return 0;
}

int
xbps_show_pkg_deps(const char *pkgname)
{
	prop_dictionary_t pkgd, propsd;
	char *path;
	int rv = 0;

	assert(pkgname != NULL);

	pkgd = xbps_find_pkg_installed_from_plist(pkgname);
	if (pkgd == NULL) {
		printf("Package %s is not installed.\n", pkgname);
		return 0;
	}

	/*
	 * Check for props.plist metadata file.
	 */
	path = xbps_xasprintf("%s/%s/metadata/%s/%s", xbps_get_rootdir(),
	    XBPS_META_PATH, pkgname, XBPS_PKGPROPS);
	if (path == NULL)
		return errno;

	propsd = prop_dictionary_internalize_from_file(path);
	free(path);
	if (propsd == NULL) {
		printf("%s: unexistent %s metadata file.\n", pkgname,
		    XBPS_PKGPROPS);
		return errno;
	}

	rv = xbps_callback_array_iter_in_dict(propsd, "run_depends",
	     list_deps, NULL);
	prop_object_release(propsd);
	prop_object_release(pkgd);

	xbps_release_regpkgdb_dict();

	return rv;
}

int
xbps_show_pkg_reverse_deps(const char *pkgname)
{
	prop_dictionary_t pkgd;
	int rv = 0;

	pkgd = xbps_find_pkg_installed_from_plist(pkgname);
	if (pkgd == NULL) {
		printf("Package %s is not installed.\n", pkgname);
		return 0;
	}

	rv = xbps_callback_array_iter_in_dict(pkgd, "requiredby",
	    list_strings_sep_in_array, NULL);
	prop_object_release(pkgd);
	xbps_release_regpkgdb_dict();

	return rv;
}
