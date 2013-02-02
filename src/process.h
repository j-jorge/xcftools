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
#ifndef PROCESS_H
#define PROCESS_H

/**
 * \brief The ProcessControl controls how the program behaves while processing
 *        the XCF files.
 */
struct ProcessControl
{
  /** \brief Enables the printing of progress messages about the conversion on
      standard error output. */
  int verboseFlag;

  /**
   * \brief The command to execute to extract a compressed XCF image.
   *
   * This command must take the filename as an argument and print the extracted
   * XCF image on stardard output.
   */
  const char* unzipper;

  /** \brief The string to use to separate the name of the containing groups
      when manipulating layers. */
  const char* pathSeparator;

  /** \brief Use the raw UTF-8 representation from the XCF file to compare
      and display layer names. */
  int use_utf8;

  /** \brief The file to process. */
  const char* inputFile;

}; // struct ProcessControl

void init_process_control( struct ProcessControl* p );

#endif /* PROCESS_H */
