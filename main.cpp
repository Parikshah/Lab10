#include <iostream>
#include <ctype.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <string>
#include <math.h>
#include <map>

using namespace std;

const int HASH_PRIME = 37;
const float MAX_LOAD = 0.7;

class String_HT {
public:
    String_HT(int);
    virtual void print() = 0;

    virtual void insert(string) = 0;
    virtual void remove(string) = 0;
    virtual bool find(string, int &) = 0;
    virtual void expand_and_rehash() = 0;

    int hash_index(string);
    int least_prime_greater_than_or_equal_to(int);

    int table_size;
    int num_entries;
    float load;
};

class Chain_String_HT : public String_HT {
public:
    Chain_String_HT(int);
    void print();

    void insert(string);
    void remove(string);
    bool find(string, int &);
    void expand_and_rehash();

    vector<vector<string>> table;
};

class Probe_String_HT : public String_HT {
public:
    Probe_String_HT(int);
    void print();

    void insert(string);
    void remove(string);
    bool find(string, int &);
    void expand_and_rehash();

    vector<string> table;
    const string DELETED = "<DELETED>";
};

// Implementation of String_HT
String_HT::String_HT(int num_elements) {
    table_size = least_prime_greater_than_or_equal_to(num_elements);
    num_entries = 0;
    load = 0.0;
}

int String_HT::least_prime_greater_than_or_equal_to(int n) {
    int i, sqrt_n, p;

    sqrt_n = (int) floor(sqrt(n));
    for (p = n; p < 2 * n; p++) {
        for (i = 2; i <= sqrt_n; i++) 
            if ((p % i) == 0) break;
        if (i == sqrt_n + 1) return p;
    }
    return -1;
}

int String_HT::hash_index(string key) {
    int value = 0;
    for (int i = 0; i < key.size(); i++)
        value = HASH_PRIME * value + key[i];
    value %= table_size;
    if (value < 0) value += table_size;
    return value;
}

// Implementation of Chain_String_HT
Chain_String_HT::Chain_String_HT(int num_elements) : String_HT(num_elements) {
    table.resize(table_size);
    for (int i = 0; i < table.size(); i++) 
        table[i].clear();
}

void Chain_String_HT::print() {
    printf("load = %.3f\n", load);
    for (int i = 0; i < table.size(); i++) {
        cout << i << " (" << table[i].size() << "): ";
        for (int j = 0; j < table[i].size(); j++)
            cout << table[i][j] << " ";
        cout << endl;
    }
}

void Chain_String_HT::insert(string key) {
    int index = hash_index(key);
    for (const auto &word : table[index]) {
        if (word == key) return;
    }
    table[index].push_back(key);
    num_entries++;
    load = static_cast<float>(num_entries) / table_size;

    if (load >= MAX_LOAD) expand_and_rehash();
}

void Chain_String_HT::remove(string key) {
    int index = hash_index(key);
    auto &chain = table[index];
    for (auto it = chain.begin(); it != chain.end(); ++it) {
        if (*it == key) {
            chain.erase(it);
            num_entries--;
            load = static_cast<float>(num_entries) / table_size;
            return;
        }
    }
}

bool Chain_String_HT::find(string key, int &num_collisions) {
    int index = hash_index(key);
    num_collisions = 0;
    for (const auto &word : table[index]) {
        num_collisions++;
        if (word == key) return true;
    }
    return false;
}

void Chain_String_HT::expand_and_rehash() {
    int new_table_size = least_prime_greater_than_or_equal_to(2 * table_size);
    vector<vector<string>> new_table(new_table_size);

    for (const auto &chain : table) {
        for (const auto &key : chain) {
            int new_index = hash_index(key) % new_table_size;
            new_table[new_index].push_back(key);
        }
    }

    table = std::move(new_table);
    table_size = new_table_size;
    load = static_cast<float>(num_entries) / table_size;
}

// Implementation of Probe_String_HT
Probe_String_HT::Probe_String_HT(int num_elements) : String_HT(num_elements) {
    table.resize(table_size, "");
}

void Probe_String_HT::print() {
    printf("load = %.3f\n", load);
    for (int i = 0; i < table.size(); i++) {
        cout << i << ": " << (table[i].empty() ? "EMPTY" : table[i]) << endl;
    }
}

void Probe_String_HT::insert(string key) {
    int index = hash_index(key);
    int i = 1;

    while (!table[index].empty() && table[index] != DELETED && table[index] != key) {
        index = (index + i * i * ((i % 2 == 0) ? -1 : 1)) % table_size;
        if (index < 0) index += table_size;
        i++;
    }

    if (table[index] != key) {
        table[index] = key;
        num_entries++;
        load = static_cast<float>(num_entries) / table_size;

        if (load >= MAX_LOAD) expand_and_rehash();
    }
}

