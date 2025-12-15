//=============================================================
//  Name           : CS300Project2.cpp
//  Author         : Austin Freeman
//  Version        : 1.0
//  Description    : Final Project w/ Hash Tables
//=============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <time.h>
#include <algorithm>

using namespace std;

const unsigned int DEFAULT_SIZE = 64;

/*Created a few custom function, decided to store them here for accessing
* toString(vector<string> v) - Does the obvious, converts the vector to a string
* split(string str, char delimiter) - Splits a vector
* hasItem(string ID, vector<string> t) - checks if a vector contains a string
*/

void toString(vector<string> v) {
    string s = "";
    for (string t : v) {
        s += t + ", ";
    }
    s = s.substr(0, s.length() - 2);
    cout << s << endl;
}

vector<string> split(string str, char del) {
    stringstream ss(str);
    vector<string> res;
    string token;
    while (getline(ss, token, del)) {
        res.push_back(token);
    }
    return res;
}

bool hasItem(string courseID, vector<string> target) {
    for (int i = 0; i < target.size(); i++) {
        if (courseID.compare(target[i]) == 0 || courseID.empty()) { return true; }
    }
    return false;
}


//Creates the structure that we're going to use for our curriculum
struct Course {
    string courseId;
    string courseName;
    vector<string> prereq;
};


//Creates our hashtable class that we use to organize everything 
class HashTable {
private:
    struct Node {
        Course course;
        unsigned int key;
        Node* next;

        Node() {
            key = UINT_MAX;
            next = nullptr;
        }

        Node(Course aCourse) : Node() {
            course = aCourse;
        }
        Node(Course aCourse, unsigned int aKey) : Node(aCourse) {
            key = aKey;
        }
    };

    //Initialize some nodes for us to work with. 
    vector<Node> nodes;

    //Creates a default tableSize
    unsigned int tableSize = DEFAULT_SIZE;

    unsigned int hash(int key);

public:
    HashTable();
    HashTable(unsigned int size);
    virtual ~HashTable();
    void PrintAll(vector<Course> c);
    vector<Course> getCourse();
    void Insert(Course course);
    void Search(string courseId);
};
//Default Constructor
HashTable::HashTable() {
    nodes.resize(tableSize);
}

//Initialize with unsigned int
HashTable::HashTable(unsigned int size) {
    this->tableSize = size;
    nodes.resize(tableSize);
}

//Destructor 
HashTable::~HashTable() {
    for (int i{}; i < tableSize; i++) {
        Node* t = &nodes.at(i);
        while (t != nullptr) {
            Node* prev = t;
            t = t->next;
            delete prev;
        }
        nodes[i].next = nullptr;
        nodes[i].key = UINT_MAX;
    }
}
unsigned int HashTable::hash(int key) {
    return key % tableSize;
}

//Insert into table
void HashTable::Insert(Course c) {
    unsigned key = hash(atoi(c.courseId.c_str()));
    Node* currNode = &(nodes.at(key));
    
    if (currNode->key == UINT_MAX) {
        currNode->key = key;
        currNode->course = c;
        currNode->next = nullptr;
    }
    else {
        while (currNode->next != nullptr) {
            if (currNode->course.courseId.compare(c.courseId) == 0 || currNode->course.courseName.compare(c.courseName) == 0) {
                return;
            }
            currNode = currNode->next;

        }
        currNode->next = new Node(c, key);
    }
}

//Creates a vector of course objects to organize
vector<Course> HashTable::getCourse() {
    vector<Course> courses;

    for (size_t i = 0; i < tableSize; i++) {
        Node* node = &nodes.at(i);
        if (node->key == UINT_MAX) {
            continue;
        }
        while (node != nullptr) {
            courses.push_back(node->course);
            node = node->next;
        }
    }
    return courses;
}


//Passing a vector of Course nodes to the  
void HashTable::PrintAll(vector<Course> c) {
    sort(c.begin(), c.end(),
        [](const Course& a, const Course& b) {
            return a.courseId < b.courseId;
        });
    for (const Course& course : c) {
        cout << course.courseId << ", " << course.courseName << endl;
    }
}


