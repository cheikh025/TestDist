#include "LKHmain.h"

double main(int argc, char *argv[])
{
    
    int mySeed = 1;
    const char * myFileName = argv[1];
    GainType Cost, OldOptimum;
    double Time, LastTime = GetTime();

    /* Read the specification of the problem */
	ParameterFileName = (char*)myFileName ;
    ReadParameters();
	Seed = mySeed;
    MaxMatrixDimension = 20000;
    MergeWithTour = Recombination == IPT ? MergeWithTourIPT :
    MergeWithTourGPX2;
    ReadProblem();

    if (SubproblemSize > 0) {
        if (DelaunayPartitioning)
            SolveDelaunaySubproblems();
        else if (KarpPartitioning)
            SolveKarpSubproblems();
        else if (KCenterPartitioning)
            SolveKCenterSubproblems();
        else if (KMeansPartitioning)
            SolveKMeansSubproblems();
        else if (RohePartitioning)
            SolveRoheSubproblems();
        else if (MoorePartitioning || SierpinskiPartitioning)
            SolveSFCSubproblems();
        else
            SolveTourSegmentSubproblems();
        return EXIT_SUCCESS;
    }
    AllocateStructures();

    CreateCandidateSet();

    InitializeStatistics();

    if (Norm != 0)
        BestCost = PLUS_INFINITY;
    else {
        /* The ascent has solved the problem! */
        Optimum = BestCost = (GainType) LowerBound;
        UpdateStatistics(Optimum, GetTime() - LastTime);
        RecordBetterTour();
        RecordBestTour();
        WriteTour(OutputTourFileName, BestTour, BestCost);
        WriteTour(TourFileName, BestTour, BestCost);
        Runs = 0;
    }

    /* Find a specified number (Runs) of local optima */
    for (Run = 1; Run <= Runs; Run++) {
        LastTime = GetTime();
        Cost = FindTour();      /* using the Lin-Kernighan heuristic */
        if (MaxPopulationSize > 1) {
            /* Genetic algorithm */
            int i;
            for (i = 0; i < PopulationSize; i++) {
                GainType OldCost = Cost;
                Cost = MergeTourWithIndividual(i);
                if (TraceLevel >= 1 && Cost < OldCost) {
                    printff("  Merged with %d: Cost = " GainFormat, i + 1,
                            Cost);
                    if (Optimum != MINUS_INFINITY && Optimum != 0)
                        printff(", Gap = %0.4f%%",
                                100.0 * (Cost - Optimum) / Optimum);
                    printff("\n");
                }
            }
            if (!HasFitness(Cost)) {
                if (PopulationSize < MaxPopulationSize) {
                    AddToPopulation(Cost);
                    if (TraceLevel >= 1)
                        PrintPopulation();
                } else if (Cost < Fitness[PopulationSize - 1]) {
                    i = ReplacementIndividual(Cost);
                    ReplaceIndividualWithTour(i, Cost);
                    if (TraceLevel >= 1)
                        PrintPopulation();
                }
            }
        } else if (Run > 1)
            Cost = MergeTourWithBestTour();
        if (Cost < BestCost) {
            BestCost = Cost;
            RecordBetterTour();
            RecordBestTour();
            WriteTour(OutputTourFileName, BestTour, BestCost);
            WriteTour(TourFileName, BestTour, BestCost);
        }
        OldOptimum = Optimum;

        if (Cost < Optimum) {
            if (FirstNode->InputSuc) {
                Node *N = FirstNode;
                while ((N = N->InputSuc = N->Suc) != FirstNode);
            }
            Optimum = Cost;
            printff("*** New optimum = " GainFormat " ***\n\n", Optimum);
        }

        Time = fabs(GetTime() - LastTime);
        UpdateStatistics(Cost, Time);
        if (TraceLevel >= 1 && Cost != PLUS_INFINITY) {
            printff("Run %d: Cost = " GainFormat, Run, Cost);

            if (Optimum != MINUS_INFINITY && Optimum != 0)
                printff(", Gap = %0.4f%%",
                        100.0 * (Cost - Optimum) / Optimum);
            printff(", Time = %0.2f sec. %s\n\n", Time,
                    Cost < Optimum ? "<" : Cost == Optimum ? "=" : "");
        }
        if (StopAtOptimum && Cost == OldOptimum && MaxPopulationSize >= 1) {
            Runs = Run;
            break;
        }
        if (PopulationSize >= 2 &&
            (PopulationSize == MaxPopulationSize ||
             Run >= 2 * MaxPopulationSize) && Run < Runs) {
            Node *N;
            int Parent1, Parent2;
            Parent1 = LinearSelection(PopulationSize, 1.25);
            do
                Parent2 = LinearSelection(PopulationSize, 1.25);
            while (Parent2 == Parent1);
            ApplyCrossover(Parent1, Parent2);
            N = FirstNode;
            do {
                if (ProblemType != HCP && ProblemType != HPP) {
                    int d = C(N, N->Suc);
                    AddCandidate(N, N->Suc, d, INT_MAX);
                    AddCandidate(N->Suc, N, d, INT_MAX);
                }
                N = N->InitialSuc = N->Suc;
            }
            while (N != FirstNode);
        }
        SRandom(++Seed);
    }

    GainType costWithoutDepot = 0;
    GainType depotCost = 0;
    
    Node* t1 = FirstNode;

    do
    {
        Node* t2 = t1->Suc;
        GainType localCost = (C(t1, t2)) - t1->Pi - t2->Pi;

        if (t1->Id == 1 || t1->Suc->Id == 1)
            depotCost += localCost;
        
        else
            costWithoutDepot += localCost;

        t1 = t2;

    } while(t1->Id != FirstNode->Id);

    depotCost /= Precision;
    costWithoutDepot /= Precision;

    GainType total = (depotCost+costWithoutDepot);
    
    int length = snprintf( NULL, 0, "%d;%d;%d", BestCost, depotCost, costWithoutDepot);
    char* valuesToReturn = malloc( length + 1 );
    snprintf( valuesToReturn, length + 1, "%d;%d;%d", BestCost, depotCost, costWithoutDepot );

    printff("Total cost: %d\n", BestCost);
    return BestCost;    
}
