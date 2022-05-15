#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include "Helpers.h"
#include "json.hpp"

using json = nlohmann::json;

class Config
{
public:
    /********** CONSTRUCTOR **********/

    /*
     * Constructs the Config object.
     *
     * @param   const std::string&      file_name     - Config file name
     * 
     */
    Config(const std::string& file_name);

    /********** DESTRUCTOR **********/

    /*
     * Destructs the Config object.
     */
    ~Config();

    /********** PUBLIC FUNCTIONS **********/

    /*
     * Checks if the content of the config file is valid.
     *
     * @return  void
     */
    bool checkConfigFile();

    /*
     * Initializes a new config file.
     * 
     * @param  const std::string&       lang    - Language code (it, en...)
     *
     * @return  void
     */
    void initConfigFile(const std::string& lang);

    /*
     * Updates the config file.
     *
     * @param   const std::string&      key     - Name of the option to change.
     * @param   const std::string&      value   - Value to set.
     * 
     * @return  void
     */
    void updateConfigFile(const std::string& key, const std::string& value);

private:
    // Name of the configuration file
    std::string config_file;
};

