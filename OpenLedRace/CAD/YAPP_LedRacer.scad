//---------------------------------------------------------
// Yet Another Parameterized Projectbox generator
//
//  This will generate a projectbox for a "Arduino UNO 'clone'"
//
//  Version 3.0 (02-12-2023)
//
// This design is parameterized based on the size of a PCB.
//---------------------------------------------------------
include <./YAPPgenerator_v3.scad>

// Note: length/lengte refers to X axis, 
//       width/breedte to Y, 
//       height/hoogte to Z

/*
      padding-back|<------pcb length --->|<padding-front
                            RIGHT
        0    X-as ---> 
        +----------------------------------------+   ---
        |                                        |    ^
        |                                        |   padding-right 
        |                                        |    v
        |    -5,y +----------------------+       |   ---              
 B    Y |         | 0,y              x,y |       |     ^              F
 A    - |         |                      |       |     |              R
 C    a |         |                      |       |     | pcb width    O
 K    s |         |                      |       |     |              N
        |         | 0,0              x,0 |       |     v              T
      ^ |   -5,0  +----------------------+       |   ---
      | |                                        |    padding-left
      0 +----------------------------------------+   ---
        0    X-as --->
                          LEFT
*/

//-- which half do you want to print?
printBaseShell    = true;
printLidShell     = false;


//-- Edit these parameters for your own board dimensions
wallThickness       = 1.8;
basePlaneThickness  = 1.2;
lidPlaneThickness   = 1.7;

//-- Total height of box = basePlaneThickness + lidPlaneThickness 
//--                     + baseWallHeight + lidWallHeight
//-- space between pcb and lidPlane :=
//--      (baseWallHeight+lidWall_heigth) - (standoff_heigth+pcbThickness)
//--      (6.2 + 4.5) - (3.5 + 1.5) ==> 5.7
baseWallHeight    = 30.5;
lidWallHeight     = 5.5;

//-- pcb dimensions
pcbLength         = 70;
pcbWidth          = 50;
pcbThickness      = 1.5;
                            
//-- padding between pcb and inside wall
paddingFront      = 3;
paddingBack       = 21;
paddingRight      = 21;
paddingLeft       = 3;

//-- ridge where base and lid off box can overlap
//-- Make sure this isn't less than lidWallHeight
ridgeHeight       = 3.5;
roundRadius       = 2.0;

//-- How much the PCB needs to be raised from the base
//-- to leave room for solderings and whatnot
standoffHeight    = 3.5;
pinDiameter       = 2.0;
standoffDiameter  = 5.2;


//-- C O N T R O L -------------//-> Default ---------
showSideBySide      = true;     //-> true
previewQuality      = 5;        //-> from 1 to 32, Default = 5
renderQuality       = 8;        //-> from 1 to 32, Default = 8
onLidGap            = 5;
shiftLid            = 1;
hideLidWalls        = false;    //-> false
colorLid            = "yellow";   
hideBaseWalls       = false;    //-> false
colorBase           = "gray";
showPCB             = false;
showSwitches        = false;
showPCBmarkers      = false;
showShellZero       = false;
showCenterMarkers   = false;
inspectX            = 0;        //-> 0=none (>0 from Back)
inspectY            = 0;        //-> 0=none (>0 from Right)
inspectZ            = 0;        //-> 0=none (>0 from Bottom)
inspectXfromBack    = true;     //-> View from the inspection cut foreward
inspectYfromLeft    = true;     //-> View from the inspection cut to the right
inspectZfromTop     = false;    //-> View from the inspection cut down
//-- C O N T R O L ---------------------------------------


//===================================================================
// *** PCB Supports ***
// Pin and Socket standoffs 
//-------------------------------------------------------------------
//  Default origin =  yappCoordPCB : pcb[0,0,0]
//
//  Parameters:
//   Required:
//    (0) = posx
//    (1) = posy
//   Optional:
//    (2) = Height to bottom of PCB : Default = standoffHeight
//    (3) = PCB Gap : Default = -1 : Default for yappCoordPCB=pcbThickness, yappCoordBox=0
//    (4) = standoffDiameter    Default = standoffDiameter;
//    (5) = standoffPinDiameter Default = standoffPinDiameter;
//    (6) = standoffHoleSlack   Default = standoffHoleSlack;
//    (7) = filletRadius (0 = auto size)
//    (n) = { <yappBoth> | yappLidOnly | yappBaseOnly }
//    (n) = { yappHole, <yappPin> } // Baseplate support treatment
//    (n) = { <yappAllCorners> | yappFrontLeft | yappFrontRight | yappBackLeft | yappBackRight }
//    (n) = { yappCoordBox, <yappCoordPCB> }  
//    (n) = { yappNoFillet }
//-------------------------------------------------------------------
pcbStands = 
[
  [ 2,  2, 3.5, yappBaseOnly, yappPin, ]                           // back-left
 ,[ 2, 48, 3.5, yappBaseOnly, yappPin]                                    // back-right
 ,[68,  2, 3.5, yappBaseOnly, yappPin]                 // front-left
 ,[68, 48, 3.5, yappBaseOnly, yappPin]                // front-right
];

