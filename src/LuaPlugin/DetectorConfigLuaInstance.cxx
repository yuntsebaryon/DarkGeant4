/*
 * DetectorConfigLuaInstance.cxx
 * 
 * Copyright 2016 Emma Davenport <Davenport.physics@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include "DetectorConfigLuaInstance.hh"


/*
 * DetectorConfigLuaInstane::DetectorConfigLuaInstance(string ModulePath)
 * 
 * 
 * */

DetectorConfigLuaInstance::DetectorConfigLuaInstance(string ModulePath)
: LuaInstance(ModulePath, "DetectorConfig.lua")
{
	LoadTable("World");
	Initialize_world();
	
	LoadTable("DetectorConfig");
	Initialize_number_of_detector_components();
	Initialize_detector_components();
	
}

/*
 * DetectorConfigLuaInstance::~DetectorConfigLuaInstance()
 * 
 * 
 * */

DetectorConfigLuaInstance::~DetectorConfigLuaInstance() 
{
		
	
}

void DetectorConfigLuaInstance::Initialize_world() {
	
	G4String Volume_Type = GetStringFromTable_WithHalt("Volume_Type",
                                  "No Volume_Type specified for world");
                              
    G4String Name = GetStringFromTable_NoHalt("World_Name",
                                              "Default World Name",
                                              "World");
    this->World = WithVolumeGetDetectorComponent(Volume_Type, Name);    
}

/*
 * DetectorConfigLuaInstance::Initialize_number_of_detector_components()
 * 
 * 
 * 
 * */

void DetectorConfigLuaInstance::Initialize_number_of_detector_components() {
	
	this->Number_of_Dectector_Components = GetNumberFromTable_WithHalt(
                                "Number_of_Detector_Components",
                                "Missing Number_of_Detector_Components"
                                + string(" in DetectorConfig table."));
	                            
	if (this->Number_of_Dectector_Components <= 0) {
	
		cout << "You did not define the variable ";
		cout << "Number_of_Detector_Components sufficiently.\n";
		cout << "Please be sure you set it to an integer greater than 0";
		cout << ".\n";
		cout << "Halting execution\n";
		exit(0);
		
	}
	
}



/*
 * DetectorConfigLuaInstance::Initialize_detector_components()
 * 
 * 
 * 
 * */

void DetectorConfigLuaInstance::Initialize_detector_components() {
	
	for (int i = 1; i <= this->Number_of_Dectector_Components;i++) {
	
		//Pop entire stack
		lua_pop(this->L, -1);
		
		string tempstring = ConvertIntToString(i);
		
		cout << "\nDetectorComponent_" + tempstring << ":\n";
		LoadTable("DetectorComponent_" + tempstring);
		
		G4String Volume_Type = GetStringFromTable_WithHalt("Volume_Type",
		                     "You didn't define an appropriate volume "
		                     + string("for DetectorComponent_"
		                     + tempstring));
        G4String Name = GetStringFromTable_NoHalt("Component_Name",
                                          "Default Component_Name Used",
                             "DetectorComponent_" + tempstring);            
       /*
        * * Comment
        * 
        * 		Since I've passed true for Halt Execution, I'm going to
        * 		assume that the value that Volume_Type is valid.
        * 
        * */
		this->Components.push_back(WithVolumeGetDetectorComponent(Volume_Type, Name));
		
	}
	cout << "\n";
	
}


DetectorComponent *DetectorConfigLuaInstance::WithVolumeGetDetectorComponent(G4String Volume_Type, G4String Name) {
	
	DetectorComponent *Component;
	if (Volume_Type == "Cylinder")
		Component = MakeDetectorComponent_Cylinder(Name);
    else if (Volume_Type == "Box")
		Component = MakeDetectorComponent_Box(Name);
	
	return Component;
	
}


/*
 * DetectorConfigLuaInstance::MakeDetectorComponent_Cylinder()
 * 
 * 
 * 
 * 
 * */
 
DetectorComponent_Cylinder *DetectorConfigLuaInstance::MakeDetectorComponent_Cylinder(G4String Name) {
	
	
	G4String MaterialString = GetStringFromTable_WithHalt("Material",
                                        "No Material found."
                                        + string(" Halting Execution"));
	     
	                                      
	G4double Inner_Radius = GetNumberFromTable_NoHalt("Inner_Radius",
                                             "No Inner_Radius found."
                                             + string(" Set to 0.0"),
                                             0.0);
                                                                               
	G4double Outer_Radius = GetNumberFromTable_WithHalt("Outer_Radius",
                                             "No Outer_Radiys found."
                                        + string(" Halting Execution"));
                                      
	G4double Start_Angle = GetNumberFromTable_NoHalt("Start_Angle",
                                             "No Start_Angle found."
                                             + string(" Set to 0.0"),
                                             0.0);
                                               
	G4double End_Angle = GetNumberFromTable_NoHalt("End_Angle",
                                             "No End_Angle found."
                                             + string(" Set to 360."),
                                             360.);
                                            
	G4double Half_Length = GetNumberFromTable_WithHalt("Half_Length",
                                             "No Half_Length found."
                                        + string(" Halting Execution"));
                                        
	G4String Inside = GetStringFromTable_WithHalt("Inside",
                                                 "Please define Inside."
                                        + string(" Halting Execution"));
                                         
                                         
	G4ThreeVector Position = GetG4ThreeVector("Position");
   
   
	return new DetectorComponent_Cylinder(Name,
                                      Inner_Radius,
                                      Outer_Radius,
                                      Start_Angle,
                                      End_Angle,
                                      Half_Length,
                                      Position,
                                      MaterialString,
                                      Inside);
   
}




/*
 * DetectorConfigLuaInstance::MakeDectorComponent_Box()
 * 
 * 
 * 
 * */

DetectorComponent_Box *DetectorConfigLuaInstance::MakeDetectorComponent_Box(G4String Name) {
	
	G4String MaterialString = GetStringFromTable_WithHalt("Material",
                               "Did not provide a valid material");
    
	G4double X = GetNumberFromTable_WithHalt("X", "Did not provide X "+
                                    string("value. Halting Execution"));
	
	G4double Y = GetNumberFromTable_WithHalt("Y", "Did not provide Y "+
                                    string("value. Halting Execution"));
                                     
	G4double Z = GetNumberFromTable_WithHalt("Z", "Did not provide Z "+
                                    string("value. Halting Execution"));
                                    
	G4String Inside = GetStringFromTable_WithHalt("Inside",
                                        "Please define Inside."
                                        + string(" Halting Execution"));
                                     
	G4ThreeVector Position = GetG4ThreeVector("Position");
	
	return new DetectorComponent_Box(Name, X, Y, Z, 
                                     Position, MaterialString, Inside);
                                     
}
