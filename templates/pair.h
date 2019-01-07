#pragma once

struct pair;

struct pair *pair_make(void *first, void *second);

void *pair_1st(struct pair *pair);

void *pair_2nd(struct pair *pair);
