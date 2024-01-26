#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace std;

typedef unsigned long ulong;

struct credentials {
  void set_salt(string &);
  void set_hash(string &);

  void operator=(const credentials &);
  bool operator==(const credentials &);

  string salt;
  ulong password_hash;
};

void credentials::set_salt(string &username) {
  salt = "g0b1g0rAnge";
  //Modified the salt using the username
  //This makes it so the salt is unique to each user
  for (long unsigned int i = 0; i < salt.length(); i++) {
    salt[i] += username[i % username.size()] & 0x7;
  } 
}

void credentials::set_hash(string &password) {
  password += salt;
  password_hash = 0;
  //Used cyclic shift to calculate password_hash
  const char *c = password.c_str();
  while (*c) {
    password_hash = ((password_hash << 5) | (password_hash >> 27)) + *c++; 
  }
}

void credentials::operator=(const credentials &cred) {
  //Copying salt and password_hash
  salt = cred.salt;
  password_hash = cred.password_hash;
}

bool credentials::operator==(const credentials &cred) {
  //Comparing credentials to password_hash
  return password_hash == cred.password_hash;
}

istream &operator>>(istream &in, credentials &login) {
  //Reading salt and password_hash from file stream
  string p_hash;
  in >> login.salt >> p_hash;
  //Converting to a ulong
  login.password_hash = stoul(p_hash, nullptr, 16);

  return in;
}

ostream &operator<<(ostream &out, const credentials &login) {
  //Writing salt and password_hash to file stream
  out << left << login.salt << " " << hex << login.password_hash << endl;
  return out;
}

typedef unordered_map<string,credentials> hashtable;

void write_hashtable(hashtable &H, bool verbose) {
  string user, pass;
  int bucket = 0;

//Prints out buckets in use, capacity, and load if "-verbose" was in the command line argument
  if (verbose) {
    cout << "** S =" << setw(5) << right << bucket 
    << " N =" << setw(5) << right << H.bucket_count() 
    << " : load = " << fixed << setprecision(2) << H.load_factor() << endl;
    bucket++;
  }

//Reading username and password from stdin
  while (cin >> user >> pass) {
    credentials cred;
    cred.set_salt(user);
    cred.set_hash(pass);
    //Inserting username and credentials into hashtable
    H[user] = cred;
    if (verbose) {
      cout << "** S =" << setw(5) << right << bucket 
      << " N =" << setw(5) << right << H.bucket_count() 
      << " : load = " << fixed << setprecision(2) << H.load_factor() << endl;
      bucket++;
    }
  }

//Checking if file is open
  ofstream fout;
  fout.open("passwd.txt");
  if (!fout.is_open()) {
    cerr << "Could not open file." << endl;
  }

//Writing hash table content to file
  unordered_map<string,credentials>::iterator it = H.begin();
  while (it != H.end()) {
    fout << setw(11) << left << it->first << it->second;
    it++;
  }
  fout.close();

//Prints out each bucket and the contents of the bucket if "-verbose" was used in the cmd line argument
  if (verbose) {
    cout << endl;
    for (int i = 0; i < H.bucket_count(); i++) {
      cout << setw(6) << i << setw(5) << H.bucket_size(i);
      for (unordered_map<string,credentials>::local_iterator local = H.begin(i); local != H.end(i); local++) {
        cout << " " << local->first;
      }
      cout << endl;
    }
    cout << endl;
  }

}

void read_hashtable(hashtable &H, bool verbose) {
//Checking if file is open
  ifstream fin;
  fin.open("passwd.txt");
  if (!fin.is_open()) {
    //Error message is unable to open
    cerr << "Could not open file." << endl;
  }

  string user;
  int bucket;

//Prints out buckets in use, capacity, and load if "-verbose" was in the command line argument
  if (verbose) {
    cout << "** S =" << setw(5) << right << bucket 
    << " N =" << setw(5) << right << H.bucket_count() 
    << " : load = " << fixed << setprecision(2) << H.load_factor() << endl;
    bucket++;
  }

//Reading username and credentials from file
  while(fin >> user) {
    credentials cred;
    fin >> cred;
    //Inserting into hashtable
    H[user] = cred;
    if (verbose) {
      cout << "** S =" << setw(5) << right << bucket 
      << " N =" << setw(5) << right << H.bucket_count() 
      << " : load = " << fixed << setprecision(2) << H.load_factor() << endl;
      bucket++;
    }
  }

//Close file
  fin.close();

//Prints out each bucket and the contents of the bucket if "-verbose" was used in the cmd line argument
  if (verbose) {
    cout << endl;
    for (int i = 0; i < H.bucket_count(); i++) {
      cout << setw(6) << i << setw(5) << H.bucket_size(i);
      for (unordered_map<string,credentials>::local_iterator local = H.begin(i); local != H.end(i); local++) {
        cout << " " << local->first;
      }
      cout << endl;
    }
    cout << endl;
  }
}

int main(int argc, char *argv[]) {
  bool check, create, verbose = false;
  float load = 1.0;

//Parsing through command line
//Sets to true if used in command line
  for (int i = 1; i < argc; i++) {
    string a1 = argv[i];
    if (a1 == "-create") {
      create = true;
    }
    else if (a1 == "-check") {
      check = true;
    }
    else if (a1 == "-verbose") {
      verbose = true;
    }
    else if (a1 == "-load") {
      string Z = argv[i+1];
      load = stof(Z);
      ++i;
    }
  }

  if (!create && !check) {
    cerr << "usage: ./Prog4 -create|check [-load Z] [-verbose] < logins.txt" << endl;
    return 1;
  }

  hashtable H;
  //Set maximum load factor
  H.max_load_factor(load);

//If create is true, then write_hashtable is called
//Which creates and inserts credentials into hashtable
  if (create) {
    write_hashtable(H, verbose);
  }
  //If check is true, then read credentials from the file
  else if (check) {
    read_hashtable(H, verbose);
    
    string user, pass;
    //Reading username and password from stdin
    while (cin >> user >> pass) {
      //If username is not found in the hastable,
      //"bad username" will print out
      if(H.find(user) == H.end()) {
        cout << left << setw(11) << user << "bad username" << endl;
      }
      else {
        //Copy of username credentials
        credentials cred = H[user];
        //Calculate hash
        cred.set_salt(user);
        cred.set_hash(pass);

      //If calculated hash is equal to stored hash, print "access granted"
        if (H[user] == cred) {
          cout << left << setw(11) << user << "access granted" << endl;
        }
        else {
          cout << left << setw(11) << user << "bad password" << endl;
        }
      }
    }
  }
  return 0;
}
