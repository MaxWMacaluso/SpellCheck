#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>

using namespace std;

const int alphabetSize = 26; //Creates a global variable that is of constant size.

//Trie node
struct TrieNode
{
    TrieNode* children[alphabetSize]; //Array of pointers.
    bool isEndOfWord; //True if it is the end of the word. Ex: if we have carriage and we search for car, it will return false because the r of car does not have isEndOfWord as true.
};

//For the Levenshtein distance function and recommend word. Will be used to return the word with the min difference.
struct WordWithCost
{
    string word;
    int cost;
};

//Returns new trie node (initialized to NULL).
//Is creating a new node and saying it's not the end of the word. Also, is going through its array of pointers and setting it equal to null.
TrieNode* getNode()
{
    TrieNode* newNode = new TrieNode;
    newNode->isEndOfWord = false;
    for (int x = 0; x < alphabetSize; x++)
    {
        newNode->children[x] = NULL;
    }
    return newNode;
}

//If not present, this function inserts the word into the data structure
//If the word is prefix of trie node, it just marks the leaf node.
void insert(TrieNode* root, string word)
{
    TrieNode* newNode = root;
    for (int x = 0; x < word.length(); x++)
    {
        int index = word[x] - 'a'; //Because ASCII value of "m" - "a" will yeild position in alphabet.
        if (newNode->children[index] == NULL) //If no letter there yet.
        {
            newNode->children[index] = getNode();
        }
        newNode = newNode->children[index];
    }
    //Marks leaf node as leaf
    newNode->isEndOfWord = true;
}

//Returns true if word is present in trie, else false
bool search(TrieNode* root, string word)
{
    TrieNode* newNode = root;
    for (int x = 0; x < word.length(); x++)
    {
        int index = word[x] - 'a'; //Subtracts 65 from the value to give the correct value of the index.
        if (newNode->children[index] == NULL)
        {
            return false;
        }
        newNode = newNode->children[index];
    }
    return (newNode != NULL && newNode->isEndOfWord);  //Saying if newCrawl is not equal to null and newcrawl is the end of the word, return true. Else, false.
}

//Helper function for recommendWord, determines most similar word present in trie to the input parameter of misPelledWord.
//Algorithm adapted from: https://murilo.wordpress.com/2011/02/01/fast-and-easy-levenshtein-distance-using-a-trie-in-c/
//Author: Murilo Adriano
WordWithCost levenshteinDistance(TrieNode* root, char ch, vector<int> prev_row, string missPelledWord, string wordIAmOn)
{
    int size = prev_row.size();
    vector <int> current_row(size); //Name of vector of ints with the size, size.
    current_row[0] = prev_row[0] + 1;
 
    //Calculate the min cost of insertion, deletion, or replace.
    int insert_or_del;
    int replace;
    for (int i = 1; i < size; i++) 
    {
        insert_or_del = min(current_row[i - 1] + 1, prev_row[i] + 1);
        if (missPelledWord[i - 1] == ch)
        {
           replace = prev_row[i - 1];
        }
        else
        {
            replace = (prev_row[i - 1] + 1);
        }
        current_row[i] = min(insert_or_del, replace);
    }
    WordWithCost min; //Instance of this struct
    min.cost = current_row[size - 1];
    min.word = wordIAmOn;
 
    //Safeguard to ensure that I am measuring the difference of full words and not just parts of words.
    bool takeChildRegardless = (root->isEndOfWord == true) ? false : true; //Creating a variable named takeChildRegardless saying if it is the end of the word (true) then set TCR to false, else true.
    //Another way to express same meaning: bool takeChildRegardless = !root->isEndOfWord;

    for (int x = 0; x < alphabetSize; x++)
    {
        if (root->children[x] != NULL)
        {
            WordWithCost childMin = levenshteinDistance(root->children[x], (x + 'a'), current_row, missPelledWord, wordIAmOn + (char)('a' + x)); //Casting a + x as a character.
            if ((childMin.cost < min.cost) || takeChildRegardless == true) //When we find a cost that is less than the min.cost, is because it is the minimum until the current row, so we update value of variable.
            {
                takeChildRegardless = false;
                min = childMin;
            }
        }
    }
    return min;
}

//Should use above helper function and return the closest matching word.
//Returns correct word.
string recommendWord(TrieNode* root, string missPelledWord)
{
    vector <int> currentRow;
    for (int x = 0; x <= missPelledWord.length(); x++)
    {
        currentRow.push_back(x);
    }
    WordWithCost min = levenshteinDistance(root, '\0', currentRow, missPelledWord, ""); //'\0' means no char.
    return min.word;
}

