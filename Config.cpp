#include "Config.h"

/********** CONSTRUCTOR **********/

Config::Config(const std::string& file_name) :
    config_file(file_name)
{

}

/********** DESTRUCTOR **********/

Config::~Config()
{
}


/********** PUBLIC FUNCTIONS **********/

bool Config::checkConfigFile()
{
    // Open the file in read mode
    std::ifstream ifs(this->config_file);

    // Create a json object instance
    json j;

    // Load the file content into the json object
    ifs >> j;

    // Close the file
    ifs.close();

    /*
        To access the JSON object's elements we use here .at("key") because it raises an exception
        which can be handled if the key does not exist, while the operator [] has an unpredictable behaviour.
    */

    try
    {
        std::string fit = j.at("fit");

        // "fit" can only have values equal to 1 or 0
        if (fit != "0" && fit != "1")
        {
            return false;
        }
    }
    catch (const std::exception& e)
    {
        // "fit" key is not present
        return false;
    }

    try
    {
        std::string autoresize = j.at("autoresize");

        // "autoresize" can only have values equal to 1 or 0
        if (autoresize != "0" && autoresize != "1")
        {
            return false;
        }
    }
    catch (const std::exception& e)
    {
        // "autoresize" key is not present
        return false;
    }

    try
    {
        std::string color = j.at("color");

        // "color" can only have values equal to 1 or 0
        if (color != "0" && color != "1")
        {
            return false;
        }
    }
    catch (const std::exception& e)
    {
        // "color" key is not present
        return false;
    }

    try
    {
        std::string gradient = j.at("gradient");

        // "gradient" can only have values equal to 1 or 0
        if (gradient != "0" && gradient != "1")
        {
            return false;
        }
    }
    catch (const std::exception& e)
    {
        // "gradient" key is not present
        return false;
    }

    try
    {
        std::string zmax = j.at("zmax");

        // "zmax" can only have values equal to 1 or 0
        if (zmax != "0" && zmax != "1")
        {
            return false;
        }
    }
    catch (const std::exception& e)
    {
        // "zmax" key is not present
        return false;
    }

    try
    {
        std::string folder = j.at("folder");

        // "folder" must contain a path and it must be valid
        if (folder == "" || !Helpers::pathFileExists(folder))
        {
            return false;
        }
    }
    catch (const std::exception& e)
    {
        // "folder" key is not present or it is not valid
        return false;
    }

    try
    {
        std::string lang = j.at("lang");

        // "lang" must contain the code of one of the existing language translations
        if (lang != "it" && lang != "en")
        {
            return false;
        }
    }
    catch (const std::exception& e)
    {
        // "lang" key is not present or it is not valid
        return false;
    }

    // Once here, all the checks did pass, the file is valid
    return true;
}

void Config::initConfigFile(const std::string& lang)
{
    // Open the file in write mode, trunc flag to overwrite the eventual existing file
    std::ofstream ofs(this->config_file, std::ofstream::trunc);

    // Create a json object with the default configuration values
    json j;

    j["fit"] = "0";
    j["autoresize"] = "0";
    j["color"] = "0";
    j["gradient"] = "0";
    j["zmax"] = "0";
    j["folder"] = "C:/helios1/archivio";
    j["lang"] = lang;

    // Write the JSON converted to string into the file
    ofs << j.dump();

    // Close the file
    ofs.close();
}

void Config::updateConfigFile(const std::string& key, const std::string& value)
{
    /*
        The file is not opened directly in read/write mode, because it is required to open it with the "trunc" flag
        to be able to overwrite the previous content, with that flag set, the file results to be empty and
        the attempt to load the json string would fail.
    */

    // Open the file in read mode
    std::ifstream ifs(this->config_file);

    // Create a json object instance
    json j;

    // Load the file content into the json object
    ifs >> j;

    // Update the value at the key position
    j[key] = value;

    // Close the file
    ifs.close();

    // Open the file in write mode, trunc flag to overwrite the existing file
    std::ofstream ofs(this->config_file, std::ofstream::trunc);

    // Write the JSON converted to string into the file
    ofs << j.dump();

    // Close the file
    ofs.close();
}
