#pragma once
#include <stdbool.h>

struct list;

struct list *list_make(void *const *objects, unsigned elems);

struct list *list_append(const struct list *list, void *object);

struct list *list_concatenate(const struct list *a, const struct list *b);

unsigned list_length(const struct list *list);

void *list_value(const struct list *list, unsigned index);

void *list_reduce(const struct list *list, void *initial, void *(*f)(void *context, void *aggregate, void *value), void *context);

struct list *list_map(const struct list *list, void *(*f)(void *context, void *value), void *context);

struct list *list_filter(const struct list *list, bool (*f)(void *context, void *value), void *context);
