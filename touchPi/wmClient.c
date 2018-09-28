#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>

#include <bluetooth/bluetooth.h>
#include <cwiid.h>

#define PORT 11000
#define IP_ADDR "192.168.0.73"
#define DELAY 100

#define toggle_bit(bf,b) \
								(bf) = ((bf) & b)  \
															? ((bf) & ~(b)) \
															: ((bf) | (b))

void print_state(struct cwiid_state *state);

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);

int *get_pos(struct cwiid_state *state_h, struct cwiid_state *state_v);

cwiid_err_t err;

void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
								if (wiimote) printf("%d:", cwiid_get_id(wiimote));
								else printf("-1:");
								vprintf(s, ap);
								printf("\n");
}

int main(int argc, char *argv[])
{
								bdaddr_t my_bdaddr_any = { 0 };

								int client_s;
								struct sockaddr_in server_addr;
								char send_str[9];
								if ((client_s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
																printf("\n Socket creation error \n");
																return -1;
								}
							  int flags = fcntl	(client_s, F_GETFL, 0);
							  fcntl(client_s, F_SETFL, flags | O_NONBLOCK);

								server_addr.sin_family = AF_INET;                 // Address family to use
							  server_addr.sin_port = htons(PORT);           // Port num to use
							  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR); // IP address to use

								struct timespec ts;
								ts.tv_sec = DELAY / 1000;
    						ts.tv_nsec = (DELAY % 1000) * 1000000;

								cwiid_wiimote_t *wm_h; // horizontal wiimote
								cwiid_wiimote_t *wm_v; // vertical wiimote

								struct cwiid_state state_h;
								struct cwiid_state state_v;

								bdaddr_t bdaddr_h; // bluetooth device address
								bdaddr_t bdaddr_v;

								unsigned char led_state_h = 0;
								unsigned char led_state_v = 0;
								unsigned char rpt_mode = 0;

								int *pos;

								cwiid_set_err(err);

								/* Connect to address given on command-line, if present */
								if (argc > 2) {
																str2ba(argv[1], &bdaddr_h);
																str2ba(argv[2], &bdaddr_v);
								}
								else {
																if (argc > 1) {
																								printf("Usage: wmClient [host:port] [[MAC_H] [MAC_V]]\n"
																															"Could not parse valid mac addresses.\n");
																}
																printf("Using bluetooth discovery...\n");
																bdaddr_h = my_bdaddr_any;
																bdaddr_v = my_bdaddr_any;
								}


								/* Connect to the wiimote */
								printf("Put horizontal Wiimote in discoverable mode now (press 1+2)...\n");
								if (!(wm_h = cwiid_open(&bdaddr_h, 0))) {
																fprintf(stderr, "Unable to connect to horizontal Wiimote\n");
																return -1;
								}

								printf("Put vertical Wiimote in discoverable mode now (press 1+2)...\n");
								if (!(wm_v = cwiid_open(&bdaddr_v, 0))) {
																fprintf(stderr, "Unable to connect to vertical Wiimote\n");
																return -1;
								}

								/* Set IR reporting mode*/
								toggle_bit(rpt_mode, CWIID_RPT_IR);
								set_rpt_mode(wm_h, rpt_mode);
								set_rpt_mode(wm_v, rpt_mode);

								/* Turn on LEDs */
								toggle_bit(led_state_h, CWIID_LED1_ON);
								toggle_bit(led_state_v, CWIID_LED2_ON);
								set_led_state(wm_h, led_state_h);
								set_led_state(wm_v, led_state_v);

								while (1) {
																if (cwiid_get_state(wm_h, &state_h) | cwiid_get_state(wm_v, &state_v)) {
																								fprintf(stderr, "Error getting state\n");
																}

																pos = get_pos(&state_h, &state_v);

																if ((pos[0] >= 0) && (pos[1] >= 0)) {
																								sprintf(send_str, "%d,%d", pos[0], pos[1]);
																								printf("%d,%d\n",pos[0], pos[1]);
																								sendto(client_s, send_str, strlen(send_str)+1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
																								nanosleep(&ts, NULL);
																}
								}
}

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state)
{
								if (cwiid_set_led(wiimote, led_state)) {
																fprintf(stderr, "Error setting LEDs \n");
								}
}

void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
								if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
																fprintf(stderr, "Error setting report mode\n");
								}
}

void print_state(struct cwiid_state *state)
{
								int valid_source = 0;
								for (int i = 0; i < CWIID_IR_SRC_COUNT; i++) {
																if (state->ir_src[i].valid) {
																								valid_source = 1;
																								printf("(%d,%d) ", state->ir_src[i].pos[CWIID_X],
																															state->ir_src[i].pos[CWIID_Y]);
																}
								}
								if (!valid_source) {
																printf("no sources detected");
								}
								printf("\n");
}

int *get_pos(struct cwiid_state *state_h, struct cwiid_state * state_v)
{
								static int pos[2] = { -1 };
								if (state_h->ir_src[0].valid) {
																pos[0] = state_h->ir_src[0].pos[CWIID_X];
								}

								if (state_v->ir_src[0].valid) {
																pos[1] = state_v->ir_src[0].pos[CWIID_X];
								}
								return pos;
}

// TODO: host:port as argument
