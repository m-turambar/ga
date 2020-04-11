#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "utils.h"

using namespace mk;
using namespace std;

//const string global_target = "guayabas_con_crema_y_fresas";
const string global_target = "guayabas_con_fresas";

float perfect_fitness = 2;
const float mutation_rate = 0.01;
const int n_iters = 10000;
const int starting_pop_size = 1000;
int derivada[32] = {0};
int target_derivative[32] = {0};

ostream& operator<<(ostream& os, vector<char>& vec)
{
    for(char& c : vec)
        os << c;
    os << '\n';
    return os;
}

/* necesitas hacerlo template, o trabajar con size_t y shiftear bits, para el caso general*/
vector<vector<char>> create_new_population(int size_pop, int n_chars)
{
    if(size_pop%2 == 1) /* we want it to be divisible by two */
        size_pop++;
    vector<vector<char>> pop(size_pop);
    //const int limit = std::pow(2,n_bits);
    std::srand(std::time(nullptr)); // use current time as seed for random generator

    for (int n=0; n<size_pop; ++n)
    {
        vector<char> ind(n_chars);
        for(int j=0; j<n_chars; ++j)
        {
            int x = std::rand()%(256);  // Note: 1+rand()%6 is biased
            //std::cout << x << ' ';
            ind[j] = x;
        }
        pop[n] = ind;
    }
    return pop;
}

float gene_fitness(const char c, const char target)
{
    if(c==target)
        return perfect_fitness*2; //hardcoded, experimenting
    const float dif = (c-target)*(c-target);
    //const float dif = abs(c-target);
    return 1/dif;
}

float derivate_fitness(const int d, const int t)
{
    if(d==t)
        return perfect_fitness; //hardcoded, experimenting
    const float dif = (d-t)*(d-t);
    //const float dif = abs(d-t);
    return 1/dif;
}

void update_global_derivative(const vector<char>& individual)
{
    /*we will use a global memory buffer for this, to avoid malloc/free overhead*/
    for(int i=0; i<individual.size()-1; ++i)
    {
        derivada[i] = individual[i+1] - individual[i];
    }
}

void fill_target_derivative()
{
    for(int i=0; i<global_target.size()-1; ++i)
        target_derivative[i] = global_target[i+1] - global_target[i];
}

float individual_fitness(const vector<char>& individual)
{
    float sum = 0;
    update_global_derivative(individual);
    for(int i=0; i<global_target.size(); ++i)
    {
        sum += gene_fitness(individual[i], global_target[i]);
        sum += derivate_fitness(derivada[i], target_derivative[i]);
    }
    return sum;
}

float population_average_fitness(const vector<vector<char>>& population)
{
    float sum = 0;
    for(auto& v : population)
        sum += individual_fitness(v);
    return sum/ static_cast<float>(population.size());
}

vector<char> mate(const vector<char>& female, const vector<char>& male)
{
    const int sz = female.size();
    vector<char> child(sz);
    const int x = std::rand()%(sz);
    const int n = sz/2;
    const int rr = std::rand()%2; //so that the genes of one parent aren't always in the same position
    if(rr == 0) {
        for(int i = 0; i < x; ++i)
            child[i] = female[i];
        for(int i=x; i<x+n; ++i)
            child[i] = male[i];
        for(int i=x+n; i<sz; ++i)
            child[i] = female[i];
    }
    else {
        for(int i = 0; i < x; ++i)
            child[i] = male[i];
        for(int i=x; i<x+n; ++i)
            child[i] = female[i];
        for(int i=x+n; i<sz; ++i)
            child[i] = male[i];
    }
    return child;
}

/* calculate the fittest individuals and cross them over. I'll assume the right half is females, and the left is males.
 * this is a new idea(for me) and I think it will help avoid stagnation. */
