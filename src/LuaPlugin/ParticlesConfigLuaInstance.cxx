/*
 * ParticlesConfigLuaInstance
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

#include "ParticlesConfigLuaInstance.hh"

ParticlesConfigLua::ParticlesConfigLua(string ModulePath) 
 : LuaInstance(ModulePath, "Particles.lua")
{
	
	this->FourVectorFile = false;
	this->FileHasPosition = false;
	this->FileHasParticleNames = false;
	this->PositionDefinedByFunction = false;
	
	Initialize_ParticleFile();
	
	if (this->ParticleFile.length() == 0) {
		
		cout << "No Particle File specified\n";
		Initialize_GenericFourVector();
		return;
	}
		
	Initialize_ParticleFileType();
	if (this->FourVectorFile) {
		
		cout << "Reading Four Vector File " << this->ParticleFile << "\n";
		ReadFile_FourVector();
		
	}
	if (this->PositionDefinedByFunction) {
		
		cout << "Position is defined by function\n";
		Initialize_ParticlePositions_byFunction();
		
	}
	
}

ParticlesConfigLua::~ParticlesConfigLua() {
	
	
	
}

/*
 * Initialize_ParticleFile()
 * 
 * * Description
 * 
 *		...
 * 
 * */
 
void ParticlesConfigLua::Initialize_ParticleFile() {
	
	this->ParticleFile = GetStringFromGlobal_NoHalt("Particle_File","");
	
}

/*
 * Initialize_ParticleFileType()
 * 
 * * Description
 * 
 * 		...
 * 
 * */

void ParticlesConfigLua::Initialize_ParticleFileType() {
	
	this->ParticleFileType = GetStringFromGlobal_WithHalt(
                                                  "Particle_File_Type");
	Parse_ParticleFileType();
	
}

/*
 * Initialize_ParticlePositions_ByFunction()
 * 
 * * Description
 * 
 * 		...
 * 
 * */

void ParticlesConfigLua::Initialize_ParticlePositions_byFunction() {
	
	Load_PositionFunction();
	
	for (int i = 1; i <= (int)this->FourVectors.size(); i++) {
		
		lua_pushinteger(this->L, i);
		
		G4double *PositionPointers[3] = {&this->FourVectors[i].X,
                                         &this->FourVectors[i].Y,
                                         &this->FourVectors[i].Z};
                                         
		for (int j = 1; j <= 3; j++) {
		
			lua_pushinteger(this->L, j);
			lua_gettable(this->L, -3);
			
			if (lua_type(this->L, -1) != LUA_TNUMBER) {
			
				// TODO do something
				
			}
			*(PositionPointers[j-1]) = lua_tonumber(this->L, -1);
			//Pops value
			lua_pop(this->L, 1);
			
		}
		
	}
	
	// Pops Particle_Table
	lua_pop(this->L, 1);
}

void ParticlesConfigLua::Initialize_GenericFourVector() {

	cout << "Particle_Table\n";

	Parse_ParticlePosition();
	LoadTable("Particle_Table");
	G4String ParticleName = GetStringFromTable_WithHalt("Particle_Name",
                                            "Particle_Name not found.");
                                             
	int NumberOfEvents = GetIntegerFromTable_WithHalt("Number_Of_Events",
                                         "Number_Of_Events not found.");
                                         
	G4double Energy = GetNumberFromTable_WithHalt("Energy",
                                                  "Energy not found.");
                                                  
	G4ThreeVector Momentum  = GetG4ThreeVector("Momentum_Direction");
	
	//Pops Particle_Table
	lua_pop(this->L, 1);
	
	FourVector Vector;
	Vector.ParticleName = ParticleName;
	Vector.X = Positions.x();
	Vector.Y = Positions.y();
	Vector.Z = Positions.z();
	Vector.P_x = Momentum.x();
	Vector.P_y = Momentum.y();
	Vector.P_z = Momentum.z();
	Vector.E = Energy;
	Vector.T = GetParticleKineticEnergy(ParticleName, Energy); 
	for (int i = 0;i < NumberOfEvents;i++) {
	
		this->FourVectors.push_back(Vector);
		
	}
	
}

