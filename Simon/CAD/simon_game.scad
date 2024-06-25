outer_dia = 150; 
wall_thickness = 2;
$fn=64;

module simon_base(height=12) {
    difference() {
        cylinder(d=outer_dia-wall_thickness, h=height);
        translate([0,0,wall_thickness]) cylinder(d=outer_dia-3*wall_thickness, h=height);
        translate([0,0,wall_thickness]) cube([70,50,2], center=true);  
        translate([0,73,10]) cube([3.4,4,5], center=true);  
        translate([0,-73,10]) cube([3.4,4,5], center=true);  
    }
    translate([0,0,3]) simon_led_carrier();    
}

module simon_lid(height=15) {
  difference() {
    union() {  
    difference() {
        cylinder(d=outer_dia, h=height);
        translate([0,0,wall_thickness]) cylinder(d=outer_dia-2*wall_thickness, h=height);
        translate([0,0,height-5]) cylinder(d=outer_dia-wall_thickness+0.4, h=height);

        translate([ 0, 0,-(wall_thickness/2)]) cylinder(d=3, h=height+wall_thickness);
        translate([ 0, 6,-(wall_thickness/2)]) cylinder(d=3, h=height+wall_thickness);
        translate([ 0,-6,-(wall_thickness/2)]) cylinder(d=3, h=height+wall_thickness);
        translate([ 6, 0,-(wall_thickness/2)]) cylinder(d=3, h=height+wall_thickness);
        translate([-6, 0,-(wall_thickness/2)]) cylinder(d=3, h=height+wall_thickness);
        translate([0,0,-(wall_thickness/2)]) union() {
            difference() {
                cylinder(d=130, h=height+wall_thickness);
                cylinder(d=70, h=height+wall_thickness);
            }
        }

    }
    translate([50,0,wall_thickness/2]) cube([40,10,wall_thickness], center=true); 
    translate([-50,0,wall_thickness/2]) cube([40,10,wall_thickness], center=true); 
    translate([0,50,wall_thickness/2]) cube([10,40,wall_thickness], center=true); 
    translate([0,-50,wall_thickness/2]) cube([10,40,wall_thickness], center=true); 

    translate([52,52,height-2.5]) rotate([0,0,45]) cube([2,3,5], center=true);  
    translate([-52,-52,height-2.5]) rotate([0,0,45]) cube([2,3,5], center=true);
    } 
    translate([0,-75,height/2]) rotate([90,0,0]) cylinder(d=12.5, h=30, center=true);
  } 
}



module simon_button_cutout() {
    projection() difference() {
        cylinder(d=outer_dia, h=2);
        simon_lid();
        cylinder(d=50, h=20);
        translate([outer_dia/2,0,0]) cube([outer_dia, outer_dia, 20], center=true);
        translate([0,outer_dia/2,0]) cube([outer_dia, outer_dia, 20], center=true);
    }
}

module simon_btn() {
    difference() {
        linear_extrude(height=8) simon_button_cutout();
        translate([-4,-4,wall_thickness]) scale([0.9,0.9, 1]) linear_extrude(height=8.5) simon_button_cutout();
    }
}

module simon_button(height=15) {
    difference() {
        difference() {
            cylinder(d=129.8, h=height);
            translate([0,0,-(wall_thickness/2)]) cylinder(d=70.2, h=height+wall_thickness);
            translate([(outer_dia-10.2)/2,0,-1]) cube([outer_dia, outer_dia, 40], center=true);
            translate([0,(outer_dia-10.2)/2,-1]) cube([outer_dia, outer_dia, 40], center=true);
        }
        translate([0,0,wall_thickness]) difference() {
            cylinder(d=126, h=height);
            translate([0,0,0]) cylinder(d=74, h=height);
            translate([(outer_dia-10-wall_thickness*2)/2,0,0]) cube([outer_dia, outer_dia, 40], center=true);
            translate([0,(outer_dia-10-wall_thickness*2)/2,0]) cube([outer_dia, outer_dia, 40], center=true);
        }
        translate([-5,-50,height]) cube([10,10,4], center=true); 
        translate([-50,-5,height]) cube([10,10,4], center=true); 
        translate([-25,-25,height]) cube([10,10,4], center=true); 
    }
}

module simon_led_carrier() {
    difference() {
        cube([100,100,2], center=true);
        translate([0,0,0])cube([70.2,50.2,4], center=true);
    }
    translate([ 45, 13.5,1]) cylinder(h=1, d1=4, d2=3);
    translate([ 45, 13.5,2]) cylinder(h=3, d=2.5);
    translate([ 45,-13.5,1]) cylinder(h=1, d1=4, d2=3);
    translate([ 45,-13.5,2]) cylinder(h=3, d=2.5);
    translate([-45, 13.5,1]) cylinder(h=1, d1=4, d2=3);
    translate([-45, 13.5,2]) cylinder(h=3, d=2.5);
    translate([-45,-13.5,1]) cylinder(h=1, d1=4, d2=3);
    translate([-45,-13.5,2]) cylinder(h=3, d=2.5);
    translate([ 13.5, 45,1]) cylinder(h=1, d1=4, d2=3);
    translate([ 13.5, 45,2]) cylinder(h=3, d=2.5);
    translate([-13.5, 45,1]) cylinder(h=1, d1=4, d2=3);
    translate([-13.5, 45,2]) cylinder(h=3, d=2.5);
    translate([ 13.5,-45,1]) cylinder(h=1, d1=4, d2=3);
    translate([ 13.5,-45,2]) cylinder(h=3, d=2.5);
    translate([-13.5,-45,1]) cylinder(h=1, d1=4, d2=3);
    translate([-13.5,-45,2]) cylinder(h=3, d=2.5);
}

module simon_text() {
    linear_extrude(2) text(text="simon", size=14, font="CuppaJoe", halign="center", valign="center"); 
}

//simon_base();
//simon_lid(); 
translate([0,15,2]) simon_text();
//simon_button();
//simon_led_carrier();