vector<vector<char>> crossover(const vector<vector<char>>& males, const vector<vector<char>>& females)
{
    auto t1 = tp();
    vector<float> fitness_males(males.size());
    vector<float> fitness_females(females.size());
    vector<vector<char>> offspring(males.size() + females.size());

    for(int k=0; k<males.size(); ++k)
    {
        const vector<char>& vec = males[k];
        fitness_males[k] = individual_fitness(vec);
    }

    for(int k=0; k<females.size(); ++k)
    {
        const vector<char>& vec = females[k];
        fitness_females[k] = individual_fitness(vec);
    }

    // sort(fitness_males.begin(), fitness_males.end()) //won't work, because the original males still keep their position
    // we would need to introduce a separate data structure to keep track of this :/
    vector<float> sum_males(fitness_males.size());

    float total_fitness=0;
    for(int i=0; i<fitness_males.size(); ++i)
    {
        sum_males[i] = total_fitness;
        total_fitness+=fitness_males[i];
    }
    auto t2 = tp();
    cout << "crossover stage a: " << dur(t1, t2) << '\n';

    t1 = tp();
    /* this time, the females are going to choose. The males will accept any female. */
    //const float largest = *std::max_element(fitness_males.begin(), fitness_males.end());
    const int fsz = fitness_females.size();
    const int msz = fitness_males.size();
    for(int i=0; i<fsz; ++i)
    {
        const float ff = total_fitness*static_cast<float>(std::rand())/static_cast<float>(RAND_MAX);

        /* this approach uses binary search instead. Results are better: dropped from 45us to 33us for stage b for an
         * 8 character string. Dropped from 45 to 35us for a 12 character string. Hitting memory corruption if string
         * is increased to 18 chars. */
        int lb = 0;
        int hb = msz;
        while(true)
        {
            const int idx = (hb+lb)/2;
            const float lower = sum_males[idx];
            const float upper = sum_males[idx] + fitness_males[idx];
            /* Important to add equal to or greater/lesser. Otherwise we can get stuck when values are equal */
            if (ff >= lower && ff <= upper) {
                offspring[i * 2] = mate(females[i], males[idx]);
                offspring[i * 2 + 1] = mate(females[i], males[idx]);
                break;
            }
            else if(ff < lower) {
                hb = idx;
            }
            else {
                lb = idx;
            }
        }


        /* this approach is O(n), since sum_males is sorted, we could do Log(n), but my brain is fried. */
        /*
        for(int j=0; j<msz; ++j) {
            const float lower = sum_males[j];
            const float upper = sum_males[j] + fitness_males[j];
            if (ff > lower && ff <= upper) {
                offspring[i * 2] = mate(females[i], males[j]);
                offspring[i * 2 + 1] = mate(females[i], males[j]);
                break;
            }
        }
        */

    }

    t2 = tp();
    cout << "crossover stage b: " << dur(t1, t2) << '\n';
    return offspring;
}

void mutate(vector<vector<char>>& pop, float probability)
{
    for(auto& e : pop)
    {
        for(auto& c : e)
        {
            const float p = static_cast<float>(std::rand())/static_cast<float>(RAND_MAX);
            if(p <= probability)
                c = std::rand()%(256);
        }
    }
}

int main()
{
    fill_target_derivative();
    vector<vector<char>> pop = create_new_population(starting_pop_size, global_target.size());
    const int pop_size = pop.size();
    for(int i=0; i<=n_iters; ++i)
    {
        auto t1 = tp();
        vector<vector<char>> males(pop.begin(), pop.begin()+pop_size/2);
        vector<vector<char>> females(pop.begin() + pop_size/2, pop.end());
        auto t2 = tp();
        cout << "time to split vectors: " << dur(t1, t2) << '\n';

        //t1 = tp();
        pop = crossover(males, females);
        //t2 = tp();
        //cout << "time for crossover: " << dur(t1, t2) << '\n';

        t1 = tp();
        mutate(pop, mutation_rate);
        t2 = tp();
        cout << "time for mutation: " << dur(t1, t2) << '\n';

        cout << "generation " << i << '\n';
        for(auto& e : pop) {
            string s(e.begin(), e.end());
            if(i%1000==0)
                cout << s << '\n';
            if(s == global_target)
            {
                cout << "Solucion encontrada en la generacion " << i << " : " << s << endl;
                return 0;
            }

        }
        if(i%1000==0)
        {
            cout << "population average fitness: " << population_average_fitness(pop) << '\n';
            auto temp = perfect_fitness;
            perfect_fitness = 2;
            cout << "normalized fitness: " << population_average_fitness(pop) << '\n';
            perfect_fitness = temp;
        }

        cout << endl;
    }

    return 0;
}