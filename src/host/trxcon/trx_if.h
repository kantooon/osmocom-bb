#ifndef TRX_IF_H
#define TRX_IF_H

#include <osmocom/core/timer.h>

struct trx_ctrl_msg {
	struct llist_head list;
	unsigned int cmd_id;
	char cmd[128];
	int critical;
	int cmd_len;
};

typedef void (trx_ctrl_cb_t)
	(int response, struct trx_ctrl_msg *msg);

struct trx_instance {
	struct osmo_fd trx_ofd_clck;
	struct osmo_fd trx_ofd_ctrl;
	struct osmo_fd trx_ofd_data;

	struct osmo_timer_list trx_ctrl_timer;
	struct llist_head trx_ctrl_list;

	// This counter is used for unique command ID generation
	unsigned int cmd_id_counter;

	// This callback is called as soon as transceiver responds
	unsigned int cb_cmd_id;
	trx_ctrl_cb_t *cb;
	int cb_enabled;
};

void trx_if_close(struct trx_instance *trx);
int trx_if_open(struct trx_instance **trx, const char *host, uint16_t port);

void trx_if_flush_ctrl(struct trx_instance *trx);
int trx_ctrl_set_cb(struct trx_instance *trx, trx_ctrl_cb_t *cb);

int trx_if_cmd_poweron(struct trx_instance *trx);
int trx_if_cmd_poweroff(struct trx_instance *trx);

int trx_if_cmd_setpower(struct trx_instance *trx, int db);
int trx_if_cmd_adjpower(struct trx_instance *trx, int db);

int trx_if_cmd_setrxgain(struct trx_instance *trx, int db);
int trx_if_cmd_setmaxdly(struct trx_instance *trx, int dly);

int trx_if_cmd_sync(struct trx_instance *trx);
int trx_if_cmd_rxtune(struct trx_instance *trx, uint16_t arfcn);
int trx_if_cmd_txtune(struct trx_instance *trx, uint16_t arfcn);

int trx_if_cmd_setslot(struct trx_instance *trx, uint8_t tn, uint8_t type);

#endif /* TRX_IF_H */