//Simple search function that prints details if something exists
void HashTable::Search(string courseId) {
    Node* node;
    Course c;
    unsigned key = hash(atoi(courseId.c_str()));

    node = &(nodes.at(key));

    if (node->key == UINT_MAX) {
        return;
    }
    while (node != nullptr) {
        if (node->course.courseId.compare(courseId) == 0) {
            cout << node->course.courseId << ", " << node->course.courseName << endl << "Prerequisites: ";
            toString(node->course.prereq);
            cout << endl;
            return;
        }
        node = node->next;
    }
    cout << "Not found" << endl;
    return;
}


//We pass the hashtable and the csvPath through and parse
//Through each of our entries, ensuring that they fit the requirements
void loadBids(string csvPath, HashTable* hashTable) {
    ifstream file(csvPath);
    string line;

    Course course;
    vector<string> courseInfo;
    vector<string> prereqList;

    while (!file.is_open()) {
        cout << "Error: " << csvPath << endl;
        return;
    }
    try {
        while (getline(file, line)) {
            vector<string> s = split(line, ',');
            if (s.size() > 0) {
                courseInfo.push_back(line);
                prereqList.push_back(line.substr(0, line.find(',')));
            }
        }
        file.close();
        for (size_t i = 0; i < courseInfo.size(); i++) {
            vector<string> s = split(courseInfo[i], ',');
            if (s.size() < 2) {
                continue;
            }
            if (s.size() > 2) {
                course.courseId = s[0];
                course.courseName = s[1];
                for (size_t j = 2; j < s.size(); j++) {
                    if (!s[j].empty() && hasItem(s[j], prereqList)) {
                        course.prereq.push_back(s[j]);
                    }
                    else if (!s[j].empty()) {
                        course.prereq.clear();
                        return;
                    }
                    else {
                        course.prereq.push_back("None");
                    }
                }
            }
            hashTable->Insert(course);
            course.prereq.clear();
        }
    }
    catch (runtime_error& e) {
        cout << "Error " << e.what() << endl;
        return;
    }
}



int main()
{
    //Here we initialize a few simple variables
    cout << "Welcome to the course planner." << endl;
    string csvPath, userIn;
    cout << "Please input the file name associated with your document: ";
    getline(cin, csvPath);

    //Create the containers that we're going to use throughout the program
    HashTable* curriculum = new HashTable();
    vector<Course> c;
    Course course;



    int input = 0;
    while(input != 9) {
        cout << "==========================" << endl;
        cout << "| 1. Load Data Structure |" << endl;
        cout << "| 2. Print Course List   |" << endl;
        cout << "| 3. Print Course        |" << endl;
        cout << "| 9. Exit                |" << endl;
        cout << "==========================" << endl;
        cout << endl;
        cout << "What would you like to do?" << endl;
        string i = "0";
        getline(std::cin, i);


        while (true) {
            //Just checking to make sure our input is in a format that we can actually work with
            try {
                input = stoi(i);
                break;
            }
            //If it isn't we prompt again
            catch (...) {
                
                cout << "Improper format" << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "What would you like to do?" << endl;
                getline(cin, i);
            }
        }
        //Every time the loop iterates this course object container is updated 
        c = curriculum->getCourse();

        switch (input) {
        case 1:
            cout << "Loading from file: " << csvPath << endl;
            //load the bids
            loadBids(csvPath, curriculum);
            break;
        case 2:
            cout << "Here's a sample schedule:" << endl;
            //Print the bids. Since we're passing in the most recent vector for sorting
            curriculum->PrintAll(c);
            break;
        case 3:
            cout << "What course would you like to know about?" << endl;
            //Added this to standardize input, i.e.csci400 -> CSCI400
            cin >> userIn;
            for (auto& x : userIn) {
                x = toupper(x);
            }
            curriculum->Search(userIn);
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 9:
            cout << "Thank you for using the course planner!" << endl;
            break;
        default:
            cout << input << " is not a valid option" << endl;
            break;
        };
        
        cout << "Press enter to continue" << endl;

        //Freezes things until the user presses enter
        int flag = cin.get();

        //Clears the screen
        cout << "\033[2J\033[H";
    }
}
