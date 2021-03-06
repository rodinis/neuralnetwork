#include <iostream>
#include <iomanip>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#define DEF_MOMENT 0.0001
#define DEF_SPEED 0.0001

using namespace std;

enum{
    OUT_L = 1,
    HID_L = 0
};

double error(double *output, double *solution, int n){
    double error = 0, delta;
    for(int i = 0; i < n; ++i){
        delta = output[i] - solution[i];
        error += delta * delta;
    }
    return error / 2;
}

double sigmoid(double input){
    return 1 / (1 + exp(-input));
}

double *normalize(double *input, int n){
        double *norm = new double [n];
        for (int i = 0; i < n; ++i){
            norm[i] = 1 / input[i];
        }
        return norm;
}

class Network{
    double ***weight;
    double ***dweight;
    double **output;
    int *neurons;
    int layers;
    double moment;
    double speed;
public:
    Network(int l, int *n, double m = DEF_MOMENT, double s = DEF_SPEED){
        layers = l;
        moment = m;
        speed = s;
        neurons = new int [layers];
        for(int i = 0; i < layers; ++i){
            neurons[i] = n[i];
        }
        srand(time(NULL));
        weight = new double** [layers];
        dweight = new double** [layers];
        for (int i = 1; i < layers; ++i){
            weight[i] = new double* [neurons[i]];
            dweight[i] = new double* [neurons[i]];
            for (int j = 0; j < neurons[i]; ++j){
                weight[i][j] = new double [neurons[i - 1]];
                dweight[i][j] = new double [neurons[i - 1]];
                for (int k = 0; k < neurons[i - 1]; ++k){
                    weight[i][j][k] = 0.1 * rand() / (RAND_MAX + 1.0) + 0.5;
                    dweight[i][j][k] = 0;
                }
            }
        }
        output = new double* [layers];
        for (int i = 0; i < layers; ++i){
            output[i] = new double [neurons[i]];
        }
    }
    Network(const char *filename){
        ifstream in(filename);
        in >> layers;
        neurons = new int [layers];
        for (int i = 0; i < layers; ++i){
            in >> neurons[i];
        }
        in >> moment >> speed;
        weight = new double** [layers];
        dweight = new double** [layers];
        for (int i = 1; i < layers; ++i){
            weight[i] = new double* [neurons[i]];
            dweight[i] = new double* [neurons[i]];
            for (int j = 0; j < neurons[i]; ++j){
                weight[i][j] = new double [neurons[i - 1]];
                dweight[i][j] = new double [neurons[i - 1]];
                for (int k = 0; k < neurons[i - 1]; ++k){
                    in >> weight[i][j][k];
                    dweight[i][j][k] = 0;
                }
            }
        }
        output = new double* [layers];
        for (int i = 0; i < layers; ++i){
            output[i] = new double [neurons[i]];
        }
        in.close();
    }
    void save(const char *filename){
        ofstream out(filename);
        out << layers << endl;
        for (int i = 0; i < layers; ++i){
            out << neurons[i] << " ";
        }
        out << endl << moment << endl << speed << endl;
        for (int i = 1; i < layers; ++i){
            for (int j = 0; j < neurons[i]; ++j){
                for (int k = 0; k < neurons[i - 1]; ++k){
                    out << weight[i][j][k] << " ";
                }
                out << endl;
            }
        }
        out.close();
    }
    double *learn(double *input, double *solution){
        double *norm = normalize(input, neurons[0]);
        double *answer = solve(norm);
        delete[] norm;
        backpropagation(solution);
        return answer;
    }
    double *solve(double *input) const {
        double sum;
        for (int i = 0; i < neurons[0]; ++i){
            output[0][i] = input[i];
        }
        for (int i = 1; i < layers; ++i){
            for (int j = 0; j < neurons[i]; ++j){
                sum = 0;
                for (int k = 0; k < neurons[i - 1]; ++k){
                    sum += output[i - 1][k] * weight[i][j][k];
                }
                output[i][j] = sigmoid(sum);
                cout << sum << endl;
            }
        }
        return output[layers - 1];
    }
    void backpropagation(double *solution){
        //current layer index
        int cli = layers - 1;
        double sum;
        //allocting deltas array
        double **delta = new double* [layers];
        for (int i = 0; i < layers; ++i){
            delta[i] = new double [neurons[i]];
        }
        //computing deltal on output layer
        for (int i = 0; i < neurons[cli]; ++i){
            delta[cli][i] = -1 * output[cli][i] * (1 - output[cli][i]) * (solution[i] - output[cli][i]);
        }
        //computing deltas on hidden layers
        while (--cli){
            for (int i = 0; i < neurons[cli]; ++i){
                sum = 0;
                for (int j = 0; j < neurons[cli + 1]; ++j){
                    sum += delta[cli + 1][j] * weight[cli][j][i];
                }
                delta[cli][i] = output[cli][i] * (1 - output[cli][i]) * sum;
            }
        }
        //recomputing weights
        while(++cli < layers){
            for (int i = 0; i < neurons[cli]; ++i){
                for (int j = 0; j < neurons[cli - 1]; ++j){
                    dweight[cli][i][j] = moment * dweight[cli][i][j] + (1 - moment) * speed * delta[cli][i] * output[cli - 1][j];
                    weight[cli][i][j] += dweight[cli][i][j];
                }
            }
        }
        // deleting memory of delta array
        for (int i = 0; i < layers; ++i){
            delete[] delta[i];
        }
        delete[] delta;
    }
    ~Network(){
        for (int i = 1; i < layers; ++i){
            for (int j = 0; j < neurons[i]; ++j){
                delete[] weight[i][j];
                delete[] dweight[i][j];
            }
            delete[] weight[i];
            delete[] dweight[i];
        }
        delete[] weight;
        delete[] dweight;
        for (int i = 0; i < layers; ++i){
            delete[] output[i];
        }
        delete[] output;
        delete[] neurons;
    }
};

void teach(Network &net, const char *filename, const char *logname = "log.txt"){
    ifstream in(filename);
    ofstream log(logname);
    int tests_amt, input_dim, output_dim;
    in >> tests_amt >> input_dim >> output_dim;
    double *input = new double [input_dim];
    double *output = new double [output_dim];
    double *out;
    for (int i = 0; i < tests_amt; ++i){
        for (int j = 0; j < input_dim; ++j){
            in >> input[j]; 
        }
        for (int j = 0; j < output_dim; ++j){
            in >> output[j];
        }
        out = net.learn(input, output);
        log << error(out, output, output_dim) << endl;
    }
    log.close();
    in.close();
}

int main()
{
    int l = 4;
    int n[l] = {2,3,2,1};
    Network net(l, n);
    teach(net, "tests.txt");
    //Network net("net.txt");
    //double in[2] = {10.0, 228.0};
    //double *out;
    //out = net.solve(in);
    //cout << out[0] << endl << out[1] << endl << out[2] << endl;
    net.save("net.txt");
    return 0;
}
