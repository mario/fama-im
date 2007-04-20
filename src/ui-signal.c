#include <glib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "common.h"

int signal_pipe[2];

void
pipe_signals(int signal)
{
	if (write(signal_pipe[1], &signal, sizeof(int)) != sizeof(int)) {
		g_warning("Unix signal %d lost\n", signal);
	}
}

gboolean
deliver_signal(GIOChannel * source, GIOCondition cond, gpointer d)
{
	GError *error = NULL;	/* for error handling */

	union {
		gchar chars[sizeof(int)];
		int signal;
	} buf;
	GIOStatus status;	/* save the reading status */
	gsize bytes_read;	/* save the number of chars read */

	/*
	 * Read from the pipe as long as data is available. The reading end is 
	 * also in non-blocking mode, so if we have consumed all unix signals, 
	 * the read returns G_IO_STATUS_AGAIN. 
	 */
	while ((status = g_io_channel_read_chars(source, buf.chars,
						 sizeof(int), &bytes_read,
						 &error)) ==
	       G_IO_STATUS_NORMAL) {
		g_assert(error == NULL);	/* no error if reading returns normal */

		/*
		 * There might be some problem resulting in too few char's read.
		 * Check it.
		 */
		if (bytes_read != sizeof(int)) {
			g_warning
				("lost data in signal pipe (expected %u, received %d)\n",
				 sizeof(int), (int)bytes_read);
			continue;	/* discard the garbage and keep fingers crossed */
		}

		switch (buf.signal) {
		case SIGWINCH:
			redraw_interface();
			break;
		default:
			break;

		}

	}

	if (error != NULL) {
		g_error("reading signal pipe failed: %s\n", error->message);
	}

	if (status == G_IO_STATUS_EOF) {
		g_error("signal pipe has been closed\n");
	}

	g_assert(status == G_IO_STATUS_AGAIN);
	return (TRUE);
}


void
signal_handler_setup()
{
	GIOChannel *g_signal_in;
	GError *error = NULL;	/* handle errors */
	long fd_flags;		/* used to change the pipe into non-blocking mode */

	g_assert(pipe(signal_pipe) == 0);

	fd_flags = fcntl(signal_pipe[1], F_GETFL);
	g_assert(fd_flags != -1);

	g_assert(fcntl(signal_pipe[1], F_SETFL, fd_flags | O_NONBLOCK) != -1);

	signal(SIGWINCH, pipe_signals);

	g_signal_in = g_io_channel_unix_new(signal_pipe[0]);

	/*
	 * we only read raw binary data from the pipe, 
	 * therefore clear any encoding on the channel
	 */
	g_io_channel_set_encoding(g_signal_in, NULL, &error);
	if (error != NULL) {	/* handle potential errors */
		g_error("g_io_channel_set_encoding failed %s\n",
			error->message);
	}

	/*
	 * put the reading end also into non-blocking mode 
	 */
	g_io_channel_set_flags(g_signal_in,
			       g_io_channel_get_flags(g_signal_in) |
			       G_IO_FLAG_NONBLOCK, &error);

	if (error != NULL) {	/* tread errors */
		g_error("g_io_set_flags failed %s\n", error->message);
	}

	/*
	 * register the reading end with the event loop 
	 */
	g_io_add_watch(g_signal_in, G_IO_IN | G_IO_PRI, deliver_signal, NULL);
}
