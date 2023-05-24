#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <unordered_map>
#include <algorithm>
using namespace std; 

// to store the input from the user to use when writing back to the files to update them. 
string clicksFile; 
string impressionsFile;

// function that takes a line and gets the keywords from it and stores them in a set
void putAllKeywordsTogether(string currLine, set<string> &allKeywords, unordered_map<string, vector<string>> &keywordsURLsMap){
    stringstream ss(currLine);
    string token;
    getline(ss, token, ',');
    string url = token;
    while(getline(ss, token, ',')){
        allKeywords.insert(token);
        keywordsURLsMap[url].push_back(token);
    }
}

// function that reads the lines from the keywords file and uses putAllKeywordsTogether to store all the keywords in a set
void readKeywords(set<string> &allKeywords, set<string> &allURLs, unordered_map<string, vector<string>> &keywordsURLsMap){
    ifstream keywordsStream;
    cout << "Please enter the name of the keywords file: ";
    string keywordsFileName;
    cin >> keywordsFileName;
    keywordsStream.open(keywordsFileName);
    while(!keywordsStream.is_open()){
        cout << "Problem in opening the keywords file. Please enter the name of the keywords file again: ";
        cin >> keywordsFileName;
        keywordsStream.open(keywordsFileName);
    }
    string line;
    while(getline(keywordsStream, line)){
        string tmp = line; 
        stringstream ss(tmp);
        string token;
        getline(ss, token, ',');
        allURLs.insert(token);
        putAllKeywordsTogether(line, allKeywords, keywordsURLsMap);
    }
    keywordsStream.close();
}

// function that fills the webgraph adjacency list
void fillWebgraph(unordered_map<string, unordered_map<string, int>> &webgraph){
    ifstream webgraphStream; 
    cout << "Please enter the name of the webgraph file: ";
    string webgraphFileName;
    cin >> webgraphFileName;
    webgraphStream.open(webgraphFileName);
    while(!webgraphStream.is_open()){
        cout << "Problem in opening the webgraph file. Please enter the name of the webgraph file again: ";
        cin >> webgraphFileName;
        webgraphStream.open(webgraphFileName);
    }
    string line; 
    while(getline(webgraphStream, line)){
        stringstream ss(line);
        string token;
        getline(ss, token, ',');
        string firstWebsite = token;
        getline(ss, token, ',');
        string secondWebsite = token;
        webgraph[firstWebsite][secondWebsite] = 1;
    }
    webgraphStream.close();
}

void updateFiles(unordered_map<string, int> &numOfImpressions, unordered_map<string, int> &numOfClicks); // forward declaration

void exitProgram(unordered_map<string, int> &numOfImpressions, unordered_map<string, int> &numOfClicks){
    cout << "Thank you for using our search engine!\n";
    updateFiles(numOfImpressions, numOfClicks);
    exit(0);
}
void calculatePageRank(unordered_map<string, double> &pageRanks, unordered_map<string, unordered_map<string, int>> &webgraph, set<string> &allURLs){
    unordered_map<string, double> PRIterations; 
    // initialize all page ranks to 0.25
    for(const auto& i : allURLs)
        PRIterations[i] = 0.25;

    // calculate the page rank iterations of each webpage
    for(int i = 0; i < 100; i++){
        // initialize a temporary copy of the page rank iterations map to be used in the for loop below
        unordered_map<string, double> tempPRIterations = PRIterations;
        for(auto it = PRIterations.begin(); it != PRIterations.end(); it++){
            PRIterations[it->first] = 0;
            for(auto it2 = webgraph.begin(); it2 != webgraph.end(); it2++){
                if(webgraph[it2->first][it->first] == 1){
                    PRIterations[it->first] += tempPRIterations[it2->first] / webgraph[it2->first].size();
                }
                else{
                    // else, remove the edge from the adjacency list that was created because of the if condition above
                    webgraph[it2->first].erase(it->first);
                }
            }
        }           
    }

    // create a vector of pairs from PRIterations to sort the page ranks
    vector<pair<string, double>> pairs;
    for (const auto& entry : PRIterations) {
        pairs.emplace_back(entry);
    }

    // Sort the vector in descending order based on the values using the function sort defined in the algorithm library, which we know uses heapsort and other sorting algorithms to have an average time complexity of O(nlogn)
    sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    // Store the sorted keys in pageRanks with corresponding values
    int rank = 1;
    for (const auto& pair : pairs) {
        pageRanks[pair.first] = rank++;
    }
}

