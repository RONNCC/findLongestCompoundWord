/*
 * findLongest.cpp
 * Shomiron Ghose (RONNCC)
 * formatted to a width of 80 chars
 * a general comment structure is used,
 * rather than a specific one
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iterator>

//for malloc and similar instead of using new
#include <cstdlib>

//in order to use the stat command
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define firstchar 'a' //the 0th set that the offset
// is calculated from in order to calculate whether
// a character goes in the 'a','b','c'.. bin
// the number of these bins is NUM_CHARS
#define NUM_CHARS  26       //'a' ~ 'z'
#define string_list_t std::vector<std::string> // uses default allocator
// if we install boost, we can
// use the boost fast pool
// allocator instead

#define key_t size_t // the key type for maps,sets and any
// similar mapping data structure

#define print_top 100 //output the top 100 words found - string output is
//very slow :(, by having an exact quantity rather than
//outputting all of them, we can have a significant
//speed increase

//output file for the top '$print_top' words
//with the appropriate header saying how many words were printed $print_top
const char* compoundWordsFileName = "foundCompoundWords.txt";

/**
 * Trie Struct
 * contains a flag to tell whether that node is a word
 * and an array of edges for the next subnode.
 */
typedef struct _trie {
        bool isWord;
        struct _trie* edges[NUM_CHARS]; // we preallocate
        // for the possibility of all of the chars being next
        // instead of lazily allocating to increase speed
        // at the cost of memory
} trie;

/**
 * init_trie - makes the next level of a trie
 * @param node - the previous node from which to init from
 * in order to make the next level of the tree
 * @return the next level of the tree's parent node
 */
trie* init_trie(trie* node) {
    // init the trie node
    if (node == NULL) {
        node = (trie *) malloc(sizeof(trie));
    }

    // and set all edges to NULL, which
    // though usually 0, is not always 0
    // which means we cannot use memset :(
    // and are forced to use a for loop
    for (int i = 0; i < NUM_CHARS; i++) {
        node->edges[i] = NULL;
    }
    // set the word flag as false as we do
    // not know if this is a word ending yet
    node->isWord = false;
    return node;
}

/**
 * uinit_trie
 * @param node - frees all memory for that node
 * and it's subnodes
 */
inline void uninit_trie(trie* &node) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < NUM_CHARS; i++) {
        uninit_trie(node->edges[i]);
    }

    free(node);
    node = NULL;
}

/**
 * addWord - adds a word to the trie
 * @param node - starts adding using this as the parent node
 * @param str - the string to add
 * @return the node initially passed as the parent node with the
 * string now added on
 */
trie* addWord(trie *node, const char* str) {
    // if we reach the ending of a word, then
    // we should mark the current trie node as
    // a word ending
    if (str[0] == '\0') {
        node->isWord = true;
    }
    // otherwise we continue adding the rest of
    // the word in the trie
    else {
        int ch = str[0] - firstchar;
        trie*& edge = node->edges[ch];
        // if the next character of the word
        // isn't initialized in the trie, initialize it.
        if (edge == NULL) {
            edge = init_trie(edge);
        }
        str++;
        edge = addWord(edge, str);
    }
    return node;
}
/**
 * fileExists - check if a file exists using stat
 * @param filename - the filename/path to check for existence
 * @return 1 if it exists, 0 if it doesn't
 */
