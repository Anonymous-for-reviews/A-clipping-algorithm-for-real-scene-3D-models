
########
Code is for non-commercial use only.

########
Our code is developed using C++ and the open source graphics framework of open scene graph (OSG), and the code runs under a Windows environment.
It is necessary to ensure that the computer has the following requirements: Microsoft Visual Studio and OSG environment (configuration method: http://www.openscenegraph.org/index.php/documentation/getting-started).

########
Steps to run the code:
Clipping:  
1. Open LoadModel\LoadModel.cpp, change the define value of the fifth line to 1, and the define value of the sixth line to 0.  
2. Uncomment line 70 and change the path of line 70 to the absolute path of the file Tile_37\Tile_37.osgb. 
3. Change the path on line 135 to the absolute path of the file Lines11.osg. 
4. Modify the path on line 220 to the absolute path of the folder you want to save to.
5. Run the program to get the clipping results.

View the clipping results:   
1. Change the define value of the fifth line to 0, and the define value of the sixth line to 1.
2. Comment out line 70. Modify the path on line 73 to the absolute path of the Tile_37.osgb file in the clipping result folder.
3. Run the program to see the clipping results.

########
Data:
Tile_37 is a real-scene 3D model in OSGB format, which is generated with oblique photogrammetric images. The model has 85 corresponding OSGB files. Lines11.osg is the boundary line in OSG format.
