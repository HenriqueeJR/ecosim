#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include <thread>
#include <mutex>


static const uint32_t NUM_ROWS = 15;

// Constants
const uint32_t PLANT_MAXIMUM_AGE = 10;
const uint32_t HERBIVORE_MAXIMUM_AGE = 50;
const uint32_t CARNIVORE_MAXIMUM_AGE = 80;
const uint32_t MAXIMUM_ENERGY = 200;
const uint32_t THRESHOLD_ENERGY_FOR_REPRODUCTION = 20;

// Probabilities
const double PLANT_REPRODUCTION_PROBABILITY = 0.2;
const double HERBIVORE_REPRODUCTION_PROBABILITY = 0.075;
const double CARNIVORE_REPRODUCTION_PROBABILITY = 0.025;
const double HERBIVORE_MOVE_PROBABILITY = 0.7;
const double HERBIVORE_EAT_PROBABILITY = 0.9;
const double CARNIVORE_MOVE_PROBABILITY = 0.5;
const double CARNIVORE_EAT_PROBABILITY = 1.0;

// Type definitions
enum entity_type_t
{
    empty,
    plant,
    herbivore,
    carnivore
};

struct pos_t
{
    uint32_t i;
    uint32_t j;
};

struct entity_t
{
    entity_type_t type;
    int32_t energy;
    int32_t age;
    bool already_iterated;
    std::mutex* m;
};

// Auxiliary code to convert the entity_type_t enum to a string
NLOHMANN_JSON_SERIALIZE_ENUM(entity_type_t, {
                                                {empty, " "},
                                                {plant, "P"},
                                                {herbivore, "H"},
                                                {carnivore, "C"},
                                            })

// Auxiliary code to convert the entity_t struct to a JSON object
namespace nlohmann
{
    void to_json(nlohmann::json &j, const entity_t &e)
    {
        j = nlohmann::json{{"type", e.type}, {"energy", e.energy}, {"age", e.age}};
    }
}

// Grid that contains the entities
static std::vector<std::vector<entity_t>> entity_grid;

bool random_action(float probability) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) < probability;
}

void simulate_plant(int i, int j) {

    std::lock_guard<std::mutex> lock1(*entity_grid[i][j].m);
    if(i+1 < 15) std::lock_guard<std::mutex> lock2(*entity_grid[i+1][j].m);
    if(i-1 >= 0) std::lock_guard<std::mutex> lock3(*entity_grid[i-1][j].m);
    if(j+1 < 15) std::lock_guard<std::mutex> lock4(*entity_grid[i][j+1].m);
    if(j-1 >= 0) std::lock_guard<std::mutex> lock5(*entity_grid[i][j-1].m);


    std::vector<pos_t> possible_growth_positions;

    if(i+1 < 15) {
        if (entity_grid[i+1][j].type == empty) {
        pos_t possible_position;
        possible_position.i = i+1;
        possible_position.j = j;
        possible_growth_positions.push_back(possible_position);
        }
    }
    
    if(i-1 >= 0) {
        if (entity_grid[i-1][j].type == empty) {
        pos_t possible_position;
        possible_position.i = i-1;
        possible_position.j = j;
        possible_growth_positions.push_back(possible_position);
        }
    }
    
    if(j+1 < 15) {
        if (entity_grid[i][j+1].type == empty) {
        pos_t possible_position;
        possible_position.i = i;
        possible_position.j = j+1;
        possible_growth_positions.push_back(possible_position);
        }
    }
    
    if(j-1 >= 0) {
        if (entity_grid[i][j-1].type == empty) {
        pos_t possible_position;
        possible_position.i = i;
        possible_position.j = j-1;
        possible_growth_positions.push_back(possible_position);
        }
    }
    

    if (!possible_growth_positions.empty()) {
        if(random_action(PLANT_REPRODUCTION_PROBABILITY)) {
            std::random_device rd;  
            std::mt19937 generator(rd());  
            std::uniform_int_distribution<int> distribution(0, possible_growth_positions.size() - 1);
            
            pos_t selected_growth_position = possible_growth_positions[distribution(generator)];
            entity_grid[selected_growth_position.i][selected_growth_position.j].type = plant;
            entity_grid[selected_growth_position.i][selected_growth_position.j].energy = 0;
            entity_grid[selected_growth_position.i][selected_growth_position.j].age = 0;
            entity_grid[selected_growth_position.i][selected_growth_position.j].already_iterated = true;
        }

    }

    entity_grid[i][j].age += 1;  //increase age

    if (entity_grid[i][j].age == PLANT_MAXIMUM_AGE) {  //decompose
        entity_grid[i][j].type = empty;
        entity_grid[i][j].energy = 0;
        entity_grid[i][j].age = 0;
    }

}