void initializeCTR(unordered_map<string, int> &numOfImpressions, unordered_map<string, int> &clicks, unordered_map<string, double> &CTR, string &clicksFile, string &impressionsFile){
    // reading the impressions
    ifstream numOfImpressionsStream;
    cout << "Please enter the name of the impressions file: ";
    cin >> impressionsFile;
    numOfImpressionsStream.open(impressionsFile);
    while(!numOfImpressionsStream.is_open()){
        cout << "Problem in opening the number of impressions file. Please enter the name of the impressions file again: ";
        cin >> impressionsFile;
        numOfImpressionsStream.open(impressionsFile);
    }
    string line;
    while(getline(numOfImpressionsStream, line)){
        stringstream ss(line);
        string token;
        getline(ss, token, ',');
        string url = token;
        getline(ss, token, ',');
        int NoM = stoi(token);
        numOfImpressions[url] = NoM;
    }
    numOfImpressionsStream.close();

    // reading the clicks
    ifstream clicksStream;
    cout << "Please enter the name of the clicks file: ";
    cin >> clicksFile;
    clicksStream.open(clicksFile);
    while(!clicksStream.is_open()){
        cout << "Problem in opening the clicks file. Please enter the name of the clicks file again: ";
        cin >> clicksFile;
        clicksStream.open(clicksFile);
    }
    string line2;
    while(getline(clicksStream, line2)){
        stringstream ss(line2);
        string token;
        getline(ss, token, ',');
        string url = token;
        getline(ss, token, ',');
        int numClicks = stoi(token);
        clicks[url] = numClicks;
    }
    clicksStream.close();

    // initialize the CTR map
    for(const auto& i : numOfImpressions)
        CTR[i.first] = ((double)clicks[i.first] / (double)numOfImpressions[i.first]) * 100;
}

void calculateScores(unordered_map<string, double> &scores, unordered_map<string, double> CTRs, unordered_map<string, double> pageRanks, unordered_map<string, int> numOfImpressions){
    for(const auto& i : CTRs)
        scores[i.first] = (0.4*pageRanks[i.first]) + ((1 - (0.1*(double)numOfImpressions[i.first])/(0.1*(double)numOfImpressions[i.first] + 1))*pageRanks[i.first] + ((0.1*(double)numOfImpressions[i.first])/(0.1*(double)numOfImpressions[i.first] + 1)) * CTRs[i.first]) * 0.4;
}

void constructAllWordsAllURLs(vector<vector<int>> &allWordsAllURLs, unordered_map<string, vector<string>> &keywordsURLsMap, 
                                unordered_map<string, int> &URLIndices, unordered_map<string, int> &wordIndices, set<string> allURLs, set<string> &allKeywords){
    // resize the vector to rows being number of URLs and columns being number of keywords
    allWordsAllURLs.resize(allURLs.size());
    for(int i = 0; i < allURLs.size(); i++){
        allWordsAllURLs[i].resize(allKeywords.size(), 0);
    }

    int k = 0; // this will be the index of the URL
    for(const auto& i : allURLs){
        URLIndices[i] = k;
        k++;
    }

    int l = 0; // this will be the index of the keyword
    for(const auto& i : allKeywords){
        wordIndices[i] = l;
        l++;
    }

    for(const auto& i : allURLs){
        vector<string> currURLWords = keywordsURLsMap[i]; 
        for(int j = 0; j < currURLWords.size(); j++){
            allWordsAllURLs[URLIndices[i]][wordIndices[currURLWords[j]]] = 1;
        }
    }
}

