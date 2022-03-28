#include "global.h"

bool match(std::string& str, std::string form, std::string message)
{
	std::regex f(form);
	if (!std::regex_match(str, f)) {
		std::cout << message << std::endl;
		return false;
	}
	return true;
}

void input_and_match(std::string& str, std::string form, std::string message) {
	fflush(stdin);
	std::cin >> str;
	while (!match(str, form, message)) {
		std::cout << "ÇëÖØÐÂÊäÈë>";
		std::cin >> str;
	}
}