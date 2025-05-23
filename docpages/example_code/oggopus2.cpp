#include <dpp/dpp.h>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <oggz/oggz.h>

int main() {
	/* Load an ogg opus file into memory.
	 * The bot expects opus packets to be 2 channel stereo, 48000Hz.
	 * 
	 * You may use ffmpeg to encode songs to ogg opus:
	 * ffmpeg -i /path/to/song -c:a libopus -ar 48000 -ac 2 -vn -b:a 96K /path/to/opus.ogg 
	 */

	/* Setup the bot */
	dpp::cluster bot("token");

	bot.on_log(dpp::utility::cout_logger());

	/* The event is fired when someone issues your commands */
	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {

		/* Check which command they ran */
		if (event.command.get_command_name() == "join") {
			/* Get the guild */
			dpp::guild* g = dpp::find_guild(event.command.guild_id);

			/* Attempt to connect to a voice channel, returns false if we fail to connect. */
			if (!g->connect_member_voice(*event.owner, event.command.get_issuing_user().id)) {
				event.reply("You don't seem to be in a voice channel!");
				return;
			}
			
			/* Tell the user we joined their channel. */
			event.reply("Joined your channel!");
		} else if (event.command.get_command_name() == "play") {
			/* Get the voice channel the bot is in, in this current guild. */
			dpp::voiceconn* v = event.from()->get_voice(event.command.guild_id);

			/* If the voice channel was invalid, or there is an issue with it, then tell the user. */
			if (!v || !v->voiceclient || !v->voiceclient->is_ready()) {
				event.reply("There was an issue with getting the voice channel. Make sure I'm in a voice channel!");
				return;
			}

			/* load the audio file with oggz */
			OGGZ *track_og = oggz_open("/path/to/opus.ogg", OGGZ_READ);

			/* If there was an issue reading the file, tell the user and stop */
			if (!track_og) {
				fprintf(stderr, "Error opening file\n");
				event.reply("There was an issue opening the file!");
				return;
			}

			/* set read callback, this callback will be called on packets with the serialno,
			 * -1 means every packet will be handled with this callback.
			 */
			oggz_set_read_callback(
				track_og, -1,
				[](OGGZ *oggz, oggz_packet *packet, long serialno,
					void *user_data) {
					dpp::voiceconn *voiceconn = (dpp::voiceconn *)user_data;

					/* send the audio */
					voiceconn->voiceclient->send_audio_opus(packet->op.packet,
							packet->op.bytes);

					/* make sure to always return 0 here, read more on oggz documentation */
					return 0;
				},
				/* this will be the value of void *user_data */
				(void *)v
			);

			// read loop
			while (v && v->voiceclient && !v->voiceclient->terminating) {
				/* you can tweak this to whatever. Here I use BUFSIZ, defined in
				 * stdio.h as 8192.
				 */
				static const constexpr long CHUNK_READ = BUFSIZ * 2;

				const long read_bytes = oggz_read(track_og, CHUNK_READ);

				/* break on eof */
				if (!read_bytes) {
					break;
				}
			}

			/* Don't forget to free the memory */
			oggz_close(track_og);

			event.reply("Finished playing the audio file!");
		}
	});

	bot.on_ready([&bot](const dpp::ready_t & event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			/* Create a new command. */
			dpp::slashcommand joincommand("join", "Joins your voice channel.", bot.me.id);
			dpp::slashcommand playcommand("play", "Plays an ogg file.", bot.me.id);

			bot.global_bulk_command_create({ joincommand, playcommand });
		}
	});
	
	/* Start bot */
	bot.start(dpp::st_wait);
	
	return 0;
}