bool compareScores(const string& url1, const string& url2, const unordered_map<string, double>& scores) {
    return scores.at(url1) > scores.at(url2);
}
void sortSearchResults(vector<string>& searchResults, const unordered_map<string, double>& scores) {
    sort(searchResults.begin(), searchResults.end(), [&scores](const string& url1, const string& url2) { return compareScores(url1, url2, scores); });
}
void updateCTRsAndScores(vector<string> updated, unordered_map<string, double> &scores, unordered_map<string, double> &CTRs, unordered_map<string, int> &numOfImpressions,  unordered_map<string, int> &numOfClicks, unordered_map<string, double> pageRanks){
    for(const auto& i : updated){
        CTRs[i] = ((double)numOfClicks[i] / (double)numOfImpressions[i]) * 100;
        scores[i] = (0.4*pageRanks[i]) + ((1 - (0.1*(double)numOfImpressions[i])/(0.1*(double)numOfImpressions[i] + 1))*pageRanks[i] + ((0.1*(double)numOfImpressions[i])/(0.1*(double)numOfImpressions[i] + 1)) * CTRs[i]) * 0.4;
    }
}
void updateOneCTRScore(string updated, unordered_map<string, double> &scores, unordered_map<string, double> &CTRs, unordered_map<string, int> &numOfImpressions,  unordered_map<string, int> &numOfClicks, unordered_map<string, double> pageRanks){
    CTRs[updated] = ((double)numOfClicks[updated] / (double)numOfImpressions[updated]) * 100;
    scores[updated] = (0.4*pageRanks[updated]) + ((1 - (0.1*(double)numOfImpressions[updated])/(0.1*(double)numOfImpressions[updated] + 1))*pageRanks[updated] + ((0.1*(double)numOfImpressions[updated])/(0.1*(double)numOfImpressions[updated] + 1)) * CTRs[updated]) * 0.4;
}
vector<string> searchAND(string firstWord, string secondWord, vector<vector<int>> allWordsAllURLs, unordered_map<string, int> URLIndices, unordered_map<string, int> wordIndices){
    vector<string> searchResults; 
    if(wordIndices.find(firstWord) == wordIndices.end() || wordIndices.find(secondWord) == wordIndices.end()){
        return searchResults;
    }
    int firstWordIndex = wordIndices[firstWord];
    int secondWordIndex = wordIndices[secondWord];
    // for(int i = 0; i < allWordsAllURLs.size(); i++){
    //     if(allWordsAllURLs[i][firstWordIndex] == 1 && allWordsAllURLs[i][secondWordIndex] == 1){
    //         for(const auto& j : URLIndices){
    //             if(j.second == i){
    //                 searchResults.push_back(j.first);
    //             }
    //         }
    //     }
    // }
    for(const auto &i : URLIndices){
        int currURLIndex = i.second;
        if(allWordsAllURLs[currURLIndex][firstWordIndex] == 1 && allWordsAllURLs[currURLIndex][secondWordIndex] == 1){
            searchResults.push_back(i.first);
        }
    }
    return searchResults;
}
vector<string> searchOR(string firstWord, string secondWord, vector<vector<int>> allWordsAllURLs, unordered_map<string, int> URLIndices, unordered_map<string, int> wordIndices){
    vector<string> searchResults; 

    if(wordIndices.find(firstWord) != wordIndices.end() && wordIndices.find(secondWord) != wordIndices.end()){
        int firstWordIndex = wordIndices[firstWord];
        int secondWordIndex = wordIndices[secondWord];
        for(const auto &i : URLIndices){
            int currURLIndex = i.second;
            if(allWordsAllURLs[currURLIndex][firstWordIndex] == 1 || allWordsAllURLs[currURLIndex][secondWordIndex] == 1){
                searchResults.push_back(i.first);
            }
        }
    }
    else if(wordIndices.find(firstWord) != wordIndices.end()){
        int firstWordIndex = wordIndices[firstWord];
        for(const auto &i : URLIndices){
            int currURLIndex = i.second;
            if(allWordsAllURLs[currURLIndex][firstWordIndex] == 1){
                searchResults.push_back(i.first);
            }
        }
    }
    else if(wordIndices.find(secondWord) != wordIndices.end()){
        int secondWordIndex = wordIndices[secondWord];
        for(const auto &i : URLIndices){
            int currURLIndex = i.second;
            if(allWordsAllURLs[currURLIndex][secondWordIndex] == 1){
                searchResults.push_back(i.first);
            }
        }
    }
    return searchResults;
}
vector<string> searchReg(string query, vector<vector<int>> allWordsAllURLs, unordered_map<string, int> URLIndices, unordered_map<string, int> wordIndices){
    vector<string> searchResults;
    if(wordIndices.find(query) != wordIndices.end()){
        int queryIndex = wordIndices[query];
        for(const auto &i : URLIndices){
            int currURLIndex = i.second;
            if(allWordsAllURLs[currURLIndex][queryIndex] == 1){
                searchResults.push_back(i.first);
            }
        }
    }
    return searchResults;
}

