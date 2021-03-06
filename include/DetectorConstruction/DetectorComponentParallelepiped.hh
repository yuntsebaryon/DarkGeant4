/*
 * DetectorComponentParallelepiped.hh
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

#ifndef DETECTORCOMPONENTPARALLELEPIPED_HH
#define DETECTORCOMPONENTPARALLELEPIPED_HH

#include "DetectorComponent.hh"


class DetectorComponent_Parallelepiped : public DetectorComponent {

	/*
	 * Class Member variables
	 *
	 */
	
	public:
	
		G4double xHalfLength;
		G4double yHalfLength;
		G4double zHalfLength;
		G4double AngleOfXZFaces;
		G4double PolarAngleOfXYFaces;
		G4double AzimuthalAngleOfXYFaces;

	/*
	 * Public Class Member functions
	 * 
	 * */
	 public:
	 
		//Constructor
		DetectorComponent_Parallelepiped(DetectorComponent_vars vars);
				
		~DetectorComponent_Parallelepiped();
		
		void ConstructVolume();
		bool WithinVolume(G4double x, G4double y, G4double z);
	
};

#endif

