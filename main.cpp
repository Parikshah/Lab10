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

//----------------------------------------------------------------------------

const int HASH_PRIME = 37;
const float MAX_LOAD = 0.7;

//----------------------------------------------------------------------------

class String_HT
{
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

//----------------------------------------------------------------------------

class Chain_String_HT : public String_HT
{
public:
    Chain_String_HT(int);
    void print();

    void insert(string);
    void remove(string);
    bool find(string, int &);
    void expand_and_rehash();

    vector<vector<string>> table;
};

//----------------------------------------------------------------------------

class Probe_String_HT : public String_HT
{
public:
    Probe_String_HT(int);
    void print();

    void insert(string);
    void remove(string);
    bool find(string, int &);
    void expand_and_rehash();

    vector<string> table;
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

String_HT::String_HT(int num_elements)
{
    table_size = least_prime_greater_than_or_equal_to(num_elements);
    num_entries = 0;
    load = 0.0;
}

//----------------------------------------------------------------------------

int String_HT::least_prime_greater_than_or_equal_to(int n)
{
    for (int p = n;; p++)
    {
        bool is_prime = true;
        for (int i = 2; i <= sqrt(p); i++)
        {
            if (p % i == 0)
            {
                is_prime = false;
                break;
            }
        }
        if (is_prime)
            return p;
    }
    return -1; // Unreachable
}

//----------------------------------------------------------------------------

int String_HT::hash_index(string key)
{
    int value = 0;
    for (int i = 0; i < key.size(); i++)
        value = HASH_PRIME * value + key[i];

    value %= table_size;
    if (value < 0)
        value += table_size;

    return value;
}

//----------------------------------------------------------------------------

Chain_String_HT::Chain_String_HT(int num_elements) : String_HT(num_elements)
{
    table.resize(table_size);
}

void Chain_String_HT::print()
{
    printf("load = %.3f\n", load);
    for (int i = 0; i < table.size(); i++)
    {
        cout << i << " (" << table[i].size() << "): ";
        for (const auto &str : table[i])
            cout << str << " ";
        cout << endl;
    }
}

void Chain_String_HT::insert(string key)
{
    int index = hash_index(key);
    for (const auto &str : table[index])
        if (str == key)
            return; // Key already exists

    table[index].push_back(key);
    num_entries++;
    load = (float)num_entries / table_size;

    if (load > MAX_LOAD)
        expand_and_rehash();
}

void Chain_String_HT::remove(string key)
{
    int index = hash_index(key);
    for (auto it = table[index].begin(); it != table[index].end(); ++it)
    {
        if (*it == key)
        {
            table[index].erase(it);
            num_entries--;
            load = (float)num_entries / table_size;
            return;
        }
    }
}

bool Chain_String_HT::find(string key, int &num_collisions)
{
    int index = hash_index(key);
    num_collisions = 0;

    for (const auto &str : table[index])
    {
        num_collisions++;
        if (str == key)
            return true;
    }
    return false;
}

void Chain_String_HT::expand_and_rehash()
{
    int new_table_size = least_prime_greater_than_or_equal_to(2 * table_size);
    vector<vector<string>> new_table(new_table_size);

    for (const auto &chain : table)
    {
        for (const auto &key : chain)
        {
            int new_index = HASH_PRIME * hash_index(key) % new_table_size;
            new_table[new_index].push_back(key);
        }
    }

    table = move(new_table);
    table_size = new_table_size;
    load = (float)num_entries / table_size;
}

//----------------------------------------------------------------------------

Probe_String_HT::Probe_String_HT(int num_elements) : String_HT(num_elements)
{
    table.resize(table_size, "");
}

void Probe_String_HT::print()
{
    for (int i = 0; i < table.size(); i++)
    {
        cout << i << ": " << table[i] << endl;
    }
}

void Probe_String_HT::insert(string key)
{
    int index = hash_index(key);
    int offset = 1;
    while (!table[index].empty())
    {
        if (table[index] == key)
            return; // Key already exists

        index = (index + offset * offset) % table_size;
        offset++;
    }

    table[index] = key;
    num_entries++;
    load = (float)num_entries / table_size;

    if (load > MAX_LOAD)
        expand_and_rehash();
}

void Probe_String_HT::remove(string key)
{
    int index = hash_index(key);
    int offset = 1;

    while (!table[index].empty())
    {
        if (table[index] == key)
        {
            table[index] = ""; // Lazy deletion
            num_entries--;
            load = (float)num_entries / table_size;
            return;
        }

        index = (index + offset * offset) % table_size;
        offset++;
    }
}

bool Probe_String_HT::find(string key, int &num_collisions)
{
    int index = hash_index(key);
    int offset = 1;
    num_collisions = 0;

    while (!table[index].empty())
    {
        num_collisions++;
        if (table[index] == key)
            return true;

        index = (index + offset * offset) % table_size;
        offset++;
    }
    return false;
}

void Probe_String_HT::expand_and_rehash()
{
    int new_table_size = least_prime_greater_than_or_equal_to(2 * table_size);
    vector<string> new_table(new_table_size, "");

    for (const auto &key : table)
    {
        if (!key.empty())
        {
            int index = hash_index(key);
            int offset = 1;
            while (!new_table[index].empty())
            {
                index = (index + offset * offset) % new_table_size;
                offset++;
            }
            new_table[index] = key;
        }
    }

    table = move(new_table);
    table_size = new_table_size;
    load = (float)num_entries / table_size;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------

// make string all lower-case, remove leading/trailing punctuation, etc.
// also remove possessive trailing "'s"

void cleanup_string(string & s)
{
  int i;

  while (s.size() > 0 && ispunct(s[0]))
    s.erase(0, 1);

  while (s.size() > 0 && ispunct(s[s.size() - 1]))
    s.erase(s.size() - 1, 1);

  for (i = 0; i < s.size(); i++)
    if (isalpha(s[i]))
      s[i] = tolower(s[i]);

  // possessive checker
  
  if (s[s.length() - 2] == '\'' && s[s.length() - 1] == 's') 
    s = s.substr(0, s.length() - 2);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// count words in file

int get_word_count(string filename)
{
  // try to open file 
  
  ifstream inStream;
  string s;
  
  inStream.open(filename.c_str());

  if (inStream.fail()) {
    cout << "Failed to open file\n";
    exit(1);
  }

  // count words

  int word_count = 0;
  
  while (!inStream.eof()) {
    
    inStream >> s;

    if (!inStream.eof() && s.size())
      word_count++;
  }

  inStream.close();

  return word_count;
}

//----------------------------------------------------------------------------

// "clean" words in filename before inserting into hash table

void clean_and_insert_all_words(string filename, String_HT *HT)
{
  // try to open file 

  ifstream inStream;
  string s;

  inStream.open(filename.c_str());

  if (inStream.fail()) {
    cout << "Failed to open file\n";
    exit(1);
  }
  
  while (!inStream.eof()) {

    inStream >> s;

    if (!inStream.eof()) {

      cleanup_string(s); 

      if (s.size())
	HT->insert(s);
    }
  }

  inStream.close();
}

//----------------------------------------------------------------------------

// "clean" each word in filename before seeing if it is in hash table.
// if not, put cleaned word into badwords map along with num_collisions

void spellcheck_all_cleaned_words(string filename, String_HT *HT, map<string, int> & badwords)
{
  // try to open file 

  ifstream inStream;
  string s;
  int num_collisions;
  
  inStream.open(filename.c_str());

  if (inStream.fail()) {
    cout << "Failed to open file\n";
    exit(1);
  }
  
  while (!inStream.eof()) {

    inStream >> s;

    if (!inStream.eof()) {

      cleanup_string(s); 
      
      if (s.size() && !HT->find(s, num_collisions))  
	badwords.insert(make_pair(s, num_collisions));
    }
  }

  inStream.close();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  if (argc < 2) {
    cout << "hashcheck <command filename>\n";
    exit(1);
  }

  // open command file

  ifstream commandStream;

  commandStream.open(argv[1]);

  if (commandStream.fail()) {
    cout << "Failed to open command file\n";
    exit(1);
  }

  // iterate through file line by line assuming no syntax errors
  
  string line, s_command, s_arg;
  int word_count, num_collisions;
  String_HT *HT;
  
  while (getline(commandStream, line)) {

    stringstream ls(line);

    ls >> s_command;
    ls >> s_arg;

    // create hash table "dictionary" using separate chaining for collision resolution
    
    if (s_command == "CHAIN") {
      word_count = get_word_count(s_arg);
      HT = new Chain_String_HT(2 * word_count);
      clean_and_insert_all_words(s_arg, HT);
      cout << word_count << " " << HT->table_size << endl;
    }

    // create hash table "dictionary" using quadratic probing for collision resolution

    else if (s_command == "PROBE") {
      word_count = get_word_count(s_arg);
      HT = new Probe_String_HT(2 * word_count);
      clean_and_insert_all_words(s_arg, HT);
      cout << word_count << " " << HT->table_size << endl;
    }

    // insert word into dictionary
    
    else if (s_command == "INSERT") {
      HT->insert(s_arg);
      cout << HT->num_entries << " " << HT->table_size << endl;
    }

    // remove word from dictionary

    else if (s_command == "REMOVE") {
      HT->remove(s_arg);
      cout << HT->num_entries << " " << HT->table_size << endl;
    }

    // check if word is NOT in dictionary

    else if (s_command == "FIND") {
      if (!HT->find(s_arg, num_collisions))
	cout << s_arg << " " << num_collisions << endl;
    }

    // see which words in file are NOT in dictionary

    else if (s_command == "SPELLCHECK") {
      map<string,int> badwords;
      spellcheck_all_cleaned_words(s_arg, HT, badwords);
      cout << badwords.size() << endl;
      for (auto itr : badwords) 
	cout << itr.first << " " << itr.second << endl;
    }

    // garbage
    
    else {
      cout << "Unknown command " << s_command << endl;
      exit(1);
    }
  }
  
  commandStream.close();

  return 1;
}
