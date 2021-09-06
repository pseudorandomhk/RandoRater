#include <iostream>
#include <memory>
#include <vector>
#include <string_view>
#include <unordered_set>
#include <algorithm>

#include "pugixml.hpp"

using namespace std;

const string_view kPlusStringView = string_view("+"), kPipeStringView = string_view("|");

struct LogicNode {
	LogicNode* left;
	LogicNode* right;
	const string_view val;
	LogicNode(string_view v) : left(nullptr), right(nullptr), val(v) {}
	LogicNode(LogicNode* l, LogicNode* r, string_view v) : left(l), right(r), val(v) {}
	~LogicNode() { delete left; delete right; }

	void print() {
		if (left == nullptr && right == nullptr) {
			cout << string(val) << ";" << endl;
			return;
		}
		cout << string(val) << ": " << string(left->val) << " , " << string(right->val) << endl;
		left->print();
		right->print();
	}
};

LogicNode* MakeLogicTree(string_view logic) {
	vector<string_view> tokens;
	vector<char> operators;
	int expr_start = 0, parenthesis_balance = 0;
	for (int i = 0; i < logic.length(); i++) {
		if ((logic.at(i) == '+' || logic.at(i) == '|') && parenthesis_balance == 0) {
			tokens.push_back(logic.substr(expr_start, i - expr_start - 1));
			operators.push_back(logic.at(i));
			expr_start = i + 2;
			i++;
			continue;
		} else if (logic.at(i) == '(') {
			parenthesis_balance++;
		} else if (logic.at(i) == ')') {
			parenthesis_balance--;
		}
	}
	tokens.push_back(logic.substr(expr_start, logic.length() - expr_start));
	if (operators.empty()) {
		if (tokens[0].at(0) == '(') { //remove unnecessary parenthesis, e.g "(((ITEM)))"
			return MakeLogicTree(tokens[0].substr(1, tokens[0].length() - 2));
		} else {
			return new LogicNode(tokens[0]);
		}
	}
	vector<LogicNode*> tokens_as_trees;
	for (auto& token : tokens) {
		if (token.at(0) == '(') {
			tokens_as_trees.push_back(MakeLogicTree(token.substr(1, token.length() - 2)));
		} else {
			tokens_as_trees.push_back(MakeLogicTree(token));
		}
	}

	for (int i = operators.size() - 1; i >= 0; i--) {
		if (operators[i] == '+') {
			LogicNode* new_parent = new LogicNode(tokens_as_trees[i], tokens_as_trees[i + 1], kPlusStringView);
			tokens_as_trees.erase(tokens_as_trees.begin() + i + 1);
			tokens_as_trees[i] = new_parent;
			operators.erase(operators.begin() + i);
		}
	}

	for (int i = operators.size() - 1; i >= 0; i--) {
		LogicNode* new_parent = new LogicNode(tokens_as_trees[i], tokens_as_trees[i + 1], kPipeStringView);
		tokens_as_trees[i] = new_parent;
		tokens_as_trees.pop_back();
	}

	return *&tokens_as_trees[0];
}

unique_ptr<vector<string>> SplitLogicStatement(LogicNode* logic_tree) {
	if (logic_tree->left == nullptr && logic_tree->right == nullptr) {
		auto res = make_unique<vector<string>>();
		res->push_back(string(logic_tree->val));
		return res;
	} else if (logic_tree->val.at(0) == '+') {
		auto res = make_unique<vector<string>>();
		auto left_statements = *SplitLogicStatement(logic_tree->left), right_statements = *SplitLogicStatement(logic_tree->right);
		for (auto& left_statement : left_statements) {
			for (auto& right_statement : right_statements) {
				res->push_back(left_statement + " + " + right_statement);
			}
		}
		return res;
	} else if (logic_tree->val.at(0) == '|') {
		auto res = make_unique<vector<string>>(*SplitLogicStatement(logic_tree->left));
		auto right = *SplitLogicStatement(logic_tree->right);
		res->insert(res->end(), right.begin(), right.end());
		return res;
	} else {
		throw logic_error("Improperly constructed expression tree. Likely cause is malformed logic");
	}
}
unique_ptr<vector<string>> SplitLogicStatement(const string& logic) {
	if (logic.empty()) {
		return make_unique<vector<string>>(vector<string>{""});
	}
	LogicNode* logic_tree = MakeLogicTree(string_view(logic));
	auto res = SplitLogicStatement(logic_tree);
	delete logic_tree;
	//res->erase(remove_if(res->begin(), res->end(), [](string& s) { return s.length() == 0; }), res->end());
	return res;
}

void AppendParsedLogic(const string& item_name, const string& logic, const string& xml_node_name, pugi::xml_node& parent) {
	pugi::xml_node output = parent.append_child(xml_node_name.c_str());
	output.append_attribute("name").set_value(item_name.c_str());
	auto loadouts = *SplitLogicStatement(logic);
	for (auto& loadout : loadouts) {
		pugi::xml_node loadout_container = output.append_child("loadout");
		loadout_container.append_attribute("difficulty").set_value(0);
		loadout_container.text().set(loadout.c_str());
	}
}

