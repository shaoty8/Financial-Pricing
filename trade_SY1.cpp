#include <time.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <cfloat>
#include <deque>

using namespace std;

vector<string> date_v;
vector<string> time_v;
vector<float> open_v;
vector<float> high_v;
vector<float> low_v;
vector<float> close_v;

vector<float> highest_v;
vector<float> lowest_v;

enum Status {
    UNINVESTED = 1,
    BOUGHT = 2,
    SOLD = 3
};

const float slippage = 36;

float trade(int tau, float stppct, int end) {
    Status status = UNINVESTED;
    float prepeak = 0, prethrough = 0; 
    float money = 100000.0, wholepeak = 0;
    float equity, underwater, maxDD = 0;
    int amount = 0;

    for(int i = tau; i < end-1; i++) {
        float highest_hi = highest_v[i-tau];
        float lowest_low = lowest_v[i-tau];
        switch (status) {
            case UNINVESTED: {
                if (close_v[i] > highest_hi) {
                    amount = money / (open_v[i+1]*50);
                    money = money - open_v[i+1] *50* amount;
                    status = BOUGHT;
                    prepeak = open_v[i+1];
                } else if (close_v[i] < lowest_low) {
                    amount = -1 * (money / open_v[i+1]/50);
                    money = money - open_v[i+1] *50* amount;
                    status = SOLD;
                    prethrough = open_v[i+1];
                }
                break;
            } 
            case BOUGHT: {
                if (close_v[i] > prepeak) {
                    prepeak = close_v[i];
                }
                if (close_v[i] < prepeak * (1-stppct)) {
                    money = money + amount * (open_v[i+1]*50 - slippage);
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
                    money = money + amount * (open_v[i+1]*50 + slippage);
                    amount = 0;
                    status = UNINVESTED;
                }
            }
        }

        equity = money + amount * close_v[i]*50;
        wholepeak = max(wholepeak, equity - 100000);
        underwater = equity - wholepeak - 100000;
        maxDD = min(maxDD, underwater);
    }
    
    //calculate the output
    float NPWD = -1 * (equity - 100000) / maxDD;
    return NPWD;
}

vector<float> maxSlidingWindow(vector<float> input, int tau) {
    vector<float> res;
    deque<float> Q;
    int n = input.size();

    for (int i = 0; i < tau; i++) {
        while (!Q.empty() && input[i] > input[Q.back()]) {
            Q.pop_back();
        }
        Q.push_back(i);
    }

    for (int i = tau; i < n; i++) {
        res.push_back(input[Q.front()]);
        while (!Q.empty() && input[i] >= input[Q.back()]) {
            Q.pop_back();
        }
        while (!Q.empty() && Q.front() <= i-tau) {
            Q.pop_front();
        }
        Q.push_back(i);
    }
    res.push_back(input[Q.front()]);
    return res;
}

vector<float> minSlidingWindow(vector<float> input, int tau) {
    vector<float> res;
    deque<float> Q;
    int n = input.size();

    for (int i = 0; i < tau; i++) {
        while (!Q.empty() && input[i] < input[Q.back()]) {
            Q.pop_back();
        }
        Q.push_back(i);
    }

    for (int i = tau; i < n; i++) {
        res.push_back(input[Q.front()]);
        while (!Q.empty() && input[i] <= input[Q.back()]) {
            Q.pop_back();
        }
        while (!Q.empty() && Q.front() <= i-tau) {
            Q.pop_front();
        }
        Q.push_back(i);
    }
    res.push_back(input[Q.front()]);
    return res;
}

int main(int argc, const char* argv[]) {
    ifstream ifs("SY-5min.asc");
    if (!ifs) {
        cout << "File open failed\n";
    }
    string line, tmp_str;
    stringstream ss;
    getline(ifs, line, '\n');

    int start_line = 247048, end_line = 291957;
    int count = 0;

    while (count < start_line-1) {
        getline(ifs, line, '\n');
        count++;
    }

    while (ifs && count < end_line) {
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

    int tau = 500;
    float best = -100000.0;
    int best_tau = 0;
    float best_stppct = 0;

    clock_t t1 = clock();

    while(tau <= 10000) {
        highest_v = maxSlidingWindow(high_v, tau);
        lowest_v = minSlidingWindow(low_v, tau);
        for(float stppct = 0.005; stppct <= 0.1; stppct += 0.001) {
            float NPWD = trade(tau, stppct, end_line-start_line+1);
            cout << NPWD << " ";
            if (best < NPWD) {
                best = NPWD;
                best_stppct = stppct;
                best_tau = tau;
            }
        }
        cout << endl;
        tau += 10;
    }

    cout << "Best tau with start 0 is " << best_tau << endl;
    cout << "Best st with start 0 is " << best_stppct << endl;

    clock_t t2 = clock();
    
    cout << "Elapsed time: " << ((float)(t2 - t1))/CLOCKS_PER_SEC << endl;

    highest_v = maxSlidingWindow(high_v, 5000);
    lowest_v = minSlidingWindow(low_v, 5000);
    cout << "500, 0.02: " << trade(5000, 0.04, end_line-start_line+1) << endl;

    return 0;
}
