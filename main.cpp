#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <algorithm>
#include <vector>
#include <cmath>

#include "pugixml.hpp"

namespace RandoRater {

	const long long int kBigNumber = 1000000000000;
	const bool DEBUG = false;

	const long long int kTensTable[11] = {
		0,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000,
		1000000000,
		10000000000
	};

	enum class ItemCost { //Simple keys as cost are defined as macros
		kGrub, kEssence, kOther
	};
	struct Item {
		std::string name;
		std::string location;
		ItemCost cost_type;
		int cost;
		Item() : name(""), location(""), cost_type(ItemCost::kOther), cost(0) {}
		Item(std::string vanilla_item) : name(vanilla_item), location(vanilla_item), cost_type(ItemCost::kOther), cost(0) {}
		Item(std::string name, std::string location) : name(name), location(location), cost_type(ItemCost::kOther), cost(0) {}
		bool operator==(const Item& other) const {
			return name == other.name && location == other.location;
		}
	};
	struct ItemHasher {
		std::size_t operator()(const Item& item) const {
			return std::hash<std::string>()(item.name);
		}
	};
	struct LoadoutRating {
		int rating;
		std::unique_ptr<std::vector<std::string>> loadout;
		LoadoutRating(int rating, std::unique_ptr<std::vector<std::string>> loadout) : rating(rating), loadout(std::move(loadout)) {}
	};
	struct RandoSettings {
		std::string start_location;
		bool randomized_grubs, randomized_roots;
	};
	std::unordered_map<std::string, std::string> start_location_lookup {
		std::make_pair("King's Pass", "King's_Pass"),
		std::make_pair("Stag Nest", "Stag_Nest"),
		std::make_pair("West Crossroads", "Crossroads"),
		std::make_pair("East Crossroads", "Crossroads"),
		std::make_pair("Ancestral Mound", "Ancestral_Mound"),
		std::make_pair("West Fog Canyon", "Left_Fog_Canyon"),
		std::make_pair("East Fog Canyon", "Right_Fog_Canyon"),
		std::make_pair("Queen's Station", "Queen's_Station"),
		std::make_pair("Fungal Wastes", "Fungal_Wastes"),
		std::make_pair("Fungal Core", "Fungal_Core"),
		std::make_pair("Distant Village", "Distant_Village"),
		std::make_pair("Abyss", "Abyss"),
		std::make_pair("Hive", "Hive"),
		std::make_pair("Kingdom's Edge", "Central_Kingdom's_Edge"),
		std::make_pair("Hallownest's Crown", "Hallownest's_Crown"),
		std::make_pair("Crystallized Mound", "Crystallized_Mound"),
		std::make_pair("Royal Waterways", "Upper_Left_Waterways"),
		std::make_pair("Queen's Gardens", "Top_Left_Queen's_Gardens"),
		std::make_pair("Far Greenpath", "Greenpath"),
		std::make_pair("Greenpath", "Greenpath"),
		std::make_pair("City Storerooms", "Left_Elevator"),
		std::make_pair("King's Station", "Upper_King's_Station"),
		std::make_pair("Outside Colosseum", "Top_Kingdom's_Edge"),
		std::make_pair("City of Tears", "Left_City")
	};
	std::unordered_set<std::string> default_grub_locations {
		"Grub-Greenpath_Stag",
		"Grub-Hive_Internal",
		"Grub-City_of_Tears_Guarded",
		"Grub-Crossroads_Center",
		"Grub-Collector_3",
		"Grub-Waterways_East",
		"Grub-King's_Station",
		"Grub-Soul_Sanctum",
		"Grub-Crossroads_Spike",
		"Grub-Howling_Cliffs",
		"Grub-Queen's_Gardens_Stag",
		"Grub-Dark_Deepnest",
		"Grub-Fog_Canyon",
		"Grub-Waterways_Main",
		"Grub-Crystal_Peak_Spike",
		"Grub-Deepnest_Nosk",
		"Grub-Crystal_Peak_Crushers",
		"Grub-Crossroads_Guarded",
		"Grub-Resting_Grounds",
		"Grub-Waterways_Requires_Tram",
		"Grub-Crystal_Peak_Mimic",
		"Grub-Fungal_Spore_Shroom",
		"Grub-Basin_Requires_Wings",
		"Grub-Greenpath_MMC",
		"Grub-City_of_Tears_Left",
		"Grub-Crossroads_Acid",
		"Grub-Basin_Requires_Dive",
		"Grub-Hallownest_Crown",
		"Grub-Queen's_Gardens_Marmu",
		"Grub-Crossroads_Stag",
		"Grub-Fungal_Bouncy",
		"Grub-Crystal_Peak_Below_Chest",
		"Grub-Collector_1",
		"Grub-Greenpath_Cornifer",
		"Grub-Watcher's_Spire",
		"Grub-Hive_External",
		"Grub-Deepnest_Mimic",
		"Grub-Crystal_Heart",
		"Grub-Kingdom's_Edge_Camp",
		"Grub-Deepnest_Spike",
		"Grub-Kingdom's_Edge_Oro",
		"Grub-Collector_2",
		"Grub-Beast's_Den",
		"Grub-Queen's_Gardens_Top",
		"Grub-Greenpath_Journal"
	};
	std::unordered_map<std::string, int> default_essence_rewards {
		std::make_pair("Whispering_Root-Kingdoms_Edge", 51),
		std::make_pair("Whispering_Root-Ancestral_Mound", 42),
		std::make_pair("Whispering_Root-Hive", 20),
		std::make_pair("Whispering_Root-City", 28),
		std::make_pair("Whispering_Root-Waterways", 35),
		std::make_pair("Whispering_Root-Crossroads", 29),
		std::make_pair("Whispering_Root-Greenpath", 44),
		std::make_pair("Whispering_Root-Leg_Eater", 20),
		std::make_pair("Whispering_Root-Spirits_Glade", 34),
		std::make_pair("Whispering_Root-Queens_Gardens", 29),
		std::make_pair("Whispering_Root-Resting_Grounds", 20),
		std::make_pair("Whispering_Root-Mantis_Village", 18),
		std::make_pair("Whispering_Root-Howling_Cliffs", 46),
		std::make_pair("Whispering_Root-Deepnest", 45),
		std::make_pair("Whispering_Root-Crystal_Peak", 21),
		std::make_pair("Elder_Hu", 100),
		std::make_pair("Galien", 200),
		std::make_pair("Gorb", 100),
		std::make_pair("Markoth", 250),
		std::make_pair("Marmu", 150),
		std::make_pair("No_Eyes", 200),
		std::make_pair("Xero", 100)
	};
	std::unordered_map<std::string, std::string> misc_charms {
		std::make_pair("Shaman_Stone", "Salubra"),
		std::make_pair("Quick_Slash", "Quick_Slash"),
		std::make_pair("Fragile_Strength", "Leg_Eater"),
		std::make_pair("Hiveblood", "Hiveblood")
	};
	std::unordered_set<std::string> ignored_macros {
		"MILDSKIPS",
		"FIREBALLSKIPS",
		"SHADESKIPS",
		"ACIDSKIPS",
		"SPIKETUNNELS",
		"SPICYSKIPS",
		"DARKROOMS",
		"CURSED",
		"NOTCURSED",
		"Focus",
		"GRUBCOUNT",
		"ESSENCECOUNT",
		"200ESSENCE"
	};

