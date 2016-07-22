/* 
 * OsmocomBB <-> OsmoTRX connection bridge
 *
 * (C) 2016 by Vadim Yanitskiy <axilirator@gmail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>

#include <osmocom/core/msgb.h>
#include <osmocom/core/talloc.h>
#include <osmocom/core/signal.h>
#include <osmocom/core/select.h>
#include <osmocom/core/application.h>

#include "logging.h"

#define COPYRIGHT \
	"Copyright (C) 2016 by Vadim Yanitskiy <axilirator@gmail.com>\n" \
	"License GPLv2+: GNU GPL version 2 or later " \
		"<http://gnu.org/licenses/gpl.html>\n" \
	"This is free software: you are free to change and redistribute it.\n" \
	"There is NO WARRANTY, to the extent permitted by law.\n\n"

static struct {
	int daemonize;
	int quit;

	const char *trx_ip;
	const char *bind_socket;
	uint16_t trx_base_port;
} app_data;

void *tall_trx_ctx = NULL;

static void print_usage(const char *app)
{
	printf("Usage: %s\n", app);
}

static void print_help(void)
{
	printf(" Some help...\n");
	printf("  -h --help         this text\n");
	printf("  -d --debug        Change debug flags. Default: %s\n", DEBUG_DEFAULT);
	printf("  -i --trx-ip       The IP address of host runing OsmoTRX (default 127.0.0.1)\n");
	printf("  -p --trx-port     Base port of OsmoTRX instance (default 5700)\n");
	printf("  -s --socket       Listening socket for layer23 (default /tmp/osmocom_l2)\n");
	printf("  -D --daemonize    Run as daemon\n");
}

static void handle_options(int argc, char **argv)
{
	while (1) {
		int option_index = 0, c;
		static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			{"debug", 1, 0, 'd'},
			{"socket", 1, 0, 's'},
			{"trx-ip", 1, 0, 'i'},
			{"trx-port", 1, 0, 'p'},
			{"daemonize", 0, 0, 'D'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "d:i:p:s:Dh",
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_usage(argv[0]);
			print_help();
			exit(0);
			break;
		case 'd':
			log_parse_category_mask(osmo_stderr_target, optarg);
			break;
		case 'i':
			app_data.trx_ip = optarg;
			break;
		case 'p':
			app_data.trx_base_port = atoi(optarg);
			break;
		case 's':
			app_data.bind_socket = optarg;
			break;
		case 'D':
			app_data.daemonize = 1;
			break;
		default:
			break;
		}
	}
}

static void init_defaults(void)
{
	app_data.bind_socket = "/tmp/osmocom_l2";
	app_data.trx_ip = "127.0.0.1";
	app_data.trx_base_port = 5700;

	app_data.daemonize = 0;
	app_data.quit = 0;
}

static void signal_handler(int signal)
{
	fprintf(stderr, "signal %u received\n", signal);

	switch (signal) {
	case SIGINT:
		app_data.quit++;
		break;
	case SIGABRT:
	case SIGUSR1:
	case SIGUSR2:
		talloc_report_full(tall_trx_ctx, stderr);
		break;
	default:
		break;
	}
}

int main(int argc, char **argv)
{
	void *tall_msgb_ctx;
	int rc = 0;

	printf("%s", COPYRIGHT);
	init_defaults();
	handle_options(argc, argv);

	// Init talloc memory management system
	tall_trx_ctx = talloc_named_const(NULL, 1, "trxcon context");
	tall_msgb_ctx = talloc_pool(tall_trx_ctx, 100 * 1024);
	msgb_set_talloc_ctx(tall_msgb_ctx);

	// Setup signal handlers
	signal(SIGINT, &signal_handler);
	signal(SIGUSR1, &signal_handler);
	signal(SIGUSR2, &signal_handler);
	osmo_init_ignore_signals();

	// Init logging system
	trx_log_init(NULL);

	// Currently nothing to do
	print_usage(argv[0]);
	print_help();
	exit(0);

	if (app_data.daemonize) {
		rc = osmo_daemonize();
		if (rc < 0) {
			perror("Error during daemonize");
			exit(1);
		}
	}

	while (!app_data.quit) {
		osmo_select_main(0);
	}

	// TODO: close active connections
	return rc;
}