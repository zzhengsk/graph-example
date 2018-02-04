// Author: Sean Davis
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include "CPUTimer.h"
#include "EvacRunner.h"
#include "evac.h"

using namespace std;

bool EvacRoute::operator< (const EvacRoute &rhs) const
{
  return time < rhs.time;
} // operator<

void readCities(ifstream &inf, City cities[], int evacIDs[], int numCities,
  int numEvacs)
{
  int ID, roadCount;

  for(int i = 0; i < numEvacs; i++)
    inf >> evacIDs[i];

  for(int i = 0; i < numCities; i++)
  {
    inf >> ID;
    cities[ID].ID = ID;
    inf >> cities[ID].x >> cities[ID].y >> cities[ID].population >> roadCount;
    cities[ID].roadCount = roadCount;
    cities[ID].roads = new Road[roadCount];
    for(int j = 0; j < roadCount; j++)
      inf >> cities[ID].roads[j].destinationCityID
        >> cities[ID].roads[j].peoplePerHour >> cities[ID].roads[j].ID;
  } // for each city
  inf.close();
} // readCities()

void readCities2(ifstream &inf, City cities[], char evacIDs[], int numCities,
  int numEvacs, Road2 *roads)
{
  int ID, roadCount, roadID, destinationCityID, peoplePerHour;

  for(int i = 0; i < numCities; i++)
    evacIDs[i] = 0;

  for(int i = 0; i < numEvacs; i++)
  {
    inf >> ID;
    evacIDs[ID] = 1;
  }

  for(int i = 0; i < numCities; i++)
  {
    inf >> ID;
    inf >> cities[ID].x >> cities[ID].y >> cities[ID].population >> roadCount;
    for(int j = 0; j < roadCount; j++)
    {
      inf >> destinationCityID >> peoplePerHour >> roadID;
      roads[roadID].destinationCityID = destinationCityID;
      roads[roadID].sourceCityID = ID;
      roads[roadID].peoplePerHour = peoplePerHour;
    }
  } // for each city
  inf.close();
} // readCities()


void checker(City *cities, char *evacIDs, int numCities, int numEvacs, EvacRoute *evacRoutes,
  int routeCount, Road2 *roads)
{
  int time = 0, total = 0;
  multiset <EvacRoute> routes;
  vector <int> roadIDs;
  set <int> cityIDs;
  for(int i = 0; i < routeCount; i++)
    routes.insert(evacRoutes[i]);

  for(multiset<EvacRoute>::iterator itr = routes.begin(); itr != routes.end(); itr++)
  {
    if(itr->time < 1)
    {
      cout << "Initial time is " << itr->time << " which is less than 1\n";
      return;
    }
    if(itr->time != time)
    {
      for(set<int>::iterator citr = cityIDs.begin(); citr != cityIDs.end(); citr++)
      {
        if(cities[*citr].evacuees > cities[*citr].population)
        {
          cout << "At time " << time << " city ID " << *citr << " has "
            << cities[*citr].evacuees << " but only pop "
            << cities[*citr].population << endl;
          return;
        }

        if(cities[*citr].evacuees < 0 && evacIDs[*citr] == 0)
        {
          cout << "At time " << time << " city ID " << *citr << " has "
            << cities[*citr].evacuees << endl;
          return;
        }
      } // for each city used at this time

      for(vector<int>::iterator vitr = roadIDs.begin(); vitr != roadIDs.end(); vitr++)
        roads[*vitr].peopleThisHour = 0; // reset for next hour

      roadIDs.erase(roadIDs.begin(), roadIDs.end());
      time++;
    } // if new time

    if(itr->numPeople > 0)
    {
      roads[itr->roadID].peopleThisHour += itr->numPeople;
      if(roads[itr->roadID].peopleThisHour > roads[itr->roadID].peoplePerHour
        || roads[itr->roadID].peopleThisHour < 0)
      {
        cout << "At time " << time << " road " << itr->roadID << " from city "
          << roads[itr->roadID].sourceCityID << " to city "
          << roads[itr->roadID].destinationCityID << " has "
          << roads[itr->roadID].peopleThisHour << " using it\n";
        return;
      }
      roadIDs.push_back(itr->roadID);
      if(roads[itr->roadID].sourceCityID == 2)
        total += itr->numPeople;
      if(roads[itr->roadID].destinationCityID == 2)
        total -= itr->numPeople;

      cities[roads[itr->roadID].sourceCityID].evacuees -= itr->numPeople;
      cities[roads[itr->roadID].destinationCityID].evacuees += itr->numPeople;
      cityIDs.insert(roads[itr->roadID].sourceCityID);
      cityIDs.insert(roads[itr->roadID].destinationCityID);
    }
  }  // for all EvacRoutes

  for(int i = 0; i < numCities; i++)
    if(evacIDs[i] && -cities[i].evacuees != cities[i].population)
    {
      cout << "Evacuated city ID " << i << " has " << -cities[i].evacuees << " evacuees but a pop "
        << cities[i].population << endl;
      return;
    }

  cout << "Evacuation hours: " << time << endl; 
} // checker()

int main(int argc, char* argv[])
{
  int numCities, numRoads, numEvacs, routeCount;
  ifstream inf(argv[1]);

  inf >> numCities >> numRoads >> numEvacs;
  City *cities = new City[numCities];
  int *evacIDs = new int[numEvacs];
  readCities(inf, cities, evacIDs, numCities, numEvacs);
  EvacRoute *evacRoutes = new EvacRoute[numCities * 1000];
  CPUTimer ct;
  ct.reset();
  Evac *evac = new Evac(cities, numCities, numRoads);
  delete [] cities;
  evac->evacuate(evacIDs, numEvacs, evacRoutes, routeCount);
  cout << "CPU Time: " << ct.cur_CPUTime() << endl;
  ifstream inf2(argv[1]);
  inf2 >> numCities >> numRoads >> numEvacs;
  cities = new City[numCities];
  Road2 *roads = new Road2[2 * numRoads];
  char *evacIDs2 = new char[numCities];
  readCities2(inf2, cities, evacIDs2, numCities, numEvacs, roads);
  checker(cities, evacIDs2, numCities, numEvacs, evacRoutes, routeCount, roads);
  return 0;

}

