#include "FlaskClient.h"
#include <iostream>
#include <cstdio>

int FlaskClient::getOccupancy(string busID)
{
    string command = "curl.exe -s http://127.0.0.1:5000/get-occupancy/" + busID;

    FILE* pipe = _popen(command.c_str(), "r");

    if (pipe == NULL)
    {
        return -1;
    }

    char buffer[128];
    string response = "";

    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        response += buffer;
    }

    _pclose(pipe);

    size_t position = response.find("\"occupancy\"");

    if (position == string::npos)
    {
        return -1;
    }

    position = response.find(":", position);

    if (position == string::npos)
    {
        return -1;
    }

    return stoi(response.substr(position + 1));
}