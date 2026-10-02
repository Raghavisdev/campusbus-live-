#include <string>
using namespace std;

class Bus{

    private:

    string busID;
    string routeID;
    string dispatchTime;
    double latitude;
    double longitude;
    int capacity;
    int occupancy;

    public :

    Bus(string busID, string routeID, int capacity);

    void updateLocation(double latitude, double longitude);
    void updateOccupancy(int occupancy);
    void setDispatchTime(string dispatchTime);
    
    string getDispatchTime() const; 

    string getBusID() const;
    string getRouteID() const;

    double getLatitude() const;
    double getLongitude() const;

    int getCapacity() const;
    int getOccupancy() const;

}; 