int fileExists(const char* filename) {
    // fastest way  to check for existence on a linux machine
    // using the stat command
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

/**
 * findInTrie - checks the next line for word breaks
 * @param node - the trie to check for the word
 * @param str - the word to check
 * @param start - the point at which to begin checking the string
 * @param end - the point at which to stop checking (the end of the word)
 * @param mid - the breakpoint. So we search through for the first word
 *              we find and then start searching from there for the next word
 *              by returning mid to `howManyWords` so that it uses that as
 *              the next point to begin searching from
 *
 * @return - returns whether or not the word exists in the trie
 */
bool findInTrie(trie *node, const char *str, int start, int end, int& mid) {
    trie* subnode = node;
    const char* pch = str + start;
    for (int i = start; i <= end; i++) {
        int ch = *pch - firstchar;
        if (subnode->edges[ch] == NULL) {
            return false;
        }

        pch++;
        subnode = subnode->edges[ch];
        if (i >= mid && subnode->isWord) {
            mid = i;
            return true;
        }
    }
    mid = end;
    return subnode->isWord;
}

/**
 * howManyWordsIn - finds how many words make up the string passed
 * @param node - the node of the trie to search in
 * @param str - the string in which we search for substrings
 * @param start - the beginning of the substring
 * @param end - the end of the substring
 * @param result - return if it is even in the trie.
 * @return how many words are in the substring passed
 */
int howManyWordsIn(trie* node, const char* str, int start, int end,
        bool &result) {

    result = false; //assume it is not a compound word
    if (start > end) { // the string params are wrong
        return 0; //wrong input
    }

    //search the trie using the substring to find if it exists
    for (int i = start; i <= end; i++) {
        //ok lets try to find our first word
        bool first_word = findInTrie(node, str, start, end, i);

        //if that first word = the string then return that, and
        // set result to that to say that we found it
        if (i == end) {
            return (result = first_word);
        }

        if(!first_word)
        {
            return 0;
        }

        //otherwise look for more words since the first is true and if
        // it isn't, return to stop wasting valuable cycles!
        bool more_words = false;
        //if we found them, then yay!
        int words_in = howManyWordsIn(node, str, i + 1, end, more_words);

        //the number of words_in the substring is words_in
        // we return that and the previous word we found
        // else it means this is not a valid compound word, and
        // we return 0 words were found inside it because we cannot
        // make the second half, so this is a 'original' word
        if (more_words) {
            result = true;
            return words_in + 1;
        }

    }
    return 0; // ok we found no words :(
}

/**
 * ReadInFile
 * @param filename - file to read text in from
 * @param root - root of the trie in which we search
 * @param lenDict - the len dict where we put all of our words
 *                  by length
 * @param LengthSet - a set of all the lens we have so we know
 *                    the longest word read in at anytime
 * @return
 */
int ReadInFile(const char* filename, trie*& root,
               std::map<key_t, string_list_t>& lenDict,//map of all lengths
               std::set<key_t>& LengthSet)
{
    root = init_trie(root);
    std::ifstream ifs(filename, std::ifstream::in);
    std::istream_iterator<std::string> word_itr(ifs);
    std::istream_iterator<std::string> end_of_stream;

    int words_readin = 0;
    for (; word_itr != end_of_stream; word_itr++, words_readin++) {
        addWord(root, (*word_itr).c_str());//add the word to the trie
        key_t len = word_itr->size();
        lenDict[len].push_back(*word_itr);//add the word to that length's list
        LengthSet.insert(len);//also insert that length to the set of total
        //lengths of words
    }

    ifs.close();
    return words_readin;
}

/*
 * Main Function
 */
int main(int argc, const char** argv) {

    // check for file input
    if (argc <= 1) {
        std::cout << "usage: findLongest [filename of list of words to use]\n"

        << "if no filename is input, tries to use\n"

        << "wordsforproblem.txt\n\n";
    }

    // if no filename is given, use the default
    const char* filename;

    if (argc > 1) {
        filename = argv[1];
    }
    else {
        filename = "wordsforproblem.txt";
    }

    if (!fileExists(filename)) {
        std::cout << "File does not exist :(, try again\n";
        exit(1);
    }

    // ok now create the trie and start adding words!
    trie* root = NULL;
    std::map<key_t, string_list_t> compoundLengthsDict;
    std::set<key_t> LengthSet;

    //read in the words and find the longest words

    int words_readin = ReadInFile(filename, root, compoundLengthsDict,
            LengthSet);
    std::cout << "# of Input words: " << words_readin << std::endl;

    //ok now we print out all the compound words
    int compoundWords = 0, isConcatWord = 0;
    std::set<key_t>::reverse_iterator length_rev_it = LengthSet.rbegin();
    std::set<key_t>::reverse_iterator length_end_iterator = LengthSet.rend();

    // we print them to the filename given by $CompoundWordsFileName
    std::ofstream compoundWordsFile(compoundWordsFileName);
    // and write an appropriate header for that
    compoundWordsFile << "Top " << print_top << " compound words:" << "\n";

    //starting at the longest length elements, iterate to the smallest
    for (; length_rev_it != length_end_iterator; length_rev_it++) {
        //get the length of them
        key_t len = *length_rev_it;
        //get all of the words that match that length
        string_list_t& dict = compoundLengthsDict[len];
        for (string_list_t::iterator it=dict.begin(); it!=dict.end();
        it++) {
            bool found = false;
            isConcatWord = howManyWordsIn(root, it->c_str(),
                                        0,// begin at the smallest possible size
                                        // if we know the smallest possible word
                                        // is > (1) - 1 since we check starting
                                        // by increasing this, we can avoid
                                        // checking for non-existant words
                                        it->size()-1,found);
            if (found && isConcatWord>1) {
                switch(compoundWords) // calculates a jumptable in asm, making
                // for a faster comparison than
                // a series of if statements if we want
                // to print out more specifics later
                {
                    case 0:
                    {
                        std::cout << "The 1st longest one: " << *it <<
                        '\n';
                        break;
                    }
                    case 1:
                    {
                        std::cout << "The 2nd longest one: " << *it <<
                        '\n';
                        break;
                    }
                    //... in case we want to put more cases later
                }
                if( compoundWords++ < print_top ) {
                    compoundWordsFile << *it << '\n'; // we use a '\n'
                    //instead of an endl to reduce
                    // needless expensive flushes
                }
            }
        }
    }
    //print how many compound words we found, and where to find a list of them
    std::cout << "# of Found words (some of which in " << compoundWordsFileName
            << "): " << compoundWords << std::endl;
    uninit_trie(root);
    //clean up the trie
    //the map and set can clean up after themselves
    return 0; //successful exit
}

