// Simple simulated annealing for an 8-bit S-box.
// Build: g++ -O2 -o sbox_simple sbox_simple.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <random>
using namespace std;

const int N = 256;

void fwht(int f[N]) {
    for (int len = 1; len < N; len *= 2)
        for (int i = 0; i < N; i += 2 * len)
            for (int j = i; j < i + len; j++) {
                int u = f[j], v = f[j + len];
                f[j] = u + v;
                f[j + len] = u - v;
            }
}

double evaluate(const vector<int>& S, int& maxW) {
    double cost = 0;
    maxW = 0;
    for (int b = 1; b < N; b++) {              
        int f[N];
        for (int x = 0; x < N; x++)
            f[x] = (__builtin_popcount(b & S[x]) & 1) ? -1 : 1;
        fwht(f);
        for (int a = 0; a < N; a++) {
            int w = abs(f[a]);
            maxW = max(maxW, w);
            cost += (double)w * w * w;
        }
    }
    return cost;
}

int main() {
    mt19937 rng(12345);
    uniform_int_distribution<int> pick(0, N - 1);
    uniform_real_distribution<double> uni(0.0, 1.0);

    vector<int> S(N);
    for (int i = 0; i < N; i++) S[i] = i;
    shuffle(S.begin(), S.end(), rng);

    int maxW;
    double cur = evaluate(S, maxW);

    double T = 2e6;            
    double cooling = 0.9995; 
    int iterations = 300000;

    vector<int> best = S;
    double bestCost = cur;

    for (int it = 0; it < iterations; it++) {
        int i = pick(rng), j = pick(rng);
        if (i == j) continue;
        swap(S[i], S[j]);

        double next = evaluate(S, maxW);
        double delta = next - cur;

        if (delta < 0 || uni(rng) < exp(-delta / T)) {
            cur = next;
            if (cur < bestCost) { bestCost = cur; best = S; }
        } else {
            swap(S[i], S[j]);  
        }
        T *= cooling;

        if (it % 3000 == 0) {
            evaluate(S, maxW);
            cout << "iter " << it << "  nonlinearity = " << 128 - maxW / 2 << endl;
        }
    }

    evaluate(best, maxW);
    cout << "\nBest nonlinearity: " << 128 - maxW / 2 << "\n\nS-box:\n";
    for (int i = 0; i < N; i++)
        printf("0x%02X,%s", best[i], (i % 16 == 15) ? "\n" : " ");
}