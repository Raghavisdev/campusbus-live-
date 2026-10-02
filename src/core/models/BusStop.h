#include <string>

using namespace std;

class BusStop{
    private:
        double latitude;
        double longitude;
        string stopID;
        string stopName;
    
    public:
        BusStop(string stopID, string stopName, double latitude, double longitude);

        string getStopID() const;
        string getStopName() const;
        double getLatitude() const;
        double getLongitude() const;    
};