/*
 * This file was written by Julien Jorge <julien.jorge@gamned.org>
 * It is hereby in the public domain.
 * 
 * In jurisdictions that do not recognise grants of copyright to the
 * public domain: I, the author and (presumably, in those jurisdictions)
 * copyright holder, hereby permit anyone to distribute and use this code,
 * in source code or binary form, with or without modifications. This
 * permission is world-wide and irrevocable.
 *
 * Of course, I will not be liable for any errors or shortcomings in the
 * code, since I give it away without asking any compenstations.
 *
 * If you use or distribute this code, I would appreciate receiving
 * credit for writing it, in whichever way you find proper and customary.
 */
#ifndef VERSION_H
#define VERSION_H

#define TO_STR_BIS(v) # v
#define TO_STR(v) TO_STR_BIS(v)

#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define RELEASE_NUMBER 8
#define PACKAGE_STRING "Xcftools "                              \
  TO_STR(MAJOR_VERSION) "." TO_STR(MINOR_VERSION)               \
  "." TO_STR(RELEASE_NUMBER)

#endif
