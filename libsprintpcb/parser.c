//
// Created by Benedikt on 30.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "parser.h"
#include "primitives.h"
#include "token.h"
#include "errors.h"

sprint_parser* sprint_parser_create(sprint_tokenizer* tokenizer)
{
    if (tokenizer == NULL || tokenizer->read == NULL) return NULL;


}
