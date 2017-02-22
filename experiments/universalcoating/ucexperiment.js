// FILENAME: 	ucexperiment.js
// AUTHOR: 		Josh Daymude
// DESCRIPTION: Used to perform test runs of the universalcoating algorithm.

// various parameters, most are customizable
var id = 1;
var randSeed = 96087;
var outfilepath = "/Users/Alex/amoebotsim/ucexperiment" + id.toString() + ".csv";

// show categories in .csv
writeToFile(outfilepath, "STATIC_RADIUS,NONSTATIC_SIZE,HOLE_PROB,ASYNCH_ROUNDS,WORK,WEAK_BOUND,STRONG_BOUND,LE_ROUNDS\n");

// running test cases (forever)
//setSeed(randSeed);

for(radius = 6; radius <= 6; radius += 1) {

	for(size = 400; size <= 600; size += 100) {

	holeprob  =0.1;
	roundTotal = 0.0;
	workTotal = 0.0;
	trialCount  = 20.0;
	weakTotal = 0.0;
	strongTotal = 0.0;
	leRoundTotal = 0.0;
	for(iter = 0; iter<trialCount; iter++)
	{
				universalcoating(radius,size,holeprob);
			runUntilTermination();
			writeToFile(outfilepath,radius.toString()+","+size.toString()+","+holeprob.toString()+"," +
			getNumRounds() + "," + getNumMovements() + "," +getWeakBound()+","+getStrongBound()+ ","+getLeaderElectionRounds()+"\n");
				roundTotal += getNumRounds();
					workTotal += getNumMovements();
					weakTotal += getWeakBound();
					strongTotal+= getStrongBound();
					leRoundTotal +=getLeaderElectionRounds();
			}


		roundTotal = roundTotal / trialCount;
		workTotal  = workTotal / trialCount;
		weakTotal = weakTotal/ trialCount;
		strongTotal = strongTotal/ trialCount;
		leRoundTotal = leRoundTotal/trialCount;
		writeToFile(outfilepath,"average"+","+size.toString()+","+"," +
		roundTotal + "," + workTotal + ","+weakTotal+","+strongTotal+","+leRoundTotal+"\n");
		}

}
