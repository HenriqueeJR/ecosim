#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>

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
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0}));
        
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

                if (entity_grid[i][j].type == plant) {
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
                        if(random_action(0.2)) {
                            std::random_device rd;  
                            std::mt19937 generator(rd());  
                            std::uniform_int_distribution<int> distribution(0, possible_growth_positions.size() - 1);
                            
                            pos_t selected_growth_position = possible_growth_positions[distribution(generator)];
                            entity_grid[selected_growth_position.i][selected_growth_position.j].type = plant;
                            entity_grid[selected_growth_position.i][selected_growth_position.j].energy = 0;
                            entity_grid[selected_growth_position.i][selected_growth_position.j].age = 0;
                        }

                    }

                    entity_grid[i][j].age += 1;  //increase age

                    if (entity_grid[i][j].age == 10) {  //decompose
                        entity_grid[i][j].type = empty;
                        entity_grid[i][j].energy = 0;
                        entity_grid[i][j].age = 0;
                    }


                }

                //else if (entity_grid[i][j].type == carnivore)

                //else if (entity_grid[i][j].type == herbivore)
            }
        }
        
        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}