void simulate_herbivore(int i, int j) {

    std::lock_guard<std::mutex> lock1(*entity_grid[i][j].m);
    if(i+1 < 15) std::lock_guard<std::mutex> lock2(*entity_grid[i+1][j].m);
    if(i-1 >= 0) std::lock_guard<std::mutex> lock3(*entity_grid[i-1][j].m);
    if(j+1 < 15) std::lock_guard<std::mutex> lock4(*entity_grid[i][j+1].m);
    if(j-1 >= 0) std::lock_guard<std::mutex> lock5(*entity_grid[i][j-1].m);
    
    std::vector<pos_t> empty_neighbours;
    std::vector<pos_t> plant_neighbours;

    if(i+1 < 15) {
        
        if (entity_grid[i+1][j].type == empty) {   
            pos_t possible_position;
            possible_position.i = i+1;
            possible_position.j = j;
            empty_neighbours.push_back(possible_position);
        }
        else if (entity_grid[i+1][j].type == plant) {   
            pos_t possible_position;
            possible_position.i = i+1;
            possible_position.j = j;
            plant_neighbours.push_back(possible_position);

            
        }
    }
    
    if(i-1 >= 0) {
        
        if (entity_grid[i-1][j].type == empty) {
            pos_t possible_position;
            possible_position.i = i-1;
            possible_position.j = j;
            empty_neighbours.push_back(possible_position);
        }
        else if (entity_grid[i-1][j].type == plant) {   
            pos_t possible_position;
            possible_position.i = i-1;
            possible_position.j = j;
            plant_neighbours.push_back(possible_position);
        }
    }
    
    if(j+1 < 15) {
        
        if (entity_grid[i][j+1].type == empty) {
            pos_t possible_position;
            possible_position.i = i;
            possible_position.j = j+1;
            empty_neighbours.push_back(possible_position);
        }
        else if (entity_grid[i][j+1].type == plant) {   
            pos_t possible_position;
            possible_position.i = i;
            possible_position.j = j+1;
            plant_neighbours.push_back(possible_position);
        }
    }
    
    if(j-1 >= 0) {
        
        if (entity_grid[i][j-1].type == empty) {
            pos_t possible_position;
            possible_position.i = i;
            possible_position.j = j-1;
            empty_neighbours.push_back(possible_position);
        }
        else if (entity_grid[i][j-1].type == plant) {   
            pos_t possible_position;
            possible_position.i = i;
            possible_position.j = j-1;
            plant_neighbours.push_back(possible_position);
        }
    }

    if (!plant_neighbours.empty()) {
        if (random_action(HERBIVORE_EAT_PROBABILITY)) {  //eating

            std::random_device rd;  
            std::mt19937 generator(rd());  
            std::uniform_int_distribution<int> distribution(0, plant_neighbours.size() - 1);
            
            pos_t selected_eat_position = plant_neighbours[distribution(generator)];
            entity_grid[selected_eat_position.i][selected_eat_position.j].type = herbivore;
            entity_grid[selected_eat_position.i][selected_eat_position.j].energy = entity_grid[i][j].energy + 30;
            entity_grid[selected_eat_position.i][selected_eat_position.j].age = entity_grid[i][j].age;
            entity_grid[selected_eat_position.i][selected_eat_position.j].already_iterated = true;

            entity_grid[i][j].type = empty;
            entity_grid[i][j].energy = 0;
            entity_grid[i][j].age = 0;

            entity_grid[selected_eat_position.i][selected_eat_position.j].age += 1;

        }
        else entity_grid[i][j].age += 1;
    }

    else if (!empty_neighbours.empty()) {    //reproduction
        if(entity_grid[i][j].energy > THRESHOLD_ENERGY_FOR_REPRODUCTION) {
            if (random_action(HERBIVORE_REPRODUCTION_PROBABILITY)) {
                entity_grid[i][j].energy -= 10;

                std::random_device rd;  
                std::mt19937 generator(rd());  
                std::uniform_int_distribution<int> distribution(0, empty_neighbours.size() - 1);
                
                pos_t selected_birth_position = empty_neighbours[distribution(generator)];
                entity_grid[selected_birth_position.i][selected_birth_position.j].type = herbivore;
                entity_grid[selected_birth_position.i][selected_birth_position.j].energy = 100;
                entity_grid[selected_birth_position.i][selected_birth_position.j].age = 0;
                entity_grid[selected_birth_position.i][selected_birth_position.j].already_iterated = true;

            }
            
        }
        entity_grid[i][j].age += 1;
    }


    else if (!empty_neighbours.empty()) {                               //movement
        if(random_action(HERBIVORE_MOVE_PROBABILITY)) {
            std::random_device rd;  
            std::mt19937 generator(rd());  
            std::uniform_int_distribution<int> distribution(0, empty_neighbours.size() - 1);
            
            pos_t selected_move_position = empty_neighbours[distribution(generator)];
            entity_grid[selected_move_position.i][selected_move_position.j].type = herbivore;
            entity_grid[selected_move_position.i][selected_move_position.j].energy = entity_grid[i][j].energy - 5;
            entity_grid[selected_move_position.i][selected_move_position.j].age = entity_grid[i][j].age;
            entity_grid[selected_move_position.i][selected_move_position.j].already_iterated = true;

            entity_grid[i][j].type = empty;
            entity_grid[i][j].energy = 0;
            entity_grid[i][j].age = 0;

            entity_grid[selected_move_position.i][selected_move_position.j].age += 1;
        }
        else entity_grid[i][j].age += 1;
    }

    

    if(entity_grid[i][j].energy == 0 ||          //death
        entity_grid[i][j].age == HERBIVORE_MAXIMUM_AGE) {

            entity_grid[i][j].type = empty;
            entity_grid[i][j].energy = 0;
            entity_grid[i][j].age = 0;
    }                
}

