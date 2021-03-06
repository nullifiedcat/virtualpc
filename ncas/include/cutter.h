/*
  Created by nullifiedcat on 03.03.18.
*/

#pragma once

/*
 * Cutter splits a stream of chars into sentences, removes comments
 *
 * For example:
 *
 * this is a test
 * te,st,in,g: 123 + 456
 * this [is ; a comment
 * this "is a string; literal" ; and a comment
 *
 * Will be converted into
 * [...] is a sentence, "..." is a word
 *
 * [ "this" "is" "a" "test" ]
 * [ "te" "," "st" "," "in" "," "g" ":" "123" "+" "456" ]
 * [ "this" "[" "is" ]
 * [ "this" ""is a string; literal"" ]
 *
 */

#include "array.h"

struct cutter_s
{
    /* temporary array to store current word as cutter reads it */
    struct array_s *characters;
    /* temporary array to store pointers to allocated blocks with words */
    struct array_s *words;
    /* the output array with sentences */
    struct array_s *output;

    struct
    {
        char reading_comment : 1;
        char reading_string : 1;
    } flags;
};

struct cutter_s *cutter_init(struct array_s *output);
void cutter_push(struct cutter_s *cutter, char c);
void cutter_cut_word(struct cutter_s *cutter);
void cutter_cut_sentence(struct cutter_s *cutter);
void cutter_end(struct cutter_s *cutter);
void cutter_destroy(struct cutter_s *cutter);