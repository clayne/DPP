/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#include <dpp/discordevents.h>
#include <dpp/cluster.h>
#include <dpp/channel.h>
#include <dpp/stringops.h>
#include <dpp/json.h>


namespace dpp::events {


void thread_delete::handle(discord_client* client, json& j, const std::string& raw) {
	json& d = j["d"];

	dpp::thread t;
	t.fill_from_json(&d);
	dpp::guild* g = dpp::find_guild(t.guild_id);
	if (g) {
		g->threads.erase(std::remove(g->threads.begin(), g->threads.end(), t.id), g->threads.end());
	}
	if (!client->creator->on_thread_delete.empty()) {
		dpp::thread_delete_t td(client->owner, client->shard_id, raw);
		td.deleted = t;
		td.deleting_guild = g ? *g : guild{};
		td.deleting_guild.id = t.guild_id;
		client->creator->queue_work(1, [c = client->creator, td]() {
			c->on_thread_delete.call(td);
		});
	}
}
};