	std::ofstream LOGGER("log.txt");
	struct RaterSettings {
		bool ignore_bad_difficulty = false;
		char* path = nullptr;
	} RATER_SETTINGS;

	void SpaceToUnderscore(std::string& str) {
		for (int i = 0; i < str.length(); i++)
			if (str[i] == ' ')
				str[i] = '_';
	}

	std::unique_ptr<std::vector<std::string>> GetSpoilerLog() {
		const char* path_stem = getenv("USERPROFILE");
		if (path_stem == nullptr) {
			return nullptr;
		}
		std::ifstream spoiler_log(std::string(path_stem) + "\\AppData\\LocalLow\\Team Cherry\\Hollow Knight\\RandomizerSpoilerLog.txt");
		auto res = std::make_unique<std::vector<std::string>>();

		std::string cur_line = "";
		while (getline(spoiler_log, cur_line)) {
			res->push_back(cur_line);
		}

		spoiler_log.close();
		return res;
	}

	Item ParseRegularItem(std::string& log_line) {
		Item item;
		int i = 0;
		while (log_line.at(i) != ')') { i++; }
		i += 2;
		while (log_line.at(i) != '<') {
			char c = log_line.at(i++);
			item.name += c == ' ' ? '_' : c;
		}
		i += 10;
		while (i < log_line.length() && log_line.at(i) != '[') {
			char c = log_line.at(i++);
			item.location += c == ' ' ? '_' : c;
		}
		if (i < log_line.length()) {
			item.location = item.location.substr(0, item.location.length() - 1); //get rid of space between item and '[cost]'
			i++;
			while (log_line.at(i) != ' ') {
				item.cost = item.cost * 10 + (log_line.at(i++) - '0');
			}
			std::string cost_type_short = log_line.substr(i + 1, 2);
			if (cost_type_short == "Gr")
				item.cost_type = ItemCost::kGrub;
			else if (cost_type_short == "Es")
				item.cost_type = ItemCost::kEssence;
		}

		if (item.name.at(item.name.length() - 1) == ')') { //dupe item
			item.name = item.name.substr(0, item.name.length() - 4);
		}

		if (item.location == "King's_Idol-Glade_of_Hope") {
			item.cost_type = ItemCost::kEssence;
			item.cost = 200;
		}

		return item;
	}