void newSearch(vector<vector<int>> &allWordsAllURLs, unordered_map<string, int> &URLIndices, unordered_map<string, int> &wordIndices, unordered_map<string, int> &numOfImpressions, unordered_map<string, int> &numOfClicks, unordered_map<string, double> &scores, unordered_map<string, double> &pageRanks, unordered_map<string, double> &CTRs); // forward declaration
// I know the declaration is ridiculously long but I had to pass all the maps that I'm using since I didn't declare them as global variables
void displayResults(vector<string> &searchResults, vector<vector<int>> allWordsAllURLs, unordered_map<string, int> URLIndices, unordered_map<string, int> wordIndices,
                     unordered_map<string, int> &numOfImpressions, unordered_map<string, int> &numOfClicks, unordered_map<string, double> &scores, unordered_map<string, double> pageRanks, unordered_map<string, double> &CTRs){
    if(searchResults.size() == 0){
        cout << "No results found.\n";
        char choice; 
        cout << "Would you like to:\n1. New search\n2. Exit\n\nType in the number of your choice: ";
        cin >> choice;
        while(choice != '1' && choice != '2'){
            cout << "Invalid choice. Please try again: ";
            cin.ignore(10000, '\n');
            cin >> choice;
        }
        if(choice == '1'){
            newSearch(allWordsAllURLs, URLIndices, wordIndices, numOfImpressions, numOfClicks, scores, pageRanks, CTRs);
        }
        else{
            exitProgram(numOfImpressions, numOfClicks);
        }
    }
    else{
        char choice; 
        cout << "Results: " << endl;
        sortSearchResults(searchResults, scores);
        for(int i = 0; i < searchResults.size(); i++){
            cout << i + 1 << ". " << searchResults[i] << endl; // increment impressions
            numOfImpressions[searchResults[i]]++;
        }
        updateCTRsAndScores(searchResults, scores, CTRs, numOfImpressions, numOfClicks, pageRanks);
        cout << "\nWould you like to:\n1. Choose a webpage to open\n2. New search\n3. Exit\nType in the number of your choice: ";
        cin >> choice;
        while(choice != '1' && choice != '2' && choice != '3'){
            cout << "Invalid choice. Please try again: ";
            cin.ignore(10000, '\n');
            cin >> choice;
        }
        if(choice == '1'){
            char choice2; 
            cout << "Which webpage would you like to open? Type in its number: ";
            cin >> choice2;
            while(choice2 < '1' || (choice2 - '0') > searchResults.size()){
                cout << "Invalid choice. Please try again: ";
                cin.ignore(10000, '\n');
                cin >> choice2;
            }
            string webpage = searchResults[(choice2 - '0') - 1];
            numOfClicks[webpage]++; // increment clicks
            updateOneCTRScore(webpage, scores, CTRs, numOfImpressions, numOfClicks, pageRanks);
            cout << "You're now viewing " << webpage << ".\nWould you like to\n1. Go back to results\n2. New search\n3. Exit\n\nType in the number of your choice: ";
            char choice3; 
            cin >> choice3;
            while(choice3 != '1' && choice3 != '2' && choice3 != '3'){
                cout << "Invalid choice. Please try again: ";
                cin.ignore(10000, '\n');
                cin >> choice3;
            }
            if(choice3 == '1')
                displayResults(searchResults, allWordsAllURLs, URLIndices, wordIndices, numOfImpressions, numOfClicks, scores, pageRanks, CTRs);
            else if(choice3 == '2')
                newSearch(allWordsAllURLs, URLIndices, wordIndices, numOfImpressions, numOfClicks, scores, pageRanks, CTRs);
            else
                exitProgram(numOfImpressions, numOfClicks);
        }
        else if(choice == '2'){
            newSearch(allWordsAllURLs, URLIndices, wordIndices, numOfImpressions, numOfClicks, scores, pageRanks, CTRs);
        }
        else{
            exitProgram(numOfImpressions, numOfClicks);
        }
    }
}
void newSearch(vector<vector<int>> &allWordsAllURLs, unordered_map<string, int> &URLIndices, unordered_map<string, int> &wordIndices, unordered_map<string, int> &numOfImpressions, unordered_map<string,
                 int> &numOfClicks, unordered_map<string, double> &scores, unordered_map<string, double> &pageRanks, unordered_map<string, double> &CTRs){
    
    string query;
    cout << "Please enter your query: ";
    cin.ignore(10000, '\n');
    getline(cin, query);
    vector<string> searchResults; 

    int doesAndExist, doesOrExist, doesSpaceExist; 
    doesAndExist = query.find(" AND ");
    doesOrExist = query.find(" OR ");
    doesSpaceExist = query.find(" ");

    if(doesAndExist != query.npos){
        string firstWord = query.substr(0, doesAndExist);
        string secondWord = query.substr(doesAndExist + 5, query.length() - doesAndExist - 5);
        searchResults = searchAND(firstWord, secondWord, allWordsAllURLs, URLIndices, wordIndices);
    }
    else if(doesOrExist != query.npos){
        string firstWord = query.substr(0, doesOrExist);
        string secondWord = query.substr(doesOrExist + 4, query.length() - doesOrExist - 4);
        searchResults = searchOR(firstWord, secondWord, allWordsAllURLs, URLIndices, wordIndices);
    }
    else if(query[0] == '"' && query[query.length() - 1] == '"'){
        string extractedQuery = query.substr(1, query.length() - 2);
        searchResults = searchReg(extractedQuery, allWordsAllURLs, URLIndices, wordIndices);
    }
    else if(doesSpaceExist != query.npos){
        string firstWord = query.substr(0, doesSpaceExist);
        string secondWord = query.substr(doesSpaceExist + 1, query.length() - doesSpaceExist - 1);
        searchResults = searchOR(firstWord, secondWord, allWordsAllURLs, URLIndices, wordIndices);
    }
    else{
        searchResults = searchReg(query, allWordsAllURLs, URLIndices, wordIndices);
    }
    displayResults(searchResults, allWordsAllURLs, URLIndices, wordIndices, numOfImpressions, numOfClicks, scores, pageRanks, CTRs);
}

