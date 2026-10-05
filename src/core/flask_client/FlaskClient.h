#ifndef FLASK_CLIENT_H
#define FLASK_CLIENT_H

#include <string>

using namespace std;

class FlaskClient
{
public:

    static int getOccupancy(string busID);
};

#endif