auto CreateParsedLogic() {
	pugi::xml_document logic_xml;
	auto parsed_logic_doc = make_unique<pugi::xml_document>();

	pugi::xml_node locs = parsed_logic_doc->append_child("locations"), macros = parsed_logic_doc->append_child("macros");

	string xml_files[] = { "items", "rocks", "shops", "soul_lore" };
	for (string stem : xml_files) {
		if (logic_xml.load_file(("XML/" + stem + ".xml").c_str())) {
			for (pugi::xml_node item = logic_xml.child("randomizer").first_child(); item; item = item.next_sibling()) {
				AppendParsedLogic(
					item.attribute("name").as_string(),
					item.child("itemLogic").text().as_string(),
					"location",
					locs
				);
			}
		} else {
			throw ios_base::failure("Unable to read file " + stem + ".xml");
		}
	}

	if (logic_xml.load_file("XML/waypoints.xml")) {
		for (pugi::xml_node waypoint = logic_xml.child("randomizer").first_child(); waypoint; waypoint = waypoint.next_sibling()) {
			AppendParsedLogic(
				waypoint.attribute("name").as_string(),
				waypoint.child("itemLogic").text().as_string(),
				"macro",
				macros
			);
		}
	}

	if (logic_xml.load_file("XML/macros.xml")) {
		for (pugi::xml_node macro = logic_xml.child("randomizer").first_child(); macro && string(macro.attribute("name").as_string()) != "CANSTAG-R"; macro = macro.next_sibling()) {
			AppendParsedLogic(
				macro.attribute("name").as_string(),
				macro.text().as_string(),
				"macro",
				macros
			);
		}
	} else {
		throw ios_base::failure("Unable to read macros.xml");
	}

	AppendParsedLogic("Radiance", "DREAMER3 + (CLAW | WINGS) + (SHADOWDASH | QUAKE)", "location", locs);

	AppendParsedLogic("Nailsmith", "Left_City + (MILDSKIPS | DASH | WINGS | CLAW)", "macro", macros);

	AppendParsedLogic("NAIL1", "Nailsmith", "macro", macros);
	AppendParsedLogic("NAIL2", "Nailsmith + (Pale_Ore-Grubs | Pale_Ore-Seer | Pale_Ore-Basin | Pale_Ore-Crystal_Peak | Pale_Ore-Nosk | Pale_Ore-Colosseum)", "macro", macros);
	AppendParsedLogic("NAIL3", "Nailsmith + ((Pale_Ore-Grubs + Pale_Ore-Seer + Pale_Ore-Basin) | (Pale_Ore-Grubs + Pale_Ore-Seer + Pale_Ore-Crystal_Peak) | (Pale_Ore-Grubs + Pale_Ore-Seer + Pale_Ore-Nosk) | (Pale_Ore-Grubs + Pale_Ore-Seer + Pale_Ore-Colosseum) | (Pale_Ore-Grubs + Pale_Ore-Basin + Pale_Ore-Crystal_Peak) | (Pale_Ore-Grubs + Pale_Ore-Basin + Pale_Ore-Nosk) | (Pale_Ore-Grubs + Pale_Ore-Basin + Pale_Ore-Colosseum) | (Pale_Ore-Grubs + Pale_Ore-Crystal_Peak + Pale_Ore-Nosk) | (Pale_Ore-Grubs + Pale_Ore-Crystal_Peak + Pale_Ore-Colosseum) | (Pale_Ore-Grubs + Pale_Ore-Nosk + Pale_Ore-Colosseum) | (Pale_Ore-Seer + Pale_Ore-Basin + Pale_Ore-Crystal_Peak) | (Pale_Ore-Seer + Pale_Ore-Basin + Pale_Ore-Nosk) | (Pale_Ore-Seer + Pale_Ore-Basin + Pale_Ore-Colosseum) | (Pale_Ore-Seer + Pale_Ore-Crystal_Peak + Pale_Ore-Nosk) | (Pale_Ore-Seer + Pale_Ore-Crystal_Peak + Pale_Ore-Colosseum) | (Pale_Ore-Seer + Pale_Ore-Nosk + Pale_Ore-Colosseum) | (Pale_Ore-Basin + Pale_Ore-Crystal_Peak + Pale_Ore-Nosk) | (Pale_Ore-Basin + Pale_Ore-Crystal_Peak + Pale_Ore-Colosseum) | (Pale_Ore-Basin + Pale_Ore-Nosk + Pale_Ore-Colosseum) | (Pale_Ore-Crystal_Peak + Pale_Ore-Nosk + Pale_Ore-Colosseum))", "macro", macros);
	AppendParsedLogic("NAIL4", "Nailsmith + (Pale_Ore-Grubs + Pale_Ore-Seer + Pale_Ore-Basin + Pale_Ore-Crystal_Peak + Pale_Ore-Nosk + Pale_Ore-Colosseum)", "macro", macros);

	AppendParsedLogic("DASH", "Mothwing_Cloak", "macro", macros);
	AppendParsedLogic("CLAW", "Mantis_Claw", "macro", macros);
	AppendParsedLogic("DREAMNAIL", "Dream_Nail", "macro", macros);
	AppendParsedLogic("SUPERDASH", "Crystal_Heart", "macro", macros);
	AppendParsedLogic("ACID", "Isma's_Tear", "macro", macros);
	AppendParsedLogic("WINGS", "Monarch_Wings", "macro", macros);
	AppendParsedLogic("FIREBALL", "Vengeful_Spirit", "macro", macros);
	AppendParsedLogic("QUAKE", "Desolate_Dive", "macro", macros);
	AppendParsedLogic("SCREAM", "Howling_Wraiths", "macro", macros);

	return parsed_logic_doc;
}



int main() {
	CreateParsedLogic();

	return 0;
}