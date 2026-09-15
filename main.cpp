#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <ctime>
using namespace std;
namespace fs = filesystem;

string readFile(const fs::path& path) {
    ifstream file(path, ios::binary);
    if (!file) throw runtime_error("Cannot read: " + path.string());
    ostringstream data;
    data << file.rdbuf();
    if (file.bad()) throw runtime_error("Read failed: " + path.string());
    return data.str();
}

void writeFile(const fs::path& path, const string& data) {
    ofstream file(path, ios::binary);
    if (!file) throw runtime_error("Cannot write: " + path.string());
    file << data;
    file.close();
    if (!file) throw runtime_error("Write failed: " + path.string());
}

string hashContent(const string& data) {
    uint64_t value = 14695981039346656037ULL;
    for (unsigned char byte : data) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    ostringstream result;
    result << hex << setfill('0') << setw(16) << value;
    return result.str();
}

string storeObject(const string& type, const string& content) {
    string data = type + "\n" + content;
    string id = hashContent(data);
    fs::path path = fs::path(".minigit/objects") / id;
    if (fs::exists(path)) {
        if (readFile(path) != data) throw runtime_error("Hash collision detected.");
    } else {
        writeFile(path, data);
    }
    return id;
}

map<string, string> readIndex() {
    map<string, string> entries;
    istringstream input(readFile(".minigit/index"));
    string name, id;
    while (input >> quoted(name) >> id) entries[name] = id;
    return entries;
}

string serializeIndex(const map<string, string>& entries) {
    ostringstream output;
    for (const auto& entry : entries) {
        output << quoted(entry.first) << ' ' << entry.second << '\n';
    }
    return output.str();
}

void initialize() {
    if (fs::exists(".minigit")) {
        cout << "Repository already exists.\n";
        return;
    }
    fs::create_directories(".minigit/objects");
    writeFile(".minigit/index", "");
    writeFile(".minigit/HEAD", "");
    cout << "Initialized .minigit\n";
}

void addFile(const string& name) {
    fs::path path(name);
    if (name.empty() || path.has_parent_path() || name == ".minigit" || name == ".git") {
        throw runtime_error("Use a filename in this folder, such as notes.txt.");
    }
    if (name.find_first_of("\r\n") != string::npos) throw runtime_error("Newlines are not allowed in filenames.");
    if (fs::is_symlink(path) || !fs::is_regular_file(path)) throw runtime_error("Expected a regular file.");
    auto entries = readIndex();
    entries[name] = storeObject("blob", readFile(path));
    writeFile(".minigit/index", serializeIndex(entries));
    cout << "Staged " << name << '\n';
}

void commit(const string& message) {
    if (message.empty()) throw runtime_error("Commit message cannot be empty.");
    auto entries = readIndex();
    if (entries.empty()) throw runtime_error("Nothing staged. Use add first.");
    string tree = storeObject("tree", serializeIndex(entries));
    string parent = readFile(".minigit/HEAD");
    ostringstream content;
    content << tree << '\n' << parent << '\n' << time(nullptr) << '\n' << message;
    string id = storeObject("commit", content.str());
    writeFile(".minigit/HEAD", id);
    cout << "Committed " << id << '\n';
}

void showLog() {
    string id = readFile(".minigit/HEAD");
    if (id.empty()) cout << "No commits yet.\n";
    while (!id.empty()) {
        if (id.size() != 16 || id.find_first_not_of("0123456789abcdef") != string::npos) throw runtime_error("Invalid commit ID.");
        string data = readFile(fs::path(".minigit/objects") / id);
        if (hashContent(data) != id) throw runtime_error("Damaged commit object.");
        istringstream input(data);
        string type, tree, parent, timestamp, message;
        getline(input, type);
        getline(input, tree);
        getline(input, parent);
        getline(input, timestamp);
        getline(input, message, '\0');
        if (type != "commit") throw runtime_error("Expected a commit object.");
        cout << "commit " << id << "\nTree: " << tree << "\nTime: " << timestamp << "\nMessage: " << message << "\n\n";
        id = parent;
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) throw runtime_error("Usage: minigit init | add FILE | commit MESSAGE | log");
        string command = argv[1];
        if (command == "init" && argc == 2) {
            initialize();
        } else {
            if (!fs::is_directory(".minigit/objects")) throw runtime_error("Run minigit init first.");
            if (command == "add" && argc == 3) addFile(argv[2]);
            else if (command == "commit" && argc == 3) commit(argv[2]);
            else if (command == "log" && argc == 2) showLog();
            else throw runtime_error("Usage: minigit init | add FILE | commit MESSAGE | log");
        }
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