void simulate_carnivore(int i, int j) {

    std::lock_guard<std::mutex> lock1(*entity_grid[i][j].m);
    if(i+1 < 15) std::lock_guard<std::mutex> lock2(*entity_grid[i+1][j].m);
    if(i-1 >= 0) std::lock_guard<std::mutex> lock3(*entity_grid[i-1][j].m);
    if(j+1 < 15) std::lock_guard<std::mutex> lock4(*entity_grid[i][j+1].m);
    if(j-1 >= 0) std::lock_guard<std::mutex> lock5(*entity_grid[i][j-1].m);


    std::vector<pos_t> empty_neighbours;
    std::vector<pos_t> herbivore_neighbours;

    if(i+1 < 15) {
        if (entity_grid[i+1][j].type == empty) {   
            pos_t possible_position;
            possible_position.i = i+1;
            possible_position.j = j;
            empty_neighbours.push_back(possible_position);
        }
        else if (entity_grid[i+1][j].type == herbivore) {   
            pos_t possible_position;
            possible_position.i = i+1;
            possible_position.j = j;
            herbivore_neighbours.push_back(possible_position);

            
        }
    }
    
    if(i-1 >= 0) {
        if (entity_grid[i-1][j].type == empty) {
            pos_t possible_position;
            possible_position.i = i-1;
            possible_position.j = j;
            empty_neighbours.push_back(possible_position);
        }
        else if (entity_grid[i-1][j].type == herbivore) {   
            pos_t possible_position;
            possible_position.i = i-1;
            possible_position.j = j;
            herbivore_neighbours.push_back(possible_position);
        }
    }
    
    if(j+1 < 15) {
        if (entity_grid[i][j+1].type == empty) {
            pos_t possible_position;
            possible_position.i = i;
            possible_position.j = j+1;
            empty_neighbours.push_back(possible_position);
        }
        else if (entity_grid[i][j+1].type == herbivore) {   
            pos_t possible_position;
            possible_position.i = i;
            possible_position.j = j+1;
            herbivore_neighbours.push_back(possible_position);
        }
    }
    
    if(j-1 >= 0) {
        if (entity_grid[i][j-1].type == empty) {
            pos_t possible_position;
            possible_position.i = i;
            possible_position.j = j-1;
            empty_neighbours.push_back(possible_position);
        }
        else if (entity_grid[i][j-1].type == herbivore) {   
            pos_t possible_position;
            possible_position.i = i;
            possible_position.j = j-1;
            herbivore_neighbours.push_back(possible_position);
        }
    }

    if (!herbivore_neighbours.empty()) {
        if (random_action(CARNIVORE_EAT_PROBABILITY)) {  //eating

            std::random_device rd;  
            std::mt19937 generator(rd());  
            std::uniform_int_distribution<int> distribution(0, herbivore_neighbours.size() - 1);
            
            pos_t selected_eat_position = herbivore_neighbours[distribution(generator)];
            entity_grid[selected_eat_position.i][selected_eat_position.j].type = carnivore;
            entity_grid[selected_eat_position.i][selected_eat_position.j].energy = entity_grid[i][j].energy + 20;
            entity_grid[selected_eat_position.i][selected_eat_position.j].age = entity_grid[i][j].age;
            entity_grid[selected_eat_position.i][selected_eat_position.j].already_iterated = true;

            entity_grid[i][j].type = empty;
            entity_grid[i][j].energy = 0;
            entity_grid[i][j].age = 0;

            entity_grid[selected_eat_position.i][selected_eat_position.j].age += 1;

        }
        else entity_grid[i][j].age += 1;
    }

    else if (!empty_neighbours.empty()) {    //reproduction
        if(entity_grid[i][j].energy > THRESHOLD_ENERGY_FOR_REPRODUCTION) {
            if (random_action(CARNIVORE_REPRODUCTION_PROBABILITY)) {
                entity_grid[i][j].energy -= 10;

                std::random_device rd;  
                std::mt19937 generator(rd());  
                std::uniform_int_distribution<int> distribution(0, empty_neighbours.size() - 1);
                
                pos_t selected_birth_position = empty_neighbours[distribution(generator)];
                entity_grid[selected_birth_position.i][selected_birth_position.j].type = carnivore;
                entity_grid[selected_birth_position.i][selected_birth_position.j].energy = 100;
                entity_grid[selected_birth_position.i][selected_birth_position.j].age = 0;
                entity_grid[selected_birth_position.i][selected_birth_position.j].already_iterated = true;
            }
            
        }
        entity_grid[i][j].age += 1;
    }


    else if (!empty_neighbours.empty()) {                               //movement
        if(random_action(CARNIVORE_MOVE_PROBABILITY)) {
            std::random_device rd;  
            std::mt19937 generator(rd());  
            std::uniform_int_distribution<int> distribution(0, empty_neighbours.size() - 1);
            
            pos_t selected_move_position = empty_neighbours[distribution(generator)];
            entity_grid[selected_move_position.i][selected_move_position.j].type = carnivore;
            entity_grid[selected_move_position.i][selected_move_position.j].energy = entity_grid[i][j].energy - 5;
            entity_grid[selected_move_position.i][selected_move_position.j].age = entity_grid[i][j].age;
            entity_grid[selected_move_position.i][selected_move_position.j].already_iterated = true;

            entity_grid[i][j].type = empty;
            entity_grid[i][j].energy = 0;
            entity_grid[i][j].age = 0;

            entity_grid[selected_move_position.i][selected_move_position.j].age += 1;
        }
        else entity_grid[i][j].age += 1;
    }

    

    if(entity_grid[i][j].energy == 0 ||          //death
        entity_grid[i][j].age == CARNIVORE_MAXIMUM_AGE) {

            entity_grid[i][j].type = empty;
            entity_grid[i][j].energy = 0;
            entity_grid[i][j].age = 0;
    }
                        
}
int main()
{
    crow::SimpleApp app;

    // Endpoint to serve the HTML page
    CROW_ROUTE(app, "/")
    ([](crow::request &, crow::response &res)
     {
        // Return the HTML content here
        res.set_static_file_info_unsafe("../public/index.html");
        res.end(); });

    CROW_ROUTE(app, "/start-simulation")
        .methods("POST"_method)([](crow::request &req, crow::response &res)
                                { 
        // Parse the JSON request body
        nlohmann::json request_body = nlohmann::json::parse(req.body);

       // Validate the request body 
        uint32_t total_entinties = (uint32_t)request_body["plants"] + (uint32_t)request_body["herbivores"] + (uint32_t)request_body["carnivores"];
        if (total_entinties > NUM_ROWS * NUM_ROWS) {
        res.code = 400;
        res.body = "Too many entities";
        res.end();
        return;
        }

        // Clear the entity grid
        entity_grid.clear();
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0, false, new std::mutex()}));
        for(int i=0; i<15; i++) {
            for(int j=0; j<15; j++) {
                entity_grid[i][j].m = new std::mutex();
            }
        }
        
        // Create the entities
        // <YOUR CODE HERE>
        for (int i=0; i<(uint32_t)request_body["plants"]; i++) {

            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 14);
            int random_i = dis(gen);
            int random_j = dis(gen);

            while (entity_grid[random_i][random_j].type != empty) {
                random_i = dis(gen);
                random_j = dis(gen);
            }

            entity_grid[random_i][random_j].type = plant;
            entity_grid[random_i][random_j].age = 0;
            entity_grid[random_i][random_j].energy = 0;
        }


        for (int i=0; i<(uint32_t)request_body["carnivores"]; i++) {

            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 14);
            int random_i = dis(gen);
            int random_j = dis(gen);

            while (entity_grid[random_i][random_j].type != empty) {
                random_i = dis(gen);
                random_j = dis(gen);
            }
           
            entity_grid[random_i][random_j].type = carnivore;
            entity_grid[random_i][random_j].age = 0;
            entity_grid[random_i][random_j].energy = 100;
        }

        for (int i=0; i<(uint32_t)request_body["herbivores"]; i++) {

            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 14);
            int random_i = dis(gen);
            int random_j = dis(gen);

            while (entity_grid[random_i][random_j].type != empty) {
                random_i = dis(gen);
                random_j = dis(gen);
            }

            entity_grid[random_i][random_j].type = herbivore;
            entity_grid[random_i][random_j].age = 0;
            entity_grid[random_i][random_j].energy = 100;
        }



        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        res.body = json_grid.dump();
        res.end(); });

    // Endpoint to process HTTP GET requests for the next simulation iteration
    CROW_ROUTE(app, "/next-iteration")
        .methods("GET"_method)([]()
                               {
        // Simulate the next iteration
        // Iterate over the entity grid and simulate the behaviour of each entity
        
        // <YOUR CODE HERE>
        for (int i=0; i<15; i++) {
            for (int j=0; j<15; j++) {
                entity_grid[i][j].already_iterated = false;
            }
        }

        for (int i=0; i<15; i++) {
            for (int j=0; j<15; j++) {

                if (!entity_grid[i][j].already_iterated) {
                    if (entity_grid[i][j].type == plant) {
                        std::thread t_plant(simulate_plant, i, j);
                        t_plant.detach();
                    }
                    else if (entity_grid[i][j].type == herbivore) {
                        std::thread t_herbivore(simulate_herbivore, i, j);
                        t_herbivore.detach();
                    }
                    else if (entity_grid[i][j].type == carnivore) {
                        std::thread t_carnivore(simulate_carnivore, i, j);
                        t_carnivore.detach();
                    }
                    
                }
            }
        }
               
                
        
        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}