void ParticlesConfigLua::Load_PositionFunction() {
	
	/*
	 * Loads Particle_Table and pushes the position function
	 * to the top of the stack.
	 * 
	 * */
	
	LoadTable("Particle_Table");
	lua_pushstring(this->L, "Position");
	lua_gettable(this->L, -2);
	
	// Pushes how many elements we want to generate.
	lua_pushinteger(this->L, (int)this->FourVectors.size());
	
	if (lua_pcall(this->L, 1, 1, 0) != 0) {
	
		cout << "Error running function " << lua_tostring(this->L, -1);
		exit(0);
		
	}
	
	if (lua_type(this->L, -1) != LUA_TTABLE) {
		
		cout << "Expecting to obtain a table after calling position";
		cout << " function.";
		exit(0);
			
	}
	
}

/*
 * Parse_ParticleFileType()
 * 
 * * Description
 * 
 * 		...
 * 
 * */

void ParticlesConfigLua::Parse_ParticleFileType() {
	
	if (this->ParticleFileType.find("Four Vector") != std::string::npos) {
		
		this->FourVectorFile = true;
		Parse_ParticleFileType_FourVector();
		
	}
	
}

/*
 * Parse_ParticleFileType_FourVector()
 * 
 * * Description
 * 
 * 		If this->ParticleFileType was determined to contain the string
 * 		"Four Vector" then this function is called to determine
 * 		whether this->ParticleFileType has other attributes.
 * 
 * */

void ParticlesConfigLua::Parse_ParticleFileType_FourVector() {
	
	/*
	 * Determines whether the string "with name" can be found
	 * within the string this->ParticleFileType.
	 * 
	 * If it doesn't find the string "with name", the user is expected
	 * to give the name of the particles via the lua script.
	 * 
	 * */
	 
	if (this->ParticleFileType.find("with name") != std::string::npos) {
		
		this->FileHasParticleNames = true;
		
	} else {
		
		LoadTable("Particle_Table");
		this->PrimaryParticle_Name = GetStringFromTable_WithHalt(
                                                        "Particle_Name",
                                         "Must specify Particle_Name.");
		lua_pop(this->L, 1);
		
	}
	
	/*
	 * Determines whether the string "with position" can be found
	 * within the string this->ParticleFileType.
	 * 
	 * If it doesn't find the string "with position", the user is
	 * expected to give the position of the particles via the lua
	 * script.
	 * 
	 * */
	
	if (this->ParticleFileType.find("with position") != std::string::npos) {
		
		this->FileHasPosition = true;
		
	} else {

		Parse_ParticlePosition();

	}
	
}

/*
 * Parse_ParticlePosition()
 * 
 * * Description
 * 
 * 		...
 * 
 * */

void ParticlesConfigLua::Parse_ParticlePosition() {

	LoadTable("Particle_Table");
	lua_pushstring(this->L, "Position");
	lua_gettable(this->L, -2);
	
	switch (lua_type(this->L, -1)) {
	
		case LUA_TTABLE:
		
			/*
			 * Position table is at top of stack but GetG4ThreeVector
			 * assumes that Position isn't loaded so we have to pop
			 * it before calling GetG4ThreeVector("Position")
			 * 
			 * */
			
			lua_pop(this->L, 1);
			this->Position = GetG4ThreeVector("Position");
		
		break;
		case LUA_TSTRING:
		
			if (strcasecmp(lua_tostring(this->L, 1), "distribution") == 0) {
			
				// TODO GEANT4 provides a distribution function.
				
			}
			lua_pop(this->L, 1);
		
		break;
		case LUA_TFUNCTION:
		
			//Just saving for later.
			this->Position = G4ThreeVector(0., 0., 0.);
			this->PositionDefinedByFunction = true;
			lua_pop(this->L, 1);
		
		break;
		default:
		
			lua_pop(this->L, 1);
		
		break;
		
	}
	
	// Pop Particle_Table
	lua_pop(this->L, 1);
	
}

/*
 * ReadFile_FourVector()
 * 
 * * Description
 * 
 * 		...
 * 
 * */

