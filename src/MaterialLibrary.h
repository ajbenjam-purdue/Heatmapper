#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

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
    // The library's tracked materials
    std::vector<Material> materials;

    void load_json(std::string path)
    {
        if (std::filesystem::exists(std::filesystem::path(path))) // json exists, load
        {
            std::fstream File;
            File.open(path);

            json Doc{json::parse(File)};

            if (Doc.contains("materials"))
            {
                for (const auto& material : Doc["materials"])
                {
                    materials.push_back(
                        Material(
                            material["name"], material["density"], material["specific_heat"], material["thermal_conductivity"]
                        )
                    );
                }
            }
        }
        else // json doesn't exist, load defaults + create file
        {
            // I'm picking 6061 and stainless to start with. TODO: BUILD OUT REAL LIB
            // These come from asm.matweb.com
            materials = {
                Material("Aluminum 6061", 2700.0, 896.0, 167.0),
                Material("304 Stainless Steel", 8000.0, 500.0, 16.2)
            };

            json Doc;
            
            Doc["materials"] = json::array();
            for (Material& material : materials) {
                Doc["materials"].push_back({
                    {"name", material.name},
                    {"density", material.density},
                    {"specific_heat", material.specific_heat},
                    {"thermal_conductivity", material.thermal_conductivity}
                });
            }

            // Write it to the path
            std::ofstream file(path);
            if (file.is_open()) {
                file << Doc.dump(4); // 4 space indentation
                file.close();
            }
        }
    }
};