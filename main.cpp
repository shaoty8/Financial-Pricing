#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <cfloat>

using namespace std;

#define four_year 40000

vector<string> date_v;
vector<string> time_v;
vector<float> open_v;
vector<float> high_v;
vector<float> low_v;
vector<float> close_v;

enum Status {
    UNINVESTED = 1,
    BOUGHT = 2,
    SOLD = 3
};

float extreme_in_v(vector<float> v, int l, int r, bool indicator) {
    float tmp = indicator ? -10000000.0 : 10000000.0;
    for (int i = l ; i < r+1; i++) {
        tmp = indicator ? max(tmp, v[i]) : min(tmp, v[i]);
    }
    return tmp;
}

float trade(int tau, float stppct, int start, int end) {
    Status status = UNINVESTED;
    float prepeak = 0, prethrough = 0;
    float money = 100000.0, wholepeak = 0;
    float equity, underwater, maxDD = 0;
    int amount = 0;
    
    for(int i = tau+start; i < end-1; i++) {
        float highest_hi = extreme_in_v(high_v, i-tau, i-1, true);
        float lowest_low = extreme_in_v(low_v, i-tau, i-1, false);
        switch (status) {
            case UNINVESTED: {
                if (close_v[i] > highest_hi) {
                    amount = money / open_v[i+1];
                    money = money - open_v[i+1] * amount;
                    status = BOUGHT;
                    prepeak = open_v[i+1];
                } else if (close_v[i] < lowest_low) {
                    amount = -1 * (money / open_v[i+1]);
                    money = money - open_v[i+1] * amount;
                    status = SOLD;
                    prethrough = open_v[i+1];
                }
                
                if (i >= 9999 && i < 10010) {
                    cout << "amount: " << amount << endl;
                }
                
                break;
            }
            case BOUGHT: {
                if (close_v[i] > prepeak) {
                    prepeak = close_v[i];
                }
                if (close_v[i] < prepeak * (1-stppct)) {
                    money = money + amount * open_v[i+1];
                    amount = 0;
                    status = UNINVESTED;
                }
                break;
            }
            case SOLD: {
                if (close_v[i] < prethrough) {
                    prethrough = close_v[i];
                }
                if (close_v[i] > prethrough * (1+stppct)) {
                    money = money + amount * open_v[i+1];
                    amount = 0;
                    status = UNINVESTED;
                }
            }
        }
        
        equity = money + amount * close_v[i];
        wholepeak = max(wholepeak, equity - 100000);
        underwater = equity - wholepeak - 100000;
        maxDD = min(maxDD, underwater);
    }
    
    //calculate the output
    float NPWD = -1 * (equity - 100000) / maxDD;
    return NPWD;
}

void trade_loop(){
    int start = 0, end;
    while(start<date_v.size()){
        if (start + four_year < date_v.size()) {
            end = start + four_year;
        }
        else {
            end = date_v.size() - 1;
        }
        float best = -100000.0;
        int best_tau = 0;
        float best_stppct = 0;
        for (int tau = 500; tau <= 10000; tau += 1000) {
            for(float stppct = 0.005; stppct <= 0.1; stppct += 0.01){
                float NPWD = trade(tau, stppct, start, end);
                if (best < NPWD) {
                    best = NPWD;
                    best_stppct = stppct;
                    best_tau = tau;
                }
            }
        }
        cout << "Best tau with start " << start << " is " << best_tau<<endl;
        cout << "Best st with start " << start << " is " << best_stppct << endl;
        start = start + four_year;
    }
}

int main(int argc, const char* argv[]) {
    ifstream ifs("WC-5min.asc");
    if (!ifs) {
        cout << "File open failed\n";
    }
    string line, tmp_str;
    stringstream ss;
    getline(ifs, line, '\n');
    
    int count = 0;
    
    while (ifs && count < 40000) {
        count++;
        getline(ifs, line, '\n');
        stringstream ss(line);
        getline(ss, tmp_str, ',');
        date_v.push_back(tmp_str);
        getline(ss, tmp_str, ',');
        time_v.push_back(tmp_str);
        getline(ss, tmp_str, ',');
        open_v.push_back(stof(tmp_str));
        getline(ss, tmp_str, ',');
        high_v.push_back(stof(tmp_str));
        getline(ss, tmp_str, ',');
        low_v.push_back(stof(tmp_str));
        getline(ss, tmp_str, ',');
        close_v.push_back(stof(tmp_str));
    }
    cout << trade(500, 0.02, 0, four_year) << endl;
    //trade_loop();
    return 0;
}