void ParticlesConfigLua::ReadFile_FourVector() {
	
	string path = this->Module_Path + "/" + this->ParticleFile;
	FILE *fp = fopen(path.c_str(), "r");
	
	FourVector Temp_FourVector;
	
	/*
	 * this->ParticleFileType has the following substrings:
	 * 
	 * 		"with name"
	 * 		"with position"
	 * 
	 * Reads a file with the following format:
	 * 
	 * <Particle Name> , <E/c, P_x, P_y, P_z> , <X, Y, Z>
	 *      Name       ,      Four Vector     ,  Position
	 * 
	 * */
	 
	if (this->FileHasParticleNames && this->FileHasPosition) {
	
		string input = "%s %lf %lf %lf %lf %lf %lf %lf";
		char Temp_ParticleName[256] = {'\0'};
		while (fscanf(fp, input.c_str(), Temp_ParticleName,
                                        &Temp_FourVector.E,
                                        &Temp_FourVector.P_x,
                                        &Temp_FourVector.P_y,
                                        &Temp_FourVector.P_z,
                                        &Temp_FourVector.X,
                                        &Temp_FourVector.Y,
                                        &Temp_FourVector.Z) != EOF) 
		{
			
			Temp_FourVector.ParticleName = G4String(Temp_ParticleName);
			this->FourVectors.push_back(Temp_FourVector);
			
		}
	
	/*
	 * this->ParticleFileType has the following substrings:
	 * 
	 * 		"with name"
	 * 
	 * Reads a file with the following format:
	 * 
	 * <Particle Name> , <E/c, P_x, P_y, P_z>
	 *      Name       ,      Four Vector
	 * 
	 * */
	
	} else if (this->FileHasParticleNames) {
	
		string input = "%s %lf %lf %lf %lf";
		char Temp_ParticleName[256] = {'\0'};
		
		Temp_FourVector.X = this->Position.x();
		Temp_FourVector.Y = this->Position.y();
		Temp_FourVector.Z = this->Position.z();
		
		while (fscanf(fp, input.c_str(), Temp_ParticleName,
                                        &Temp_FourVector.E,
                                        &Temp_FourVector.P_x,
                                        &Temp_FourVector.P_y,
                                        &Temp_FourVector.P_z) != EOF) 
		{
			
			Temp_FourVector.ParticleName = G4String(Temp_ParticleName);
			
			this->FourVectors.push_back(Temp_FourVector);
			
		}
		
	/*
	 * this->ParticleFileType has the following substrings:
	 * 
	 * 		"with position"
	 * 
	 * Reads a file with the following format:
	 * 
	 * <E/c, P_x, P_y, P_z> , <X, Y, Z>
	 *      Four Vector     , Position
	 *  
	 * */
	
	} else if (this->FileHasPosition) {
		
		string input = "%lf %lf %lf %lf %lf %lf %lf";
		Temp_FourVector.ParticleName = this->PrimaryParticle_Name;
		
		while (fscanf(fp, input.c_str(), &Temp_FourVector.E,
                                         &Temp_FourVector.P_x,
                                         &Temp_FourVector.P_y,
                                         &Temp_FourVector.P_z,
                                         &Temp_FourVector.X,
                                         &Temp_FourVector.Y,
                                         &Temp_FourVector.Z) != EOF) 
		{
			
			this->FourVectors.push_back(Temp_FourVector);
			
		}
		
	/*
	 * this->ParticleFileType has ***none*** the following substrings:
	 * 
	 * 		"with name"
	 * 		"with position"
	 * 
	 * 	Reads a file with the following format:	
	 * 
	 * <E/c, P_x, P_y, P_z>
	 *      Four Vector
	 * 
	 * */	
		
	} else {
		
		string input = "%lf %lf %lf %lf";
		
		Temp_FourVector.ParticleName = this->PrimaryParticle_Name;
		Temp_FourVector.X = this->Position.x();
		Temp_FourVector.Y = this->Position.y();
		Temp_FourVector.Z = this->Position.z();
		
		while (fscanf(fp, input.c_str(), &Temp_FourVector.E,
                                         &Temp_FourVector.P_x,
                                         &Temp_FourVector.P_y,
                                         &Temp_FourVector.P_z) != EOF) 
		{
		
			this->FourVectors.push_back(Temp_FourVector);
			
		}
		
	}
	
	fclose(fp);
	
}