	int AddProgression(std::vector<std::string>& spoiler_log, std::unordered_set<Item, ItemHasher>& item_locations, int progression_start = 0) {
		int i = progression_start;
		while (spoiler_log[i] != "PROGRESSION ITEMS") { i++; }
		i++;
		for (; spoiler_log[i] != "ALL ITEMS"; i++) {
			if (spoiler_log[i].length() == 0) continue;
			item_locations.insert(ParseRegularItem(spoiler_log[i]));
		}
		return i;
	}

	int AddMiscItems(std::vector<std::string>& spoiler_log, std::unordered_set<Item, ItemHasher>& item_locations, int all_items_start = 0) {
		int i = all_items_start;
		while (spoiler_log[i] != "ALL ITEMS") { i++; }
		i++;
		std::string cur_area = "";
		for (; spoiler_log[i] != "SETTINGS"; i++) {
			if (spoiler_log[i].length() == 0) continue;
			int colon_pos = spoiler_log[i].find(':');
			if (colon_pos != std::string::npos) {
				if (spoiler_log[i].at(0) == '(') {
					int j = 0;
					while (spoiler_log[i].at(j) != ')') { j++; }
					cur_area = spoiler_log[i].substr(j + 2, spoiler_log[i].length() - j - 3);
				} else {
					cur_area = spoiler_log[i].substr(0, spoiler_log[i].length() - 1);
				}
				SpaceToUnderscore(cur_area);
			} else {
				if (spoiler_log[i].at(0) == '(') {
					Item item = ParseRegularItem(spoiler_log[i]);
					if (misc_charms.count(item.name) || item.name.compare(0, 8, "Pale_Ore") == 0) {
						item_locations.insert(item);
					}
				} else {
					int j = 0;
					while (j < spoiler_log[i].length() && spoiler_log[i].at(j) != '[') { j++; }
					if (spoiler_log[i].at(j) != '[') {
						throw std::logic_error("Bad line : " + spoiler_log[i]);
					}
					std::string item_name = spoiler_log[i].substr(0, j - 1);
					SpaceToUnderscore(item_name);
					if (misc_charms.count(item_name)) {
						item_locations.insert(Item(item_name, cur_area));
					}
				}
			}
		}
		return i;
	}

	RandoSettings ParseSettings(std::vector<std::string>& spoiler_log, int settings_start = 0) {
		RandoSettings res;
		int i = settings_start;
		while (spoiler_log[i] != "SETTINGS") { i++; }
		i += 2;
		if (spoiler_log[i].substr(6) != "Item Randomizer") {
			throw std::logic_error("Modes other than Item Randomizer not supported");
		}
		i += 2;
		if (spoiler_log[i].length() < 16) {
			throw std::logic_error("Error parsing spoiler log (start location)");
		}
		try {
			res.start_location = start_location_lookup.at(spoiler_log[i].substr(16));
		} catch (std::out_of_range e) {
			throw std::logic_error("Unknown start location: " + spoiler_log[i].substr(16));
		}
		i += 11;
		for (; spoiler_log[i] != "QUALITY OF LIFE"; i++) {
			int j = spoiler_log[i].find(':');
			if (spoiler_log[i].compare(0, j, "Grubs") == 0) {
				res.randomized_grubs = spoiler_log[i].compare(j + 2, 4, "True") == 0;
			} else if (spoiler_log[i].compare(0, j, "Whispering roots") == 0) {
				res.randomized_roots = spoiler_log[i].compare(j + 2, 4, "True") == 0;
			}
		}
		return res;
	}

