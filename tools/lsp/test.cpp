//
// Created by jsans on 2/08/23.
//

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

using json = nlohmann::json;
using namespace std;

int main() {
    json j2 = {{"pi", 3.141},
               {"happy", true},
               {"name", "Niels"},
               {"token", "this-is-my-token"},
               {"ntoken", "1234567890"},
               {"nothing", nullptr},
               {"answer", {{"everything", 42}}},
               {"list", {1, 0, 2}},
               {"object", {{"currency", "USD"}, {"value", 42.99}}}};

    cout << j2 << endl;

    string_view sv;
    cout << j2["token"].dump() << endl;
    cout << j2["ntoken"] << endl;
    auto x = j2["ptoken"];
    cout << x << endl;
    cout << sv << endl;

}