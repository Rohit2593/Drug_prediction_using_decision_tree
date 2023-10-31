#include<bits/stdc++.h>
#include <fstream>
#include <sstream>

using namespace std;

const int numLabels = 4;
const vector<string> labels = {"Age", "Sex", "BP", "Cholestrol", "Drug"};
vector<set<string>> labelValues(numLabels);

void setLabelValues(vector<vector<string>> &data){
    for(auto i: data){
        for(int j = 0; j<numLabels; j++)
            labelValues[j].insert(i[j]);
    }
}

string classifyAge(string age){
    int ageInt = stoi(age);
    if(ageInt <= 18)
        return "LOW";

    if(ageInt <= 27)
        return "LOW-MEDIUM";
    
    if(ageInt <= 45)
        return "HIGH-MEDIUM";
    return "HIGH"; 
}

void transformData(vector<vector<string>> &data){
    for(auto &i: data){ // classifying age into HIGH or LOW
        i[0] = classifyAge(i[0]);
    }

    for(auto &i: data){ // erasing the unwanted column from dataset
        i.erase(i.begin() + 4);
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

double calculateGiniIndex(map<string, int> &m){
    int total = 0;

    for(auto i: m)
        total += i.second;

    double gini = 1.0;

    for(auto i: m){
        double currProbability = (double) i.second / (double) total;
        gini -= (currProbability * currProbability);
    }

    return gini;
} 

class node{
public:
    map<string, node*> children; 
    map<string, int> countOfTargetVariable;
    vector<string> labelsTakenTillNow;
    int splitIndex = -1;   

    node(){
        children.clear();
        splitIndex = -1;

        countOfTargetVariable.clear();

        labelsTakenTillNow.clear();
        labelsTakenTillNow.resize(numLabels, "");
    }

    node(vector<string> labelsTakenTillNow){
        children.clear();
        splitIndex = -1;

        countOfTargetVariable.clear();

        this->labelsTakenTillNow = labelsTakenTillNow;
    }

    bool isLeafNode(){
        return children.size() == 0;
    }

    void init(vector<vector<string>> &data){
        for(auto i: data){
            bool canTake = true;
            for(int j = 0; j<4; j++){
                if(labelsTakenTillNow[j] != "" && labelsTakenTillNow[j] != i[j])
                    canTake = false;
            }
            
            if(canTake)
                countOfTargetVariable[i[4]]++;
        }
    }   

    double setSplitIndex(vector<vector<string>> &data){
        vector<int> remainingLabels;

        for(int i = 0; i<4; i++){
            if(labelsTakenTillNow[i] == "")
                remainingLabels.push_back(i);
        }

        if(remainingLabels.empty())
            return 0.0;

        double gini = 1.0;
        int ind = -1;

        for(auto i: remainingLabels){
            map<string, map<string, int>> freq;
            int total = 0;

            for(auto k: data){
                bool canTake = true;
                for(int j = 0; j<4; j++){
                    if(labelsTakenTillNow[j] != "" && labelsTakenTillNow[j] != k[j])
                        canTake = false;
                }

                if(canTake){
                    freq[k[i]][k[4]]++;
                    total++;
                }
            }   

            double gini1 = 0.0;

            for(auto j: freq){  
                double gini2 = calculateGiniIndex(j.second);
                int total2 = 0;
                for(auto k: j.second)
                    total2 += k.second;

                double weight = (double) total2 / (double) total;
                gini1 += gini2 * weight;
            }

            if(gini1 < gini){
                gini = gini1;
                ind = i;
            }
        }

        splitIndex = ind;

        return gini;
    }

    void build(vector<vector<string>> &data){
        init(data);
        double gini = setSplitIndex(data);

        if(gini <= 0.15)
            return;

        for(auto value: labelValues[splitIndex]){
            this->children[value] = new node(this->labelsTakenTillNow);
            this->children[value]->labelsTakenTillNow[splitIndex] = value;
            this->children[value]->build(data);
        }
    }

    string modalValue(){
        int mx = 0;
        string ans = "";

        for(auto i : countOfTargetVariable){
            if(i.second > mx){
                mx = i.second;
                ans = i.first;
            }
        }

        return ans;
    }

    bool exists(string &s){
        return this->children.find(s) != this->children.end();
    }

    string predict(vector<string> &sample){
        if(this->isLeafNode() || !this->exists(sample[splitIndex]))
            return this->modalValue();

        return this->children[sample[splitIndex]]->predict(sample);
    }
};

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

    setLabelValues(trainData);

    node *root = new node();
    root->build(trainData);

    int total = 0;
    int right = 0;

    for(auto i: trainData){
        for(auto j: i)
            cout << j << ' ';
        cout << ": ";
        string prediction = root->predict(i);
        cout << prediction << '\n';

        total++;
        right += (prediction == i[4]);
    }

    cout << "Accuracy: " << ((double) right / (double) total);


}   