	auto ConvertXMLToLoadoutRating(pugi::xml_node root) {
		auto res = std::make_unique<std::unordered_map<std::string, std::unique_ptr<std::vector<LoadoutRating>>>>();
		for (auto location = root.first_child(); location; location = location.next_sibling()) {
			auto ratings = std::make_unique<std::vector<LoadoutRating>>();
			for (auto loadout = location.first_child(); loadout; loadout = loadout.next_sibling()) {
				auto split_logic = std::make_unique<std::vector<std::string>>();
				std::string logic = loadout.text().as_string();
				int symbol_start = 0;
				for (int i = 0; i < logic.length(); i++) {
					if (logic[i] == '+') {
						split_logic->push_back(logic.substr(symbol_start, i - symbol_start - 1));
						symbol_start = i + 2;
						i++;
					}
				}
				split_logic->push_back(logic.substr(symbol_start));
				int loadout_rating = loadout.attribute("difficulty").as_int();
				if (loadout_rating < 0) {
					if (RATER_SETTINGS.ignore_bad_difficulty) {
						loadout_rating = 0;
					} else {
						std::string err_location = "parsed.xml -> " + std::string(root.name()) + " -> " + location.attribute("name").as_string() + " -> loadout \"" + logic + "\"";
						throw std::logic_error("Difficulty must be non-negative: " + err_location);
					}
				}
				ratings->push_back(LoadoutRating(loadout_rating, std::move(split_logic)));
			}
			res->insert(std::make_pair(location.attribute("name").as_string(), std::move(ratings)));
		}
		return res;
	}

	auto BuildLookupTable(pugi::xml_document& parsed_logic_doc) {
		return std::make_pair(
			ConvertXMLToLoadoutRating(parsed_logic_doc.child("locations")),
			ConvertXMLToLoadoutRating(parsed_logic_doc.child("macros"))
		);
	}

	long long int EvaluateMacro(std::string macro,
		std::unordered_map<std::string, std::unique_ptr<std::vector<LoadoutRating>>>& macro_lookup,
		std::unordered_map<std::string, long long int>& acquired_items,
		std::unordered_map<std::string, long long int>& evaluated_items) {
		if (ignored_macros.count(macro)) {
			return 0;
		}

		auto iter = acquired_items.find(macro);
		if (iter != acquired_items.end()) {
			return iter->second;
		}

		iter = evaluated_items.find(macro);
		if (iter != evaluated_items.end()) {
			return iter->second;
		}

		if (!macro_lookup.count(macro)) { //unacquired item (or typo)
			return -1;
		}

		bool uncertain = false;
		evaluated_items.insert(std::make_pair(macro, -2));
		long long int macro_rating = kBigNumber;
		for (auto& rating : *(macro_lookup.at(macro))) {
			long long int loadout_rating = 0;
			for (auto& symbol : *(rating.loadout)) {
				try {
					long long int evaluation = EvaluateMacro(symbol, macro_lookup, acquired_items, evaluated_items);
					if (evaluation < 0) {
						if (evaluation == -2) {
							uncertain = true;
						}
						loadout_rating = kBigNumber; //prevents overriding macro_rating with invalid value
						break;
					}
					evaluated_items.insert(std::make_pair(symbol, evaluation));
					loadout_rating += evaluation;
				} catch (std::logic_error e) {
					std::cerr << e.what() << std::endl;
					throw std::logic_error("Invalid macro in " + macro);
				}
			}
			macro_rating = std::min(macro_rating, std::max(loadout_rating, kTensTable[rating.rating]));
		}

		if (macro_rating == kBigNumber) {
			macro_rating = -1;
		}

		if (macro_rating > -1) {
			acquired_items.insert(std::make_pair(macro, macro_rating));
		} else if (uncertain) {
			evaluated_items.erase(macro);
		} else {
			evaluated_items.at(macro) = -1;
		}

		return macro_rating;
	}