void Probe_String_HT::remove(string key) {
    int index = hash_index(key);
    int i = 1;

    while (!table[index].empty()) {
        if (table[index] == key) {
            table[index] = DELETED;
            num_entries--;
            return;
        }
        index = (index + i * i * ((i % 2 == 0) ? -1 : 1)) % table_size;
        if (index < 0) index += table_size;
        i++;
    }
}

bool Probe_String_HT::find(string key, int &num_collisions) {
    int index = hash_index(key);
    int i = 1;
    num_collisions = 0;

    while (!table[index].empty()) {
        num_collisions++;
        if (table[index] == key) return true;
        index = (index + i * i * ((i % 2 == 0) ? -1 : 1)) % table_size;
        if (index < 0) index += table_size;
        i++;
    }
    return false;
}

void Probe_String_HT::expand_and_rehash() {
    int new_table_size = least_prime_greater_than_or_equal_to(2 * table_size);
    vector<string> new_table(new_table_size, "");

    for (const auto &key : table) {
        if (!key.empty() && key != DELETED) {
            int index = hash_index(key) % new_table_size;
            int i = 1;

            while (!new_table[index].empty()) {
                index = (index + i * i * ((i % 2 == 0) ? -1 : 1)) % new_table_size;
                if (index < 0) index += new_table_size;
                i++;
            }
            new_table[index] = key;
        }
    }

    table = std::move(new_table);
    table_size = new_table_size;
    load = static_cast<float>(num_entries) / table_size;
}

//----------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------

// make string all lower-case, remove leading/trailing punctuation, etc.
// also remove possessive trailing "'s"

void cleanup_string(string &word) {
    for (auto &ch : word)
        ch = tolower(ch);
    word.erase(remove_if(word.begin(), word.end(), [](char c) { return !isalpha(c); }), word.end());
}

void clean_and_insert_all_words(String_HT &hash_table, const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    string word;
    while (file >> word) {
        cleanup_string(word);
        if (!word.empty()) {
            hash_table.insert(word);
        }
    }
    file.close();
}

void process_spellcheck(String_HT &hash_table, const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    string word;
    map<string, int> badwords;

    while (file >> word) {
        cleanup_string(word);
        if (!word.empty()) {
            int num_collisions = 0;
            if (!hash_table.find(word, num_collisions)) {
                badwords[word] = num_collisions;
            }
        }
    }

    file.close();

    cout << badwords.size() << endl;
    for (const auto &entry : badwords) {
        cout << entry.first << " " << entry.second << endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <command filename>" << endl;
        return 1;
    }

    ifstream command_file(argv[1]);
    if (!command_file.is_open()) {
        cerr << "Error: Could not open file " << argv[1] << endl;
        return 1;
    }

    string line;
    String_HT *hash_table = nullptr;

    while (getline(command_file, line)) {
        istringstream iss(line);
        string command, arg1, arg2;
        iss >> command >> arg1;

        if (command == "CHAIN" || command == "PROBE") {
            if (hash_table) {
                delete hash_table;
                hash_table = nullptr;
            }

            ifstream dict_file(arg1);
            if (!dict_file.is_open()) {
                cerr << "Error: Could not open dictionary file " << arg1 << endl;
                return 1;
            }

            int word_count = 0;
            string word;
            while (dict_file >> word) {
                cleanup_string(word);
                if (!word.empty()) {
                    word_count++;
                }
            }
            dict_file.close();

            if (command == "CHAIN") {
                hash_table = new Chain_String_HT(word_count);
            } else if (command == "PROBE") {
                hash_table = new Probe_String_HT(word_count);
            }

            clean_and_insert_all_words(*hash_table, arg1);
            cout << hash_table->num_entries << " " << hash_table->table_size << endl;
        } else if (command == "INSERT") {
            if (hash_table) {
                hash_table->insert(arg1);
                cout << hash_table->num_entries << " " << hash_table->table_size << endl;
            }
        } else if (command == "REMOVE") {
            if (hash_table) {
                hash_table->remove(arg1);
                cout << hash_table->num_entries << " " << hash_table->table_size << endl;
            }
        } else if (command == "FIND") {
            if (hash_table) {
                int num_collisions = 0;
                if (!hash_table->find(arg1, num_collisions)) {
                    cout << arg1 << " " << num_collisions << endl;
                }
            }
        } else if (command == "SPELLCHECK") {
            if (hash_table) {
                process_spellcheck(*hash_table, arg1);
            }
        }
    }

    delete hash_table;
    command_file.close();

    return 0;
}
