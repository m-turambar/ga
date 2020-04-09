#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

const string global_target = "guayabas";

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

float gene_fitness(char c, char target)
{
    if(c==target)
        return 2; //hardcoded, experimenting
    const float dif = (c-target)*(c-target);
    return 1/dif;
}

float individual_fitness(vector<char>& individual)
{
    float sum = 0;
    for(int i=0; i<global_target.size(); ++i)
    {
        sum += gene_fitness(individual[i], global_target[i]);
    }
    return sum;
}

vector<char> mate(vector<char>& female, vector<char>& male)
{
    const auto sz = female.size();
    vector<char> child(sz);
    int x = std::rand()%(sz);
    int n = sz/2;
    int rr = std::rand()%2; //so that the genes of one parent aren't always in the same position
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
vector<vector<char>> crossover(vector<vector<char>>& males, vector<vector<char>>& females)
{
    vector<float> fitness_males(males.size());
    vector<float> fitness_females(females.size());
    vector<vector<char>> offspring(males.size() + females.size());

    for(int k=0; k<males.size(); ++k)
    {
        vector<char>& vec = males[k];
        fitness_males[k] = individual_fitness(vec);
    }

    for(int k=0; k<females.size(); ++k)
    {
        vector<char>& vec = females[k];
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
    vector<float> ffs(sum_males.size());

    /* this time, the females are going to choose. The males will accept any female. */
    //const float largest = *std::max_element(fitness_males.begin(), fitness_males.end());
    for(int i=0; i<females.size(); ++i)
    {
        const float ff = total_fitness*static_cast<float>(std::rand())/static_cast<float>(RAND_MAX);
        ffs[i] = ff;
        /* this approach is O(n), since sum_males is sorted, we could do Log(n), but my brain is fried. */
        for(int j=0; j<fitness_males.size(); ++j)
        {
            const float lower = sum_males[j];
            const float upper = sum_males[j] + fitness_males[j];
            if( ff > lower && ff <= upper )
            {
                offspring[i*2] = mate(females[i], males[j]);
                offspring[i*2+1] = mate(females[i], males[j]);
                break;
            }
        }
    }
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
    vector<vector<char>> pop = create_new_population(100, global_target.size());
    const int pop_size = pop.size();
    for(int i=0; i<10000; ++i)
    {
        vector<vector<char>> males(pop.begin(), pop.begin()+pop_size/2);
        vector<vector<char>> females(pop.begin() + pop_size/2, pop.end());

        pop = crossover(males, females);
        mutate(pop, 0.01);
        cout << "generation" << i << '\n';
        for(auto& e : pop) {
            string s(e.begin(), e.end());
            cout << s << '\n';
            if(s == global_target)
                return 0;
        }
        cout << endl;
    }
    
    return 0;
}