	long long int EvaluateLocation(std::string location,
		std::unordered_map<std::string, std::unique_ptr<std::vector<LoadoutRating>>>& location_lookup,
		std::unordered_map<std::string, std::unique_ptr<std::vector<LoadoutRating>>>& macro_lookup,
		std::unordered_map<std::string, long long int>& acquired_items,
		std::unordered_map<std::string, long long int>& evaluated_items) {
		if (DEBUG) {
			LOGGER << "Location " << location << std::endl;
			std::cout << "Location " << location << std::endl;
		}
		auto ratings = location_lookup.find(location);
		if (ratings == location_lookup.end()) {
			throw std::logic_error("Unknown location " + location);
		}
		long long int easiest_loadout_rating = kBigNumber;
		for (auto& rating : *(ratings->second)) {
			long long int cur_loadout_rating = 0;
			for (auto& symbol : *(rating.loadout)) {
				try {
					long long int symbol_rating = EvaluateMacro(symbol, macro_lookup, acquired_items, evaluated_items);
					if (symbol_rating < 0) {
						if (symbol_rating == -1) {
							evaluated_items.insert(std::make_pair(symbol, -1));
						}
						cur_loadout_rating = kBigNumber;
						break;
					}
					evaluated_items.insert(std::make_pair(symbol, symbol_rating));
					cur_loadout_rating += symbol_rating;
				} catch (std::logic_error e) {
					std::cerr << e.what() << std::endl;
					std::string loadout = "";
					for (auto& s : *(rating.loadout)) loadout += s + " ";
					throw std::logic_error("Invalid macro in loadout \"" + loadout + "\" for location " + location);
				}
			}
			easiest_loadout_rating = std::min(easiest_loadout_rating, std::max(cur_loadout_rating, kTensTable[rating.rating]));
		}
		return easiest_loadout_rating == kBigNumber ? -1 : easiest_loadout_rating;
	}

