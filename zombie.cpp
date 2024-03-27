// Project Identifier: 9504853406CBAC39EE89AA3AD238AA12CA198043
#include <cstdio>
#include <string.h>
#include <deque>
#include <iostream>
#include <vector>
#include "P2random.h"
#include <getopt.h>
#include <queue>
#include <limits>

using namespace std;

class Game {
private:
    struct zombie{
        string name;
        uint32_t distance;
        uint32_t speed;
        uint32_t health;
        uint32_t rounds_active = 1;
    };

    struct dead_zombie{
        string name;
        uint32_t death_no;
    };

    bool verbose = false;
    bool statistics = false;
    int N = 0;
    bool median = false;

    uint32_t quiver_capacity = 0;
    uint32_t seed = 0;
    uint32_t rand_dist = 0;
    uint32_t rand_speed = 0;
    uint32_t rand_health = 0;

    bool player_alive;


public:
    // Read and process command line arguments.
    void get_options(int argc, char** argv);

    // Read in the CSV music file through stdin.
    void read_header();

    void solver();

    struct eta_compare {
    bool operator()(const zombie *a, const zombie *b){ // work on 
        if((a->distance/a->speed)==(b->distance/b->speed))
            if(a->health==b->health)
                return a->name>b->name;
            else
                return a->health>b->health;
        else
            return (a->distance/a->speed)>(b->distance/b->speed);
    }
};

struct compare_most_active {
    bool operator()(const zombie* a, zombie* b) {
        return a->rounds_active < b->rounds_active || 
              (a->rounds_active == b->rounds_active && a->name > b->name);
    }
};

struct compare_least_active {
    bool operator()(const zombie* a, const zombie* b) {
        return a->rounds_active > b->rounds_active || 
              (a->rounds_active == b->rounds_active && a->name > b->name);
    }
};
};



int main(int argc, char** argv) {
    ios_base::sync_with_stdio(false);

    Game Invasion;
    Invasion.get_options(argc, argv);
    Invasion.read_header();
    Invasion.solver();
    return 0;
}


void Game::get_options(int argc, char** argv) {
    int option_index = 0, option = 0;

    // Don't display getopt error messages about options
    opterr = false;

    struct option longOpts[] = {
        {"statistics", required_argument, nullptr,  's'},
        {  "help",       no_argument, nullptr,  'h'},
        { "verbose",       no_argument, nullptr,  'v'},
        { "median",       no_argument, nullptr,  'm'},
        { nullptr,                 0, nullptr, '\0'}
    };

    while ((option = getopt_long(argc, argv, "s:vmh", longOpts, &option_index)) != -1) {
        switch (option) {
        case 's':
            statistics = true;
            N = atoi(optarg);
            break;
        case 'h':
            cout << "Helpful message..\n";
            exit(0);
        case 'v':
            verbose = true;
            break;
        case 'm':
            median = true;
            break;
        default:
            cerr << "Invalid option!\n";
            exit(1);
        }
    }
}

// Read data into the program through stdin.
void Game::read_header() {
    string line;

    getline(cin,line);
    cin >> line >> quiver_capacity;
    cin >> line >> seed;
    cin >> line >> rand_dist;
    cin >> line >> rand_speed;
    cin >> line >> rand_health;

    P2random::initialize(seed,rand_dist, rand_speed, rand_health);
}

