// C headers
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C++ headers
#include <algorithm>
using std::max;
using std::min;
using std::remove_if;
using std::swap;

#include <array>
using std::array;

#include <exception>
using std::exception;

#include <filesystem>
using std::filesystem::path;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <iomanip>
using std::setw;

#include <iostream>
using std::cout;

#include <iterator>
using std::istreambuf_iterator;

#include <ostream>
using std::ostream;

#include <set>
using std::set;

#include <stdexcept>
using std::runtime_error;

#include <string>
using std::string;
using std::to_string;

#include <unordered_map>
using std::unordered_map;

#include <unordered_set>
using std::unordered_set;

#include <vector>
using std::vector;

// debug
#ifdef NDEBUG
#define debug(a)
#else
#define debug(a) cout << __FILE__ << ':' << __LINE__ << ": " << __func__ << ": " << #a << ": " << a << '\n'
#endif

// a lot of output syntax uses comma separators
struct Separator {
	bool subsequent = 0;

	bool operator()() {
		auto a = subsequent;
		subsequent = 1;
		return a;
	}
};

// SORT
inline bool eq(const char* s, const char* t) {
	for (auto i = strlen(t); i--;)
		if (*s++ != *t++)
			return 0;
	return 1;
}

// input
string file;
string text;
vector<string> V;

void readText() {
	ifstream is(file, std::ios::in);
	text = {istreambuf_iterator<char>(is), istreambuf_iterator<char>()};

	// make sure input ends with a newline, to simplify parser code
	if (text.empty() || text.back() != '\n')
		text += '\n';
}

void readLines() {
	readText();
	auto s = text.data();
	V.clear();
	while (*s) {
		auto u = strchr(s, '\n');
		auto t = u;
		while (s < t && (t[-1] == ' ' || t[-1] == '\t' || t[-1] == '\r'))
			--t;
		V.push_back(string(s, t));
		s = u + 1;
	}
}

void pread(string cmd) {
	auto f = _popen(cmd.data(), "r");
	if (!f)
		throw runtime_error(cmd + ": " + strerror(errno));
	text.clear();
	for (;;) {
		auto c = fgetc(f);
		if (c < 0) {
			auto r = _pclose(f);
			if (r)
				throw runtime_error(cmd + ": " + to_string(r));
			return;
		}
		text += c;
	}
}

// output
void writeLines() {
	ofstream os(file, std::ios::binary);
	for (auto s: V)
		os << s << '\n';
}

// SORT
bool endsWith(string s, const char* t) {
	auto n = strlen(t);
	if (s.size() < n)
		return 0;
	return memcmp(s.data() + s.size() - n, t, n) == 0;
}

string esc(string s) {
	string o = "\"";
	for (auto c: s) {
		if (isprint((unsigned char)c)) {
			if (c == '"')
				o += '\\';
			o += c;
			continue;
		}
		char buf[7];
		sprintf(buf, "\\x%02x\"\"", (unsigned char)c);
		o += buf;
	}
	return o + '"';
}

int indent(int i) {
	// end of file is end of scope, so semantically a dedent
	if (i == V.size())
		return -1;

	auto s = V[i];

	// blank line does not meaningfully have an indent level
	if (s.empty())
		return INT_MAX;

	// in C++, nor does a preprocessor directive
	if (s[0] == '#')
		return INT_MAX;

	// assuming each file uses either tabs or spaces consistently
	int j = 0;
	while (s[j] == '\t' || s[j] == ' ')
		++j;
	return j;
}

bool startsWith(string s, const char* t) {
	auto n = strlen(t);
	if (s.size() < n)
		return 0;
	return memcmp(s.data(), t, n) == 0;
}

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char** argv) {
	try {
		if (argc < 2)
			return 1;
		file = argv[1];
		readLines();
		bool blockComment = 0;
#ifdef _WIN32
		auto console = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
		int i = 0;
		for (auto s: V) {
			++i;

			// skip blank lines
			if (s.empty())
				continue;

			// skip block comments
			// this is heuristic, not an exact parse
			// it will not work for all possible C++ code
			if (startsWith(s, "/*"))
				blockComment = 1;
			if (endsWith(s, "*/")) {
				blockComment = 0;
				continue;
			}
			if (blockComment)
				continue;

			// skip implementation details
			if (isspace(s[0]))
				continue;

			// skip trailing boilerplate
			if (s[0] == '}' && !startsWith(s, "} //"))
				continue;

			cout << setw(6) << i << "  ";
#ifdef _WIN32
			if (startsWith(s, "//"))
				SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			else if (startsWith(s, "#"))
				SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			else if (startsWith(s, "namespace ") || startsWith(s, "} // namespace"))
				SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			else if (startsWith(s, "class ") || startsWith(s, "struct "))
				SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			else
				SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif
			cout << s << '\n';
#ifdef _WIN32
			SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif
		}
		return 0;
	} catch (exception& e) {
		cout << e.what() << '\n';
		return 1;
	}
}
