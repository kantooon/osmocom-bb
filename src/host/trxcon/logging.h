#ifndef _LOGGING_H
#define _LOGGING_H

#define DEBUG
#include <osmocom/core/logging.h>

#define DEBUG_DEFAULT "DAPP"

enum {
	DAPP
};

extern const struct log_info trx_log_info;

int trx_log_init(const char *category_mask);

#endif /* _LOGGING_H */