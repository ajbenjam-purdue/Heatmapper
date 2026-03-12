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

    // Pushes a new material into a library and returns its index in the vector
    size_t push_material(Material new_material)
    {
        materials.push_back(new_material);
        return materials.size() - 1;
    }

    // Inserts a material at the specified index
    void insert_material(Material new_material, int index)
    {
        if (index >= 0 && index <= materials.size())
            materials.insert(materials.begin()+index, new_material);
    }

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

    // Extracted into its own public method so the UI may trigger a save
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
        else
        {
            // Low quality pop-up
            wxMessageBox(wxString::Format("Could not save to path '%s'", path));
        }
    }

private:
    void load_defaults()
    {
        materials = {
            // Aluminum
            Material("Aluminum (Pure)", 2702.0, 903.0, 237.0),
            Material("Aluminum 6061", 2700.0, 896.0, 167.0),
            Material("Aluminum 2024-T6", 2770.0, 875.0, 177.0),

            // Copper
            Material("Copper", 8933.0, 385.0, 401.0),
            Material("Commercial Bronze", 8800.0, 420.0, 52.0),
            Material("Phosphor Bronze", 8780.0, 355.0, 54.0),
            Material("Cartridge Brass", 8530.0, 380.0, 110.0),
            Material("Constantan", 8920.0, 384.0, 23.0),
            
            // Iron/Steel
            Material("Armco Pure Iron", 7870.0, 447.0, 72.7),
            Material("Carbon Steel", 7854.0, 434.0, 60.5),
            Material("AISI 1010 Steel", 7832.0, 434.0, 63.9),
            Material("AISI 302 Stainless Steel", 8055.0, 480.0, 15.1),
            Material("AISI 304 Stainless Steel", 8000.0, 500.0, 16.2),
            Material("AISI 316 Stainless Steel", 8238.0, 468.0, 13.4),
            Material("AISI 347 Stainless Steel", 7978.0, 480.0, 14.2),
            Material("Carbon-Silicon Steel", 7817.0, 446.0, 51.9),
            Material("Carbon-Manganese-Silicon Steel", 8131.0, 434.0, 41.0),
            Material("Cr-Mo Steel (1/2Cr-1/4Mo-Si)", 7822.0, 444.0, 37.7),
            Material("Cr-Mo Steel (1Cr-1/2Mo)", 7858.0, 442.0, 42.3),
            Material("Cr-V Steel", 7836.0, 443.0, 48.9),

            // High Temp Alloys
            Material("Nichrome", 8400.0, 420.0, 12.0),
            Material("Inconel X-750", 8510.0, 439.0, 11.7),
            Material("Nickel", 8900.0, 444.0, 90.7),
            Material("Titanium", 4500.0, 522.0, 21.9),
            Material("Tungsten", 19300.0, 132.0, 174.0),
            Material("Molybdenum", 10240.0, 251.0, 138.0),
            Material("Niobium", 8570.0, 265.0, 53.7),
            Material("Vanadium", 6100.0, 489.0, 30.7),

            // Structural/Insulation
            Material("Plywood", 545.0, 1215.0, 0.12),
            Material("Sheathing", 290.0, 1300.0, 0.055),
            Material("Acoustic Tile", 290.0, 1340.0, 0.058),
            Material("Hardboard Siding", 640.0, 1170.0, 0.094),
            Material("Hardboard High Density", 1010.0, 1380.0, 0.15),
            Material("Particle Board Low Density", 590.0, 1300.0, 0.078),
            Material("Particle Board High Density", 1000.0, 1300.0, 0.17),
            Material("Hardwood", 720.0, 1255.0, 0.16),
            Material("Softwood", 510.0, 1380.0, 0.12),
            Material("Cement Mortar", 1860.0, 780.0, 0.72),
            Material("Common Brick", 1920.0, 835.0, 0.72),
            Material("Face Brick", 2083.0, 835.0, 1.3),
            Material("Ice", 920.0, 2040.0, 1.88),
            Material("Paraffin Wax", 900.0, 2890.0, 0.240),

            // Fluids TODO: Look into temperature-evaluated system
            Material("Water", 1000.0, 4187.0, 600.0),
            Material("Air", 1.276, 1006.0, 0.02435)
        };
    }
};