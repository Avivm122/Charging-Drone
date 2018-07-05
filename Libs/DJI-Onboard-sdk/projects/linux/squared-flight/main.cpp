/*! @file flight-control/main.cpp
 *  @version 3.3
 *  @date Jun 05 2017
 *
 *  @brief
 *  main for Flight Control API usage in a Linux environment.
 *  Provides a number of helpful additions to core API calls,
 *  especially for position control, attitude control, takeoff,
 *  landing.
 *
 *  @Copyright (c) 2016-2017 DJI
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "flight_control_sample.hpp"

using namespace std;
using namespace DJI::OSDK;
using namespace DJI::OSDK::Telemetry;

void SquareFlight(Vehicle* vehicle); 
/*! main
 *
 */
int main(int argc, char** argv){
  // Initialize variables
  int functionTimeout = 1;

  // Setup OSDK.
  LinuxSetup linuxEnvironment(argc, argv);
  Vehicle*   vehicle = linuxEnvironment.getVehicle();
  if (vehicle == NULL)
  {
    cout << "Vehicle not initialized, exiting.\n";
    return -1;
  }

  // Obtain Control Authority
  vehicle->obtainCtrlAuthority(functionTimeout);

  // Display interactive prompt
  cout	<< "| Prepare for squared flight.                                            |"	<< endl;
  cout	<< "| Taking off                                                             |" << endl; 
  monitoredTakeoff(vehicle);
  SquareFlight(vehicle);
  monitoredLanding(vehicle);
  return 0;
}

void SquareFlight(Vehicle* vehicle){
	cout << "A" << endl;
	moveByPositionOffset(vehicle, 5, 0, 0, 0);
	cout << "B" << endl;
	moveByPositionOffset(vehicle, 5, 5, 5, 90);
	cout << "C" << endl;
	moveByPositionOffset(vehicle, 0, 5, -5, 90);
	cout << "D" << endl;
 	moveByPositionOffset(vehicle, 0, 0, 0, 90);
	cout << "E" << endl;
	moveByPositionOffset(vehicle, 5, 0, 0, 90);
}
