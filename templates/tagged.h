#pragma once

struct tagged;

struct tagged *tagged_make(void *tag, void *value);

void *tagged_tag(struct tagged *tagged);

void *tagged_value(struct tagged *tagged);
