// FILENAME: 	twobridgeexp.js
// AUTHOR: 		Josh Daymude
// DESCRIPTION: Used to perform test runs with varying parameters of the 2-Site Exploration Bridging simulation.

// script run parameters
var id = 1;
var randSeed = 96087;
var outfilepath = "C:/Users/Josh/SkyDrive/Documents/Research/SOPS Simulator/AmoebotSim/experiments/twositeebridging/exp" + id.toString() + ".csv";

// show categories in .csv
writeToFile(outfilepath, "NUM_PARTICLES,EXP_LAMB,COMP_LAMB,SITE_DIST_FACTOR,TIME_TO_DISCOVER\n");

// set random seed for rng
setSeed(randSeed);

// run and record test cases