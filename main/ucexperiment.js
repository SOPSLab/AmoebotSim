// FILENAME: 	ucexperiment.js
// AUTHOR: 		Josh Daymude
// DESCRIPTION: Used to perform test runs of the universalcoating algorithm.

// various parameters, most are customizable
var id = 1;
var randSeed = 96087;
var outfilepath = "./test.txt"

// running test cases (forever)
//setSeed(randSeed);

let holeProb = 0.1;
let mode = "h";
let numParticles = 200;

for (i = 0; i < 6; i++) {
    numParticles += i;
    shapeformation(numParticles, holeProb, mode);
    writeToFile(outfilepath, numParticles.toString() + "," + mode + "," + holeProb.toString() + "\n");
    writeToFile(outfilepath, "STUFF: " + getNumParticles() + ", " + getNumObjects())
}