//Will read in the file and add the contents of the file into the trie. Will also give the option for the user to add a word to the dictionary.
void readInFileFunction(TrieNode* root, string fileName, int userResponse, string userWord)
{
    fstream dictStream;
    if (userResponse == 1) //Add word to dictionary. Append to the end.
    {
        dictStream.open(fileName, ios_base::app); //Opens the file and appends a word to the end of the file. The word will already be made into a lowercase in other function.
        //dictStream.open(fileName); //Seems to do same as above line.
        dictStream << userWord << endl; //Stream is cout, this works same way. Instead of: cout << userWord << endl;
        cout << userWord << " added successfully." << endl;
        cout << endl;
    }
    else //Read in file.
    {
        dictStream.open(fileName);
        if (dictStream.is_open())
        {
            string line = "";
            //Since the format of the file is going to be one word per line, only need one while loop.
            //Below line says: while I can grab a new line from disctStream, get that line and store it in the value of line. Works because one word per line.
            while (getline(dictStream, line)) //Delimeter here is the end line value. This is the defualt value for getline.
            {
                insert(root, line); //Will add the word to the Trie.
            }
        }
        else
        {
            cout << "Something went wrong opening the file." << endl;
            return;
        }
        dictStream.close(); //Will close file, preventing memory leak.
    }
}

//Will prompt the user to enter a word and will tell them if it's a word or not. Will also recommend a new, correct word.
void spellCheckDriver(TrieNode* root)
{
    string response = "";
    string rightResponse = "";
    int counter = 0;
    bool flag = false;
    cout << "Please enter a word. If it is spelled correctly, we will let you know. If not, we will recommend an alternative word. Can be upper or lowercase." << endl;
    getline(cin, response);
    cout << endl;
    
    //Below function is to make sure that it is a valid word. (No numbers)
    while (flag == false)
    {
        counter = 0; //Resets counter for new word.
        for (int x = 0; x < response.length(); x++)
        {
            if (isdigit(response[x]) == true)
            {
                counter++;
            }
        }
        if (counter != 0)
        {
            cout << "Please enter a valid word." << endl;
            getline(cin, response);
        }
        else
        {
            flag = true;
        }
    }

    //Will make the word lowercase.
    for (int x = 0; x < response.length(); x++)
    {
       response[x] = tolower(response[x]);
    }

    if (search(root, response) == true) //If word is already in tree, it is spelled correctly.
    {
        cout << response << " spelled correctly!" << endl;
        cout << endl;
    }
    else //If word is spelled incorrectly.
    {
        rightResponse = recommendWord(root, response);
        cout << "The correct spelling of the word is: " << rightResponse << endl;
        cout << endl;
    }
}

int main()
{
    TrieNode* root = getNode(); //Function is kind of like a constructor. This function creates a new node. Calling this particular node "root".
    string userResponse = "";
    bool flag = false;
    int counter = 0;
    cout << "Welcome to Max Macaluso and Richie Diaz's final project." << endl;
    cout << "Please press 1 if you would like to add a word to the dictionary. Press 2 if you would like to use the spell-checker function. Press 3 to quit." << endl;
    getline(cin, userResponse);

    //Function to ensure that the user enters a valid input.
    while (userResponse != "1" && userResponse != "2" && userResponse != "3") //Not using or (||) because that would enter the while loop if any of the condtions were true. EX: if user entered 2, which is valid, it would go into loop even though that is a valid input. So use and operator.
    {
        cout << "Please enter a valid option. Options are 1, 2, or 3." << endl;
        getline(cin, userResponse);
    }
    while (stoi(userResponse) != 3)
    {
        if (stoi(userResponse) == 1) //Add word to dictionary.
        {
            cout << "Please enter word that you would like to be added to dictionary:" << endl;
            getline(cin, userResponse);
            
            //Below function is to make sure that it is a valid word. (No numbers)
            while (flag == false)
            {
                //counter = 0; //Resets counter for new word.
                for (int x = 0; x < userResponse.length(); x++)
                {
                    if (isdigit(userResponse[x]) == true)
                    {
                        counter++;
                    }
                }
                if (counter != 0)
                {
                    cout << "Please enter a valid word." << endl;
                    getline(cin, userResponse);
                }
                else
                {
                    flag = true;
                }
            }
            //Will make the word lowercase.
            for (int x = 0; x < userResponse.length(); x++)
            {
                userResponse[x] = tolower(userResponse[x]);
            }
            readInFileFunction(root, "dict.txt", 1, userResponse); //Have to add 1 as input parameter because I am going to be calling functions multiple times. Only want to prompt user to add word if they press 1 though.
        }
        else if (stoi(userResponse) == 2) //Spell checker function
        {
            readInFileFunction(root, "dict.txt", 2, userResponse); //This will only read in file, not prompt user to add word.
            spellCheckDriver(root); //This will prompt the user to enter a word and all that jazz.
        }
        //To prompt user again
        cout << "Please press 1 if you would like to add a word to the dictionary. Press 2 if you would like to use the spell-checker function. Press 3 to quit." << endl;
        getline(cin, userResponse);

        //Function to ensure that the user enters a valid input.
        while (userResponse != "1" && userResponse != "2" && userResponse != "3") //Not using or (||) because that would enter the while loop if any of the condtions were true. EX: if user entered 2, which is valid, it would go into loop even though that is a valid input. So use and operator.
        {
            cout << "Please enter a valid option. Options are 1, 2, or 3." << endl;
            getline(cin, userResponse);
        }

    }
    if (stoi(userResponse) == 3) //To quit
    {
        cout << "Goodbye! Thanks for using our project." << endl;
        return 0;
    }
}