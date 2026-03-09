#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "json.hpp"
using json = nlohmann::json;

struct Material
{
    // The material's display name
    std::string name;

    // The material's density [kg/m3]
    double density;

    // The material's specific heat [J/kg-K]
    double specific_heat;

    // The material's thermal conductivity [W/m-K]
    double thermal_conductivity;

    // Default constructor
    Material() : name("Unknown"), density(1.0), specific_heat(1.0), thermal_conductivity(1.0) {}

    // Create a material with the specified properties
    Material(std::string mat_name, double mat_dens, double mat_spec_heat, double mat_therm_cond) :
        name(mat_name), 
        density(mat_dens), 
        specific_heat(mat_spec_heat), 
        thermal_conductivity(mat_therm_cond)
    {}
};

class MaterialLibrary
{
public:
    std::vector<Material> materials;

    void load_json(const std::string& path)
    {
        materials.clear(); // Wipe existing materials to prevent duplicates on reload

        if (std::filesystem::exists(path))
        {
            std::ifstream file(path);
            if (file.is_open())
            {
                try {
                    json Doc = json::parse(file);
                    if (Doc.contains("materials"))
                    {
                        for (const auto& material : Doc["materials"])
                        {
                            materials.push_back(
                                Material(
                                    material["name"], 
                                    material["density"], 
                                    material["specific_heat"], 
                                    material["thermal_conductivity"]
                                )
                            );
                        }
                    }
                } 
                catch (const json::parse_error& e) {
                    // If the user corrupted the JSON file, catch the error so the app doesn't crash
                    std::cerr << "Material Library Parse Error: " << e.what() << "\n";
                    load_defaults();
                    save_json(path); // Overwrite the corrupted file with defaults
                }
            }
        }
        else 
        {
            // json doesn't exist, load defaults + create file
            load_defaults();
            save_json(path);
        }
    }

    // Extracted into its own public method so the UI can trigger a save!
    void save_json(const std::string& path)
    {
        json Doc;
        Doc["materials"] = json::array();
        for (const Material& material : materials) {
            Doc["materials"].push_back({
                {"name", material.name},
                {"density", material.density},
                {"specific_heat", material.specific_heat},
                {"thermal_conductivity", material.thermal_conductivity}
            });
        }

        std::ofstream file(path);
        if (file.is_open()) {
            file << Doc.dump(4);
            file.close();
        }
    }

private:
    void load_defaults()
    {
        materials = {
            Material("Aluminum 6061", 2700.0, 896.0, 167.0),
            Material("304 Stainless Steel", 8000.0, 500.0, 16.2),
            Material("Water", 1000.0, 4187.0, 600.0),
            Material("Air", 1.276, 1006.0, 0.02435)
        };
    }
};