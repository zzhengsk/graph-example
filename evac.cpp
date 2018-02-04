#include <cstdlib>
#include "evac.h"
#include "EvacRunner.h"

using namespace std;

int City3Cmp(const void *ptr, const void *ptr2)
{
  if( ((City3*) ptr)->ratio > ((City3*) ptr2)->ratio)
    return -1;
  else
    if( ((City3*) ptr)->ratio < ((City3*) ptr2)-> ratio)
      return 1;
    else
      return 0;
} // City3Cmp()

Evac::Evac(City *citie, int numCitie, int numRoads) : numCities(numCitie)
{
  int ID, roadID, roadCount;
  cities = new City2[numCities];
  roads = new Road3[numRoads * 2];
  usedIDs = new int[numRoads * 5];
  visitedIDs = new int[numCities];

  for(int i = 0; i < numCitie; i++)
  {
    ID = citie[i].ID;
    cities[ID].evacuees = 0;
    cities[ID].population = citie[i].population;
    cities[ID].roadCount = roadCount = citie[i].roadCount;
    cities[ID].roadIDs = new int[roadCount];
    for(int j = 0; j < roadCount; j++)
    {
      cities[ID].roadIDs[j] = roadID = citie[i].roads[j].ID;
      roads[roadID].destinationCityID = (unsigned short) citie[i].roads[j].destinationCityID;
      roads[roadID].peoplePerHour =  citie[i].roads[j].peoplePerHour;
    } // for each road
  }
} // Evac()


void Evac::evacuate(int *evacIDs, int numEvacs, EvacRoute *evacRoutes,
    int &routeCount)
{
  int ID;
  time = 1;
  routeCount = 0;
  evacArray = new City3[numEvacs];
  queue = new unsigned short[numCities];
  visitedCount = front = back = 0;
  bool left = true;
  evacRoutes2 = evacRoutes;

  for(int i = 0; i < numEvacs; i++)
  {
    ID = evacIDs[i];
    cities[ID].known = true;
    cities[ID].end = false;
    cities[ID].evacuatedCity = true;
    cities[ID].depth = 1;
    queue[back++] = ID;
    evacArray[i].ID = ID;
  }

  while(left)
  {
    processQ();
    left = processEvacsArray(evacArray, numEvacs);
    usedCount = 0;
    for(int i = 0; i < numEvacs; i++)
    {

      int cityID = evacArray[i].ID;
      int total = 0;
      int needed = cities[cityID].population - cities[cityID].evacuees;
      while(visitedCount > 0)
      {
        cities[visitedIDs[--visitedCount]].visited = false;
     //   cities[visitedIDs[visitedCount]].visitedCount = 0;
      }
      visitedIDs[visitedCount++] = cityID;
      cities[cityID].visited = true;

      for(int i = 0; i < cities[cityID].roadCount && total < needed; i++)
      {
        int flow = roads[cities[cityID].roadIDs[i]].peoplePerHour
          - roads[cities[cityID].roadIDs[i]].used;
        if(flow > needed - total)
          flow = needed - total;
        flow = dfs(roads[cities[cityID].roadIDs[i]].destinationCityID, flow, cityID);
        roads[cities[cityID].roadIDs[i]].used += flow;
        total += flow;
        if(flow > 0)
          usedIDs[usedCount++] = cities[cityID].roadIDs[i];
      }  // for each road
      cities[cityID].evacuees += total;
    } // for each evac city
    for(int i = 0; i < numCities; i++)
      cities[i].visited = 0;
    storeRoutes(evacRoutes, routeCount);
    time++;
  }

} // evacuate


void Evac::storeRoutes(EvacRoute *evacRoutes, int &routeCount)
{
  for(int i = 0; i < usedCount; i++)
  {
    evacRoutes[routeCount].roadID = usedIDs[i];
    evacRoutes[routeCount].numPeople = roads[usedIDs[i]].used;
    evacRoutes[routeCount++].time = time;
    roads[usedIDs[i]].used = 0;
    cities[roads[usedIDs[i]].destinationCityID].visited = 0;
  } // for each road used
}

int Evac::dfs(unsigned short cityID, int needed, unsigned short sourceCityID)
{
  int i, total = 0;
  if(cities[cityID].visited)
     return 0;
  else
  {
    cities[cityID].visited = true; // stop loops
    visitedIDs[visitedCount++] = cityID;
  }
  
  if(!cities[cityID].evacuatedCity) // not an evacuated city
  {
    if(needed <= cities[cityID].population - cities[cityID].evacuees)
    {
      cities[cityID].evacuees += needed;
      return needed;
    }
    else // not enough room in city for needed
    {
      total = cities[cityID].population - cities[cityID].evacuees;
      cities[cityID].evacuees = cities[cityID].population;
    }
  } // if not an evacuated city then absorb some of the needed

  if(cities[cityID].end)
    return total;

  for(i = 0; i < cities[cityID].roadCount && total < needed; i++)
  {
    if(roads[cities[cityID].roadIDs[i]].destinationCityID == sourceCityID)
      continue;  // no infinite loops!
    int flow = roads[cities[cityID].roadIDs[i]].peoplePerHour
      - roads[cities[cityID].roadIDs[i]].used;
    if(flow > needed - total)
      flow = needed - total;
    flow = dfs(roads[cities[cityID].roadIDs[i]].destinationCityID, flow, cityID);
    roads[cities[cityID].roadIDs[i]].used += flow;
    total += flow;
    if(flow > 0)
      usedIDs[usedCount++] = cities[cityID].roadIDs[i];
  }
  return total;
} // dfs()

bool Evac::processEvacsArray(City3 *evacArray, int numEvacs)
{
  int capacity;
  bool left = false;

  for(int i = 0; i < numEvacs; i++)
  {
    int ID = evacArray[i].ID;
    if(cities[ID].evacuees < cities[ID].population)
    {
      left = true;
      capacity = 0;
      for(int j = 0; j < cities[ID].roadCount; j++)
        if(roads[cities[ID].roadIDs[j]].peoplePerHour <
          cities[roads[cities[ID].roadIDs[j]].destinationCityID].population)
          capacity += roads[cities[ID].roadIDs[j]].peoplePerHour;
        else
          capacity += cities[roads[cities[ID].roadIDs[j]].destinationCityID].population;
      evacArray[i].ratio = (cities[ID].population - cities[ID].evacuees)
          / (1.0 * capacity);
    } // if some evacuees still left
  } // for each city still in the evacArray

  if(left)
    qsort(evacArray, numEvacs, sizeof(City3), City3Cmp);

  return left;
} // processEvacsArray


void Evac::processQ()
{
  int back2 = back, ID, destCityID;
  for(int i = front; i < back; i++)
    cities[queue[i]].end = false;

  while(front < back2)
  {
    ID = queue[front++];

    for(int j = 0; j < cities[ID].roadCount;)
    {
      destCityID = roads[cities[ID].roadIDs[j]].destinationCityID;
      if(!cities[destCityID].known)
      {
        queue[back++] = destCityID;
        cities[ID].depth = time + 1;
        cities[destCityID].known = true;
      }

      if(cities[destCityID].depth != 0 && cities[destCityID].depth < time - 1)
      // more than one level back in tree so eliminate road
      {
        int temp = cities[ID].roadIDs[j];
        cities[ID].roadIDs[j] = cities[ID].roadIDs[--cities[ID].roadCount];
        cities[ID].roadIDs[cities[ID].roadCount] = temp;
      }

      else // city not in tree
        j++;
    } // for each road
  } // while more in the queue for this round

} // processQ()