void Game::solver() {

    string line;
    int next_round = 0; // next zombie spawn round
    player_alive = true;
    uint32_t quiver = 0;
    int curr_round = 0;
    bool caught = false;
    string killer_zomb;
    string last_killed;
    uint32_t random_zomb = 0;
    uint32_t named_zomb = 0;
    uint32_t active_zombies = 0;
    bool next_inv = true;



    //deque of all zombies created
    deque<zombie *> master;

    //zombie active master list
    priority_queue<zombie*, vector<zombie*>, eta_compare> alive;

    // dead zombies stored
    deque<zombie *> graveyard;

    // pqs for median calculation
    priority_queue<uint32_t> lower_half;  // max-heap
    priority_queue<uint32_t, vector<uint32_t>, greater<uint32_t>> upper_half;  // min-heap


    //first round
    getline(cin, line); //for blank space, not working without this 
    getline(cin, line);//for delimiter

    // Attempt to read the next line and integer
    cin >> line;
    cin >> next_round;
    while (active_zombies != 0 || next_inv) {
        //step 1
        curr_round++;
        if (verbose) {
            cout << "Round: " << curr_round << "\n";
        }

        //step 2 (refill quiver)
        quiver = quiver_capacity;

        //step 3 (zombies advance)
        priority_queue<zombie*, vector<zombie*>, eta_compare> temp_master;
        for (size_t i = 0; i < alive.size(); i++) {
            zombie* temp = alive.top();
            alive.pop();
            temp->rounds_active++;
            temp->distance -= min(temp->distance, temp->speed);
            if (temp->distance == 0) {
                caught = true; //zombie reached player
            }

            if (verbose) {
                cout << "Moved: " << temp->name << " (distance: " << temp->distance << ", speed: " << temp->speed
                << ", health: " << temp->health << ")\n";
            }
            if (caught && player_alive) {
                player_alive = false;
                killer_zomb = temp->name;
            }
            temp_master.push(temp);
        }
        alive = move(temp_master);

        //step 4 (if dead, game over)
        if (!player_alive) {
            cout << "DEFEAT IN ROUND " << curr_round << "! " << killer_zomb << " ate your brains!\n";
            if (statistics) { 

                cout << "Zombies still active: " << active_zombies << "\n";

                cout << "First zombies killed:\n";
                for(int i = 0; i < min(N, static_cast<int>(graveyard.size())); i++) {
                    cout << graveyard[i]->name << " " << (i + 1) << "\n";
                }
            
                cout << "Last zombies killed:\n";
                for(int i = 0; i < min(N, static_cast<int>(graveyard.size())); i++) {
                    cout << graveyard[graveyard.size() - i - 1]->name << " " << (i + 1) << "\n";
                }

                priority_queue<zombie*, vector<zombie*>, compare_most_active> most_active_pq;
                priority_queue<zombie*, vector<zombie*>, compare_least_active> least_active_pq;

                for(zombie* zomb : master) {
                    most_active_pq.push(zomb);
                    least_active_pq.push(zomb);
                }
                cout << "Most active zombies:\n";
                for(int i = 0; i < N && !most_active_pq.empty(); ++i) {
                    zombie* zomb = most_active_pq.top();
                    most_active_pq.pop();
                    cout << zomb->name << " " << zomb->rounds_active << "\n";
                }
                cout << "Least active zombies:\n";
                for(int i = 0; i < N && !least_active_pq.empty(); ++i) {
                    zombie* zomb = least_active_pq.top();
                    least_active_pq.pop();
                    cout << zomb->name << " " << zomb->rounds_active << "\n";
                }
            }
            for(zombie* zombie_ptr : master) {
                delete zombie_ptr;
            }
            return;
        }

        //step 5 (new zombies)
        if (curr_round == next_round) { //check if the next round spawns zombies
            cin >> line >> random_zomb;
            cin >> line >> named_zomb; 
            for (uint32_t i = 0; i < random_zomb; i++) {
                zombie* temp = new zombie;
                temp->name  = P2random::getNextZombieName();
	            temp->distance = P2random::getNextZombieDistance();
	            temp->speed    = P2random::getNextZombieSpeed();
	            temp->health   = P2random::getNextZombieHealth();
                alive.push(temp);
                master.push_back(temp);
                active_zombies++;
                if (verbose) {
                    cout << "Created: " << temp->name << " (distance: " << temp->distance << ", speed: " << temp->speed
                << ", health: " << temp->health << ")\n";
                }
            } 
            for (uint32_t i = 0; i < named_zomb; i++) {
                zombie* temp = new zombie;
                cin >> temp->name >> line >> temp->distance >> line >> temp->speed >> line >> temp->health;
                // Add the pointer to the priority queue.
                alive.push(temp);
                master.push_back(temp);
                active_zombies++;
                if (verbose) {
                    cout << "Created: " << temp->name << " (distance: " << temp->distance << ", speed: " << temp->speed
                << ", health: " << temp->health << ")\n";
                } 
            }
            getline(cin, line); //for space ig
            getline(cin, line);//for delimiter
            if (!cin) {
                next_inv = false;
                cout << curr_round;
            } else {
                cin >> line; 
                cout << line;
                cout << next_round; // 1
                cout << "Reached Here\n"; 
                cin >> next_round;
                cout << "Not here"; 
                cout << next_round; // 1
            }
    
        }

        //step 6 (shooting)
        while (quiver != 0 && active_zombies != 0) { 
            zombie* temp = alive.top();
            alive.pop();
            temp->health--;
            quiver--;
            if (temp->health == 0) { //zombie killed
                if (verbose) {
                    cout << "Destroyed: " << temp->name << " (distance: " << temp->distance << ", speed: " << temp->speed
                << ", health: " << temp->health << ")\n";
                }
                graveyard.push_back(temp); 
                last_killed = temp->name;
                active_zombies--;

                if (median) {
                    uint32_t rounds_active = temp->rounds_active;
                    if(lower_half.empty() || rounds_active <= lower_half.top()){
                        lower_half.push(rounds_active);
                    }else{
                        upper_half.push(rounds_active);
                    }

                    // Balance the heaps.
                    while(lower_half.size() > upper_half.size() + 1){
                        upper_half.push(lower_half.top());
                        lower_half.pop();
                    }
                    while(upper_half.size() > lower_half.size()){
                        lower_half.push(upper_half.top());
                        upper_half.pop();
                    }
                }
                //delete the pointer from master

            } else { //zombie not killed
                temp_master.push(temp); //can assume temp_master is empty after the last move operation
            }
        }
        alive = move(temp_master);

        //step 7 median flag
        if (median && graveyard.size() > 0) { // check if has to be killed that round or not
        double median_rounds_active;
        if(lower_half.size() == upper_half.size()){
            median_rounds_active = (lower_half.top() + upper_half.top()) / 2.0;
        }else{
            median_rounds_active = lower_half.top();
        }
        cout << "At the end of round " << curr_round << ", the median zombie lifetime is " << median_rounds_active << "\n";
        }
    }

    // step 8 VICTORY
    cout << "VICTORY IN ROUND " << curr_round << "! " << last_killed << " was the last zombie.\n";
    cout << "active zombies:" << active_zombies << " and next_inv: " << next_inv;

    if (statistics) { 

        cout << "Zombies still active: " << active_zombies << "\n";

        cout << "First zombies killed:\n";
        for(int i = 0; i < min(N, static_cast<int>(graveyard.size())); i++) {
            cout << graveyard[i]->name << " " << (i + 1) << "\n";
        }
    
        cout << "Last zombies killed:\n";
        for(int i = 0; i < min(N, static_cast<int>(graveyard.size())); i++) {
            cout << graveyard[graveyard.size() - i - 1]->name << " " << (i + 1) << "\n";
        }

        priority_queue<zombie*, vector<zombie*>, compare_most_active> most_active_pq;
        priority_queue<zombie*, vector<zombie*>, compare_least_active> least_active_pq;

        for(zombie* zomb : master) {
            most_active_pq.push(zomb);
            least_active_pq.push(zomb);
        }
        cout << "Most active zombies:\n";
        for(int i = 0; i < N && !most_active_pq.empty(); ++i) {
            zombie* zomb = most_active_pq.top();
            most_active_pq.pop();
            cout << zomb->name << " " << zomb->rounds_active << "\n";
        }

        cout << "Least active zombies:\n";
        for(int i = 0; i < N && !least_active_pq.empty(); ++i) {
            zombie* zomb = least_active_pq.top();
            least_active_pq.pop();
            cout << zomb->name << " " << zomb->rounds_active << "\n";
        }
    }

    // Cleanup: delete all remaining zombies in 'master'
    for(zombie* zombie_ptr : master) {
        delete zombie_ptr;
    }
    return;
}
