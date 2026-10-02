#include <string>
using namespace std;

class Bus{

    private:

    string busID;
    string routeID;
    double latitude;
    double longitude;
    int capacity;
    int occupancy;

    public :

    void updateLocation(double latitude, double longitude);
    void updateOccupancy(int occupancy);

    string getBusID() const;
    string getRouteID() const;

    double getLatitude() const;
    double getLongitude() const;

    int getCapacity() const;
    int getOccupancy() const;
    
}; 