//===================================================================
//  *** Cutouts ***
//    There are 6 cutouts one for each surface:
//      cutoutsBase (Bottom), cutoutsLid (Top), cutoutsFront, cutoutsBack, cutoutsLeft, cutoutsRight
//-------------------------------------------------------------------
//  Default origin = yappCoordBox: box[0,0,0]
//
//                        Required                Not Used        Note
//                      +-----------------------+---------------+------------------------------------
//  yappRectangle       | width, length         | radius        |
//  yappCircle          | radius                | width, length |
//  yappRoundedRect     | width, length, radius |               |     
//  yappCircleWithFlats | width, radius         | length        | length=distance between flats
//  yappCircleWithKey   | width, length, radius |               | width = key width length=key depth
//  yappPolygon         | width, length         | radius        | yappPolygonDef object must be provided
//
//  Parameters:
//   Required:
//    (0) = from Back
//    (1) = from Left
//    (2) = width
//    (3) = length
//    (4) = radius
//    (5) = shape : {yappRectangle | yappCircle | yappPolygon | yappRoundedRect | yappCircleWithFlats | yappCircleWithKey}
//  Optional:
//    (6) = depth : Default = 0/Auto : 0 = Auto (plane thickness)
//    (7) = angle : Default = 0
//    (n) = { yappPolygonDef } : Required if shape = yappPolygon specified -
//    (n) = { yappMaskDef } : If a yappMaskDef object is added it will be used as a mask for the cutout.
//    (n) = { [yappMaskDef, hOffset, vOffst, rotation] } : If a list for a mask is added it will be used as a mask for the cutout. With the Rotation and offsets applied. This can be used to fine tune the mask placement within the opening.
//    (n) = { <yappCoordBox> | yappCoordPCB }
//    (n) = { <yappOrigin>, yappCenter }
//  (n) = { yappLeftOrigin, <yappGlobalOrigin> } // Only affects Top(lid), Back and Right Faces
//-------------------------------------------------------------------
cutoutsLid =  
[
      [pcbLength/2, pcbWidth/2 ,25, 25, 0, yappPolygon, shapeHexagon, maskHoneycomb, yappCenter, yappCoordPCB]

//    [ pcbLength/2, (pcbWidth-1)/2,   51, 21, 1,  yappRoundedRect, yappCenter]  // 
];
              
//-- base plane    -- origin is pcb[0,0,0]
// (0) = posx
// (1) = posy
cutoutsBase =   
[
      [pcbLength/2, pcbWidth/2 ,45, 45, 0, yappPolygon, shapeHexagon, maskHoneycomb, yappCenter, yappCoordPCB]
];

//-- front plane  -- origin is pcb[0,0,0]
// (0) = posy
// (1) = posz
cutoutsFront = 
[
//    [ (pcbWidth-1)/2, 8,   12, 8, 1,  yappRoundedRect, yappCenter]  // USB
//    ,[ 40, 8, 1,1, 3.7,  yappCircle, yappCenter]  // Reset
];

//-- back plane  -- origin is pcb[0,0,0]
// (0) = posy
// (1) = posz
cutoutsBack = 
[
    [ (pcbWidth+paddingRight)/2+15, 12,   14, 14,  10.5,   yappCircle, yappCenter]  // Power Switch
   ,[ (pcbWidth+paddingRight)/2+15, 12,   22, 5,      1,   yappRoundedRect, yappCenter]  // Power Switch
   ,[ (pcbWidth+paddingRight)/2-20, 12,   14, 14,     6,   yappCircle, yappCenter]  // Power In
];

//-- right plane  -- origin is pcb[0,0,0]
// (0) = posX
// (1) = posZ
cutoutsRight = 
[
    [ (pcbLength-1)/2, 12, 20, 8,  0.1,  yappRoundedRect, yappCenter, yappCoordPCB] // right headers
];
//-- right plane  -- origin is pcb[0,0,0]
// (0) = posX
// (1) = posZ
cutoutsLeft = 
[
    [ 55, 15,  12, 8, 1,  yappRoundedRect, yappCenter]  // USB
   ,[ 11, 3.5, 1,1, 4,  yappCircle, yappCenter]  // Gameport 1
   ,[ 29, 3.5, 1,1, 4,  yappCircle, yappCenter]  // Gaemport 2
//    [46.5, 7, 42, 3, 2,  yappRoundedRect, 9, yappCenter, yappCoordPCB]   // left headers
];


//===================================================================
//  *** Snap Joins ***
//-------------------------------------------------------------------
//  Default origin = yappCoordBox: box[0,0,0]
//
//  Parameters:
//   Required:
//    (0) = posx | posy
//    (1) = width
//    (n) = yappLeft / yappRight / yappFront / yappBack (one or more)
//   Optional:
//    (n) = { <yappOrigin> | yappCenter }
//    (n) = { yappSymmetric }
//    (n) = { yappRectangle } == Make a diamond shape snap
//-------------------------------------------------------------------
snapJoins   =   
[
    [10, 5,  yappRight, yappLeft, yappSymmetric]
];

//===================================================================
//  *** Labels ***
//-------------------------------------------------------------------
//  Default origin = yappCoordBox: box[0,0,0]
//
//  Parameters:
//   (0) = posx
//   (1) = posy/z
//   (2) = rotation degrees CCW
//   (3) = depth : positive values go into case (Remove) negative valies are raised (Add)
//   (4) = plane {yappLeft | yappRight | yappFront | yappBack | yappTop | yappBottom}
//   (5) = font
//   (6) = size
//   (7) = "label text"
//-------------------------------------------------------------------
labelsPlane = [
               [28, 14,  0, 0.8, yappTop, "Arial:style=bold", 4, "Arduino CLONE" ]
             , [57, 25, 90, 0.8, yappTop, "Liberation Mono:style=bold", 5, "YAPP" ]
             , [33, 23,  0, 0.8, yappTop, "Liberation Mono:style=bold", 4, "L13" ]
             , [33, 30,  0, 0.8, yappTop, "Liberation Mono:style=bold", 4, "TX" ]
             , [33, 36,  0, 0.8, yappTop, "Liberation Mono:style=bold", 4, "RX" ]
             , [33, 43,  0, 0.8, yappTop, "Liberation Mono:style=bold", 4, "PWR" ]
            ];


YAPPgenerate();