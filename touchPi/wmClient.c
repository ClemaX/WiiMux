#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>

#include <bluetooth/bluetooth.h>
#include <cwiid.h>

#define DELAY 50

#define toggle_bit(bf,b) \
								(bf) = ((bf) & b)  \
															? ((bf) & ~(b)) \
															: ((bf) | (b))

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
void get_pos(int *buf, struct cwiid_state *state_h, struct cwiid_state *state_v);

void print_usage() {
								printf("Usage: wmclient HOST PORT [-i interval] [-H mac] [-V mac] [-lv]\n\n"
															"Arguments:\n"
															"	-l	Do not use LEDs\n"
															"	-v	Print state to stdout\n");
}

int main(int argc, char *argv[])
{
								/* Sockets */
								struct sockaddr_in server_addr;
								struct timespec ts;
								bool verbose = false;
								char send_str[9];
								int client_s;
								int interval = -1;

								if ((client_s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
																printf("\n Socket creation error \n");
																return -1;
								}
								server_addr.sin_family = AF_INET;
								int flags = fcntl (client_s, F_GETFL, 0);
								fcntl(client_s, F_SETFL, flags | O_NONBLOCK);

								/* CWIID */
								cwiid_wiimote_t *wm_h; // horizontal wiimote
								cwiid_wiimote_t *wm_v; // vertical wiimote

								struct cwiid_state state_h;
								struct cwiid_state state_v;

								bdaddr_t bdaddr_h = { 0 }; // bluetooth device address
								bdaddr_t bdaddr_v = { 0 };

								bool leds = true;
								unsigned char led_state_h = 0;
								unsigned char led_state_v = 0;
								unsigned char rpt_mode = 0;

								int pos[2];

								/* Connect to address given on command-line, if present */
								int c;
								while ((c = getopt (argc, argv, "lvH:V:i:")) != -1)
																switch (c)
																{
																case 'H': str2ba(optarg, &bdaddr_h);
																								break;
																case 'V': str2ba(optarg, &bdaddr_v);
																								break;
																case 'i': interval = atoi(optarg);
																								break;
																case 'l': leds = false;
																								break;
																case 'v': verbose = true;
																								break;
																case '?':
																								if ((optopt == 'H') | (optopt == 'V') | (optopt == 'i'))
																																fprintf (stderr, "Option -%c requires an argument.\n", optopt);
																								else if (isprint (optopt))
																																fprintf (stderr, "Unknown option `-%c'.\n", optopt);
																								else
																																fprintf (stderr, "Unknown option character `\\x%x'.\n",
																																									optopt);
																								return 1;
																default:
																								abort ();
																}

								if (argc - optind > 1) {
																server_addr.sin_addr.s_addr = inet_addr(argv[optind]);
																server_addr.sin_port = htons(atoi(argv[optind+1]));
								} else {
																print_usage();
																exit(EXIT_FAILURE);
								}

								if (interval < 0) {
																interval = DELAY;
								}
								ts.tv_sec = interval / 1000;
								ts.tv_nsec = (interval % 1000) * 1000000;

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
								printf("Sending to %s:%s every %dms\n",
															argv[optind], argv[optind+1], interval);

								/* Set IR reporting mode*/
								toggle_bit(rpt_mode, CWIID_RPT_IR);
								set_rpt_mode(wm_h, rpt_mode);
								set_rpt_mode(wm_v, rpt_mode);

								/* Turn on LEDs */
								if (leds) {
																toggle_bit(led_state_h, CWIID_LED1_ON);
																toggle_bit(led_state_v, CWIID_LED2_ON);
																set_led_state(wm_h, led_state_h);
																set_led_state(wm_v, led_state_v);
								}

								/* Report IR state */
								while (1) {
																if (cwiid_get_state(wm_h, &state_h) | cwiid_get_state(wm_v, &state_v)) {
																								fprintf(stderr, "Error getting state\n");
																}

																get_pos(pos, &state_h, &state_v);

																if ((pos[0] >= 0) && (pos[1] >= 0)) {
																								sprintf(send_str, "%d,%d", pos[0], pos[1]);
																								if (verbose) {
																																printf("%d,%d\n",pos[0], pos[1]);
																								}
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

void get_pos(int *buf, struct cwiid_state *state_h, struct cwiid_state * state_v)
{

								if (state_h->ir_src[0].valid) {
																buf[0] = state_h->ir_src[0].pos[CWIID_X];
								} else {
																buf[0] = -1;
								}

								if (state_v->ir_src[0].valid) {
																buf[1] = state_v->ir_src[0].pos[CWIID_X];
								} else {
																buf[1] = -1;
								}
}
