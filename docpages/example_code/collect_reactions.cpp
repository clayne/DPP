#include <dpp/dpp.h>

/* To create a collector we must derive from dpp::collector. As dpp::collector is a complicated template,
 * various pre-made forms exist such as this one, reaction_collector.
 */
class react_collector : public dpp::reaction_collector {
public:
	/* Collector will run for 20 seconds */
	react_collector(dpp::cluster* cl, dpp::snowflake id) : dpp::reaction_collector(cl, 20, id) { }

	/* Override the "completed" event and then output the number of collected reactions as a message. */
	virtual void completed(const std::vector<dpp::collected_reaction>& list) override {
		if (list.size()) {
			owner->message_create(dpp::message(list[0].react_channel.id, "I collected " + std::to_string(list.size()) + " reactions!"));
		} else {
			owner->message_create(dpp::message("... I got nothin'."));
		}
	}
};


int main() {
	/* Create bot */
	dpp::cluster bot("token", dpp::i_default_intents | dpp::i_message_content);

	/* Pointer to reaction collector */
	react_collector* r = nullptr;

	bot.on_log(dpp::utility::cout_logger());

	/* Message handler */
	bot.on_message_create([&r, &bot](const dpp::message_create_t& event) {

		/* If someone sends a message that has the text 'collect reactions!' start a reaction collector */
		if (event.msg.content == "collect reactions!" && r == nullptr) {
			/* Create a new reaction collector to collect reactions */
			r = new react_collector(&bot, event.msg.id);
		}

	});

	/* Start bot */
	bot.start(dpp::st_wait);

	return 0;
}