void mainMenu(vector<vector<int>> &allWordsAllURLs, unordered_map<string, int> URLIndices, unordered_map<string, int> wordIndices, unordered_map<string, int> &numOfImpressions, 
                unordered_map<string, int> &numOfClicks, unordered_map<string, double> &scores, unordered_map<string, double> &pageRanks, unordered_map<string, double> &CTRs){
    char choice1 = '0';
    cout << "Welcome!\n";
    while(choice1 != '2'){
        cout << "What would you like to do?\n1. New search\n2. Exit\n\nType in the number of your choice: ";
        cin >> choice1;
        if(choice1 == '1'){
            newSearch(allWordsAllURLs, URLIndices, wordIndices, numOfImpressions, numOfClicks, scores, pageRanks, CTRs);
        }
        else if(choice1 == '2'){
            exitProgram(numOfImpressions, numOfClicks);
        }
        else{
            cout << "Invalid choice. Please try again.\n";
            cin.ignore(10000, '\n');
        }
    }

}
void updateFiles(unordered_map<string, int> &numOfImpressions, unordered_map<string, int> &numOfClicks){
    ofstream clicksOut(clicksFile);
    ofstream impressionsOut(impressionsFile);
    for(auto it = numOfImpressions.begin(); it != numOfImpressions.end(); it++){
        impressionsOut << it->first << "," << it->second << endl;
    }
    for(auto it = numOfClicks.begin(); it != numOfClicks.end(); it++){
        clicksOut << it->first << "," << it->second << endl;
    }
    clicksOut.close();
    impressionsOut.close();
}

int main(){
    // this set will store all the keywords of all webpages
    set<string> allKeywords;
    // this set will store all the urls of all webpages 
    set<string> allURLs; 
    // this map will store the keywords of each webpage. The key is the url and the value is a vector of strings that represent the keywords of the webpage
    unordered_map<string, vector<string>> keywordsURLsMap;
    readKeywords(allKeywords, allURLs, keywordsURLsMap);
    // this map will represent the adjacency list of the webgraph. The key is the url and the value is a map whose key is the url of the webpage that the key url points to and the value will be initialized to 1
    unordered_map<string, unordered_map<string, int>> webgraph;
    fillWebgraph(webgraph);
    // this map will store the page rank of each webpage. The key is the url and the value is the page rank
    unordered_map<string, double> pageRanks;
    calculatePageRank(pageRanks, webgraph, allURLs);
    // this map will store the number of impressions of each webpage. The key is the url and the value is the number of impressions
    unordered_map<string, int> numOfImpressions; 
    // this map will store the number of click throughs of each webpage. The key is the url and the value is the number of click throughs
    unordered_map<string, int> clickThroughs;
    // this map will store the CTR of each webpage. The key is the url and the value is the CTR
    unordered_map<string, double> CTRs;
    // string clicksFile, impressionsFile; 
    initializeCTR(numOfImpressions, clickThroughs, CTRs, clicksFile, impressionsFile);
    // this map will store the scores of each webpage. The key is the url and the value is the score
    unordered_map<string, double> scores; 
    calculateScores(scores, CTRs, pageRanks, numOfImpressions);
    // this vector will represent a matrix where the rows are the urls and the columns are the keywords. The value at each index will be 1 if that keyword appears in that url and 0 otherwise
    vector<vector<int>> allWordsAllURLs;
    // this map will store the index of each url in the allWordsAllURLs vector
    unordered_map<string, int> URLIndices; 
    // this map will store the index of each word in the allWordsAllURLs vector
    unordered_map<string, int> wordIndices;
    constructAllWordsAllURLs(allWordsAllURLs, keywordsURLsMap, URLIndices, wordIndices, allURLs, allKeywords);
    

    mainMenu(allWordsAllURLs, URLIndices, wordIndices, numOfImpressions, clickThroughs, scores, pageRanks, CTRs);


    return 0;
}