	long long int RateProgression(pugi::xml_document& ratings, std::unordered_set<Item, ItemHasher>& item_locations,
		std::unordered_set<std::string>& starting_items) {
		auto lookup_table = BuildLookupTable(ratings);
		std::unordered_map<std::string, long long int> symbol_cache {}, acquired_items {};
		int grub_count = 0, essence_count = 0;

		for (auto& item : starting_items) {
			acquired_items.insert(std::make_pair(item, 0));
		}

		int c = 0;
		long long int te_rating = -1;

		do {
			symbol_cache.clear();
			std::pair<long long int, Item> next_check = std::make_pair(kBigNumber, Item());
			for (auto& location : item_locations) {
				long long int rating =
					((location.cost_type == ItemCost::kGrub && grub_count < location.cost) ||
						(location.cost_type == ItemCost::kEssence && essence_count < location.cost)) ? -1 :
					EvaluateLocation(location.location, *(lookup_table.first), *(lookup_table.second), acquired_items, symbol_cache);
				if (rating < 0) {
					continue;
				} else if (rating < next_check.first) {
					next_check = std::make_pair(rating, location);
				}
			}
			std::string item_at_check = next_check.second.name;
			if (item_at_check.length() >= 4 && item_at_check.compare(0, 4, "Grub") == 0) {
				grub_count++;
			} else if (item_at_check.length() >= 15 && default_essence_rewards.count(item_at_check)) {
				essence_count += default_essence_rewards.at(item_at_check);
			}

			if (item_at_check == "Mothwing_Cloak" || item_at_check == "Shade_Cloak") {
				if (acquired_items.count("Mothwing_Cloak") == 0) {
					acquired_items.insert(std::make_pair("Mothwing_Cloak", next_check.first));
				} else if (acquired_items.count("Shade_Cloak") == 0) {
					acquired_items.insert(std::make_pair("Shade_Cloak", next_check.first));
				}
			} else if (item_at_check == "Vengeful_Spirit" || item_at_check == "Shade_Soul") {
				if (acquired_items.count("Vengeful_Spirit") == 0) {
					acquired_items.insert(std::make_pair("Vengeful_Spirit", next_check.first));
				} else if (acquired_items.count("Shade_Soul") == 0) {
					acquired_items.insert(std::make_pair("Shade_Soul", next_check.first));
				}
			} else if (item_at_check == "Desolate_Dive" || item_at_check == "Descending_Dark") {
				if (acquired_items.count("Desolate_Dive") == 0) {
					acquired_items.insert(std::make_pair("Desolate_Dive", next_check.first));
				} else if (acquired_items.count("Descending_Dark") == 0) {
					acquired_items.insert(std::make_pair("Descending_Dark", next_check.first));
				}
			} else if (item_at_check == "Howling_Wraiths" || item_at_check == "Abyss_Shriek") {
				if (acquired_items.count("Howling_Wraiths") == 0) {
					acquired_items.insert(std::make_pair("Howling_Wraiths", next_check.first));
				} else if (acquired_items.count("Abyss_Shriek") == 0) {
					acquired_items.insert(std::make_pair("Abyss_Shriek", next_check.first));
				}
			} else if (item_at_check == "Dream_Nail" || item_at_check == "Dream_Gate" || item_at_check == "Awoken_Dream_Nail") {
				if (acquired_items.count("Dream_Nail") == 0) {
					acquired_items.insert(std::make_pair("Dream_Nail", next_check.first));
				} else if (acquired_items.count("Dream_Gate") == 0) {
					acquired_items.insert(std::make_pair("Dream_Gate", next_check.first));
				} else if (acquired_items.count("Awoken_Dream_Nail") == 0) {
					acquired_items.insert(std::make_pair("Awoken_Dream_Nail", next_check.first));
				}
			} else if (item_at_check == "Queen_Fragment" || item_at_check == "King_Fragment" || item_at_check == "Void_Heart") {
				if (acquired_items.count("Queen_Fragment") == 0) {
					acquired_items.insert(std::make_pair("Queen_Fragment", next_check.first));
				} else if (acquired_items.count("King_Fragment") == 0) {
					acquired_items.insert(std::make_pair("King_Fragment", next_check.first));
				} else if (acquired_items.count("Void_Heart") == 0) {
					acquired_items.insert(std::make_pair("Void_Heart", next_check.first));
				}
			} else {
				acquired_items.insert(std::make_pair(item_at_check, next_check.first));
			}
			item_locations.erase(next_check.second);

			te_rating = EvaluateLocation("Radiance", *(lookup_table.first), *(lookup_table.second), acquired_items, symbol_cache);
		} while (!item_locations.empty() && te_rating == -1);

		return te_rating;
	}

	int main(int argc, char** argv) {

		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--ignore-bad-difficulty") == 0) {
				RATER_SETTINGS.ignore_bad_difficulty = true;
			}
		}

		auto spoiler_log = GetSpoilerLog();

		auto item_locations = std::make_unique<std::unordered_set<Item, ItemHasher>>();
		int all_items_begin = AddProgression(*spoiler_log, *item_locations);
		int settings_begin = AddMiscItems(*spoiler_log, *item_locations, all_items_begin);
		RandoSettings settings = ParseSettings(*spoiler_log, settings_begin);
		std::unordered_set<std::string> acquired_items { settings.start_location };
		if (!settings.randomized_grubs) {
			for (auto& grub : default_grub_locations) {
				item_locations->insert(Item(grub));
			}
		}
		for (auto& essence_reward : default_essence_rewards) {
			if (!settings.randomized_roots || essence_reward.first.length() < 15) { //only dream warriors if roots are randomized
				item_locations->insert(Item(essence_reward.first));
			}
		}

		pugi::xml_document ratings;
		if (!ratings.load_file("XML/parsed.xml")) {
			throw std::ios_base::failure("Unable to open parsed.xml");
		}

		long long results = -1;
		try {
			results = RateProgression(ratings, *item_locations, acquired_items);
		} catch (const std::exception& e) {
			std::cout << e.what() << std::endl;
			exit(1);
		}

		std::cout << "Seed rating: " << (results == 0 ? 0 : log10(results)) << " (raw rating: " << results << ")" << std::endl;

		return 0;
	}
}

int main(int argc, char** argv) {
	return RandoRater::main(argc, argv);
}