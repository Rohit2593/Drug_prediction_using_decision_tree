#include<bits/stdc++.h>
#include <fstream>
#include <sstream>

using namespace std;

string classifyAge(string age){
    int ageInt = stoi(age);
    return ageInt <= 40 ? "LOW" : "HIGH";
}

void transformData(vector<vector<string>> &data){
    for(auto &i: data){
        i[0] = classifyAge(i[0]);
    }
}

void printData(vector<vector<string>> &data){
    for(auto i: data){
        for(auto j: i){
            cout << j << ' ';
        }

        cout << "\n";
    }
}  

pair<vector<vector<string>>, vector<vector<string>>> trainTestSplit(vector<vector<string>> &data, double splitRatio){
    vector<vector<string>> trainingSet, testSet;
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    for (const auto& d : data) {
        if (dis(gen) < splitRatio) {
            trainingSet.push_back(d);
        } else {
            testSet.push_back(d);
        }
    }

    return make_pair(trainingSet, testSet);
}

int main(){
    string filename = "drug200.csv";
    ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    string line;
    vector<vector<string>> data;
    bool firstRow = true;

    while(getline(file, line)){ // iterating over all the rows in dataset

        vector<string> row;
        string currString = "";

        for(auto i: line){
            if(i == ','){
                row.push_back(currString);
                currString = "";
            }

            else
                currString += i;
        }
        
        row.push_back(currString);

        if(!firstRow)
            data.push_back(row);
        firstRow = false;
    }

    transformData(data); // classifies age into {LOW, HIGH}

    pair<vector<vector<string>>, vector<vector<string>>> trainTest = trainTestSplit(data, 0.8);

    vector<vector<string>> trainData = trainTest.first;
    vector<vector<string>> testData = trainTest.second;

    // printData(trainData);
    // cout << "\n\n";
    // printData(testData);

    
}